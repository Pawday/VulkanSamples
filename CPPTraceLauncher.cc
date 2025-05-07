#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <exception>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string.h>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <csignal>
#include <cstdlib>

#include <cpptrace/basic.hpp>
#include <cpptrace/cpptrace.hpp>
#include <cpptrace/exceptions.hpp>
#include <cpptrace/forward.hpp>
#include <cpptrace/from_current.hpp>

#include "Application.hh"

static constexpr size_t signal_stacktrace_max_frames = 1024;

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

#include <signal.h>
#include <unistd.h>

namespace {

[[noreturn]] void signal_watcher_exit(int out)
{
    _exit(out);
}

std::atomic_bool got_signal = false;
std::atomic<int> signal_value = 0;
std::atomic<size_t> sigtrace_size = 0;
std::array<cpptrace::frame_ptr, signal_stacktrace_max_frames> sigtrace_buffer{};

void launcher_fill_signal_stacktrace_and_trap(int signo)
{
    sigtrace_size = cpptrace::safe_generate_raw_trace(
        sigtrace_buffer.data(), sigtrace_buffer.size(), 0);
    signal_value = signo;
    got_signal = true;
    while (true) {
        std::this_thread::yield();
    }
}

[[noreturn]] void signal_watcher_thread()
{
    while (!got_signal) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
    }

    cpptrace::raw_trace trace;
    auto buf_start = std::begin(sigtrace_buffer);
    auto buf_end = buf_start + sigtrace_size;
    std::copy(buf_start, buf_end, std::back_inserter(trace.frames));
    print_trace(trace);
    std::cout << "Got \"" << strsignal(signal_value) << "\" signal\n";
    signal_watcher_exit(signal_value);
}

bool setup_signals_trace()
{
    std::thread signal_watcher{signal_watcher_thread};
    signal_watcher.detach();
    struct sigaction sa{};
    sa.sa_handler = launcher_fill_signal_stacktrace_and_trap;
    bool status = true;
    status = status && sigaction(SIGSEGV, &sa, NULL) == 0;
    status = status && sigaction(SIGABRT, &sa, NULL) == 0;
    status = status && sigaction(SIGBUS, &sa, NULL) == 0;
    return status;
}

} // namespace

int main(int argc, char *argv[], char *envp[]) CPPTRACE_TRY
{
    if (!setup_signals_trace()) {
        std::cerr << "Cannot set signal tracing\n";
        return EXIT_FAILURE;
    }

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

