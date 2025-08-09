#include <algorithm>
#include <array>
#include <atomic>
#include <exception>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <csignal>
#include <cstdlib>
#include <cstring>
#include <string.h>

#include <cpptrace/basic.hpp>
#include <cpptrace/cpptrace.hpp>
#include <cpptrace/exceptions.hpp>
#include <cpptrace/formatting.hpp>
#include <cpptrace/forward.hpp>
#include <cpptrace/from_current.hpp>

#include "Application.hh"
#include "TerminateSignal.hh"
#include "WrapLibcArgs.hh"

using namespace std::chrono_literals;

static constexpr size_t signal_stacktrace_max_frames = 1024;

namespace {
void print_trace(const cpptrace::raw_trace &trace)
{
    cpptrace::formatter f{};
    f.addresses(cpptrace::formatter::address_mode::object);
    f.snippets(true);
    f.print(trace.resolve());
}
}; // namespace

namespace {

struct CPPTraceApplication
{
    CPPTraceApplication(Application::LibcArgs args) : _args(args)
    {
    }

    Application::LibcArgs args() const
    {
        return _args;
    }

    ~CPPTraceApplication()
    {
    }

  private:
    Application::LibcArgs _args;
};

static_assert(sizeof(Application::ImplData) >= sizeof(CPPTraceApplication));
static_assert(alignof(Application::ImplData) >= alignof(CPPTraceApplication));
CPPTraceApplication &cast(Application::ImplData &d)
{
    return *reinterpret_cast<CPPTraceApplication *>(d._);
}

const CPPTraceApplication &cast(const Application::ImplData &d)
{
    return *reinterpret_cast<const CPPTraceApplication *>(d._);
}

} // namespace

template <>
Application::Application(CPPTraceApplication &&impl)
{
    new (_impl._) CPPTraceApplication{std::move(impl)};
}

Application::~Application()
{
    cast(_impl).~CPPTraceApplication();
}

std::optional<Application::LibcArgs> Application::get_libc_args()
{
    return cast(_impl).args();
}

#include <signal.h>
#include <unistd.h>

namespace {

[[noreturn]] void signal_watcher_exit(int out)
{
    _exit(out);
}

[[noreturn]] void signal_watcher_trap()
{
    while (true) {
        std::this_thread::sleep_for(1s);
    }
}

std::atomic_flag got_signal = ATOMIC_FLAG_INIT;
std::atomic_bool has_trace{false};
int signal_value = 0;
size_t sigtrace_size = 0;
std::array<cpptrace::frame_ptr, signal_stacktrace_max_frames> sigtrace_buffer{};

void launcher_fill_signal_stacktrace_and_trap(int signo)
{
    if (got_signal.test_and_set()) {
        signal_watcher_trap();
    }
    sigtrace_size = cpptrace::safe_generate_raw_trace(
        sigtrace_buffer.data(), sigtrace_buffer.size());
    signal_value = signo;
    static_assert(decltype(has_trace)::is_always_lock_free);
    has_trace = true;
    signal_watcher_trap();
}

[[noreturn]] void signal_watcher_thread()
{
    while (!has_trace) {
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
    status = status && sigaction(SIGILL, &sa, NULL) == 0;
    return status;
}

} // namespace

int main(int argc, char *argv[], char *envp[]) CPPTRACE_TRY
{
    if (!setup_signals_trace()) {
        std::cerr << "Cannot set signal tracing\n";
        return EXIT_FAILURE;
    }

    if (!setup_terminate_signals()) {
        std::cerr << "Cannot set terminate handlers\n";
        return EXIT_FAILURE;
    }

    Application app{CPPTraceApplication{wrap_args(argc, argv, envp)}};

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
    throw;
}

