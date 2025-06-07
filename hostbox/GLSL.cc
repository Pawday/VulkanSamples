#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <glslang_c_interface.h>
#include <glslang_c_shader_types.h>
#include <resource_limits_c.h>

#include "GLSL.hh"

namespace {
std::vector<std::byte> read_file(const std::string &name)
{
    std::ifstream ifile{name, std::ios::binary};
    ifile.exceptions(std::fstream::badbit);
    ifile.exceptions(std::fstream::failbit);
    auto start = std::istreambuf_iterator<char>{ifile};
    auto end = std::istreambuf_iterator<char>{};
    std::vector<std::byte> out;
    while (start != end) {
        uint8_t val = *start;
        out.emplace_back(std::byte{val});
        start++;
    }

    return out;
}

struct ShaderDeleter
{
    void operator()(glslang_shader_t *sh)
    {
        glslang_shader_delete(sh);
    }
};

using ShaderUniquePtr = std::unique_ptr<glslang_shader_t, ShaderDeleter>;

void shader_logs(glslang_shader_t *sh)
{
    std::cerr << glslang_shader_get_info_log(sh) << '\n';
    std::cerr << glslang_shader_get_info_debug_log(sh) << '\n';
}

} // namespace

int glsl_main(const std::vector<std::string> &args)
{
    glslang_version_t version{};
    glslang_get_version(&version);

    std::cout << std::format(
                     "Version {}.{}.{}",
                     version.major,
                     version.minor,
                     version.patch)
              << '\n';

    auto data = read_file(args.at(0));
    data.emplace_back(std::byte{0});

    glslang_input_t in{};
    in.language = glslang_source_t::GLSLANG_SOURCE_GLSL;
    in.stage = glslang_stage_t::GLSLANG_STAGE_VERTEX;
    in.client = glslang_client_t::GLSLANG_CLIENT_VULKAN;
    in.client_version = GLSLANG_TARGET_VULKAN_1_0;
    in.target_language = glslang_target_language_t::GLSLANG_TARGET_SPV,
    in.target_language_version =
        glslang_target_language_version_t::GLSLANG_TARGET_SPV_1_5,
    in.code = reinterpret_cast<char *>(data.data());
    in.default_version = 100;
    in.default_profile = glslang_profile_t::GLSLANG_NO_PROFILE;
    in.force_default_version_and_profile = false;
    in.forward_compatible = false;
    in.messages = glslang_messages_t::GLSLANG_MSG_DEFAULT_BIT;
    in.resource = glslang_default_resource();

    ShaderUniquePtr sh{glslang_shader_create(&in)};
    if (sh == nullptr) {
        std::cerr << "Cannot create shader" << '\n';
        shader_logs(sh.get());
        return EXIT_FAILURE;
    }

    if (!glslang_shader_preprocess(sh.get(), &in)) {
        std::cerr << "Cannot preprocess shader" << '\n';
        shader_logs(sh.get());
        return EXIT_FAILURE;
    }

    if (!glslang_shader_parse(sh.get(), &in)) {
        std::cerr << "Cannot parse shader" << '\n';
        shader_logs(sh.get());
        return EXIT_FAILURE;
    }

    std::ofstream ofile{args.at(1)};

    return EXIT_SUCCESS;
}
