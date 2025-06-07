#include <iostream>
#include <string>
#include <vector>

#include <cstdlib>

#include "GLSL.hh"

int glsl_main(const std::vector<std::string> &args)
{
    for (auto &arg : args) {
        std::cout << "glsl arg: [" << arg << "]\n";
    }

    return EXIT_FAILURE;
}
