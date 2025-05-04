#pragma once

#include <memory>
#include <vector>

struct Application
{
    Application(int i_argc, char *i_argv[], char *i_envp[])
        : argc{i_argc}, argv{i_argv}, envp{i_envp}
    {
    }
    Application(const Application &) = delete;
    Application(Application &&) = delete;
    Application &operator=(const Application &) = delete;
    Application &operator=(Application &&) = delete;
    ~Application();

    int main();

    int argc;
    char **argv;
    char **envp;

    std::vector<std::shared_ptr<void>> _defer_destruct;
};
