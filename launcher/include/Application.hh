#pragma once

#include <optional>
#include <string>
#include <vector>

struct Application
{
    int main();

    template <typename... TS>
    Application(TS &&...ts);

    Application(const Application &) = delete;
    Application(Application &&) = delete;
    Application &operator=(const Application &) = delete;
    Application &operator=(Application &&) = delete;
    ~Application();

    struct LibcArgs
    {
        std::vector<std::string> argv;
        std::vector<std::string> env;
    };
    std::optional<LibcArgs> get_libc_args();

    bool terminate_requested() const;

    struct ImplData
    {
        alignas(8) char _[128];
    } _impl;
};
