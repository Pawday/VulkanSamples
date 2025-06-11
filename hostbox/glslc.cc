#include <expected>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Include/glslang_c_shader_types.h>
#include <glslang/Public/resource_limits_c.h>

#include "Application.hh"

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

struct ProgramDeleter
{
    void operator()(glslang_program_t *p)
    {
        glslang_program_delete(p);
    }
};

using ProgramUniquePtr = std::unique_ptr<glslang_program_t, ProgramDeleter>;

void shader_logs(glslang_shader_t *sh)
{
    std::cerr << glslang_shader_get_info_log(sh) << '\n';
    std::cerr << glslang_shader_get_info_debug_log(sh) << '\n';
}

void program_logs(glslang_program_t *prog)
{
    std::cerr << glslang_program_get_info_log(prog) << '\n';
    std::cerr << glslang_program_get_info_debug_log(prog) << '\n';
}

std::expected<glslang_stage_t, std::string>
    parse_stage(std::string_view stage_str)
{
    if (stage_str == "vert") {
        return glslang_stage_t::GLSLANG_STAGE_VERTEX;
    }

    if (stage_str == "frag") {
        return glslang_stage_t::GLSLANG_STAGE_FRAGMENT;
    }

    return std::unexpected(std::format("Unknown shader stage [{}]", stage_str));
}

int glsl_main(const std::vector<std::string> &args)
{
    auto stage_op = parse_stage(args.at(0));
    if (!stage_op) {
        std::cerr << stage_op.error() << '\n';
        return EXIT_FAILURE;
    }
    glslang_stage_t stage = stage_op.value();

    auto data = read_file(args.at(1));
    data.emplace_back(std::byte{0});
    std::ofstream ofile{args.at(2), std::ios::binary};

    glslang_input_t in{};
    in.language = glslang_source_t::GLSLANG_SOURCE_GLSL;
    in.stage = stage;
    in.client = glslang_client_t::GLSLANG_CLIENT_VULKAN;
    in.client_version = GLSLANG_TARGET_VULKAN_1_0;
    in.target_language = glslang_target_language_t::GLSLANG_TARGET_SPV,
    in.target_language_version =
        glslang_target_language_version_t::GLSLANG_TARGET_SPV_1_0,
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
        return EXIT_FAILURE;
    }

    ProgramUniquePtr prog{glslang_program_create()};
    if (prog == nullptr) {
        std::cerr << "Cannot create program" << '\n';
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

    glslang_program_add_shader(prog.get(), sh.get());

    auto messages = GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT;
    if (!glslang_program_link(prog.get(), messages)) {
        std::cerr << "Cannot link program" << '\n';
        program_logs(prog.get());
        return EXIT_FAILURE;
    }

    glslang_program_SPIRV_generate(prog.get(), stage);

    std::vector<uint32_t> SPIRV_code;
    size_t code_words = glslang_program_SPIRV_get_size(prog.get());
    SPIRV_code.resize(code_words);

    glslang_program_SPIRV_get(prog.get(), SPIRV_code.data());

    const char *spirv_messages = glslang_program_SPIRV_get_messages(prog.get());
    if (spirv_messages) {
        std::cout << spirv_messages << '\n';
    }

    std::vector<std::string> ofile_lines;
    std::vector<uint32_t> active_line_nums;

    auto flush_active_line = [&ofile_lines, &active_line_nums]() {
        std::string line;
        bool first = true;
        for (auto &line_num : active_line_nums) {
            if (!first) {
                line += ',';
            }
            first = false;
            line += std::format("0x{:x}", line_num);
        }
        ofile_lines.emplace_back(std::move(line));
        active_line_nums.clear();
    };

    auto add_word = [&active_line_nums, &flush_active_line](uint32_t word) {
        if (active_line_nums.size() == 8) {
            flush_active_line();
        }
        active_line_nums.push_back(word);
    };

    for (auto &word : SPIRV_code) {
        add_word(word);
    }
    flush_active_line();

    std::string output_content;
    bool first = true;
    for (auto &line : ofile_lines) {
        if (!first) {
            output_content += ",\n";
        }
        first = false;
        output_content += line;
    }

    ofile << output_content;

    return EXIT_SUCCESS;
}

} // namespace

int Application::main()
{
    auto argv = get_libc_args().value().argv;
    if (argv.size() < 1) {
        std::cerr << "No args" << '\n';
        return EXIT_FAILURE;
    }

    argv.erase(std::begin(argv));
    return glsl_main(argv);
}

