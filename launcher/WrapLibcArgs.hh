#pragma once

#include <cstddef>

#include "Application.hh"

namespace {

inline auto wrap_args(int argc, char *argv[], char *envp[])
    -> Application::LibcArgs
{
    Application::LibcArgs args{};
    for (size_t argidx = 0; argidx != argc; ++argidx) {
        args.argv.push_back(argv[argidx]);
    }
    while (*envp != nullptr) {
        args.env.push_back(*envp);
        envp++;
    }
    return args;
};

} // namespace
