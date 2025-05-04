#include <algorithm>
#include <exception>
#include <iostream>
#include <memory>
#include <string>

#include <cstdlib>

#include <cpptrace/basic.hpp>
#include <cpptrace/cpptrace.hpp>
#include <cpptrace/exceptions.hpp>
#include <cpptrace/from_current.hpp>
#include <utility>
#include <vector>

#include "Application.hh"

namespace {
void print_trace(const cpptrace::raw_trace &trace)
{
    auto res = trace.resolve();
    res.print_with_snippets();
}
}; // namespace

Application::~Application()
{
    std::vector<std::shared_ptr<void>> defered = std::move(_defer_destruct);
    std::reverse(std::begin(defered), std::end(defered));

    for (auto &obj : defered) {
        size_t users = obj.use_count();
        auto addr = obj.get();
        if (users > 1) {
            std::cout << "Defered obj at " << addr << " still has "
                      << std::to_string(users - 1) << "userts" << '\n';
            continue;
        }

        std::shared_ptr<void> null{obj};
        obj.reset();
    }
}

int main(int argc, char *argv[], char *envp[]) CPPTRACE_TRY
{
    Application app{argc, argv, envp};

    CPPTRACE_TRY
    {
        return app.main();
    }
    CPPTRACE_CATCH(std::exception & e)
    {
        print_trace(cpptrace::raw_trace_from_current_exception());
        std::cout << "<std::exception::what()>\n";
        std::cout << e.what() << '\n';
        std::cout << "</std::exception::what()>\n";
        std::cout.flush();
        return EXIT_FAILURE;
    }
}
CPPTRACE_CATCH(...)
{
    print_trace(cpptrace::raw_trace_from_current_exception());
    return EXIT_FAILURE;
}

