#include <format>
#include <iostream>
#include <string>
#include <vector>

#include <cstdlib>

#include <glslang_c_interface.h>

#include "GLSL.hh"

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

    return EXIT_FAILURE;
}
