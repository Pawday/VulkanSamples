#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
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
#include <cpptrace/forward.hpp>
#include <cpptrace/from_current.hpp>

#include "Application.hh"

using namespace std::chrono_literals;

static constexpr size_t signal_stacktrace_max_frames = 1024;

namespace {
void print_trace(const cpptrace::raw_trace &trace)
{
    auto res = trace.resolve();
    res.print_with_snippets();
}
}; // namespace

namespace {

struct CPPTraceApplication
{
    CPPTraceApplication(Application::MainArgs args) : _args(args)
    {
    }

    Application::MainArgs args() const
    {
        return _args;
    }

    void app_share_lifetime(std::shared_ptr<void> object)
    {
        _shared_objects.emplace_back(std::move(object));
    }

    ~CPPTraceApplication()
    {
        destruct_shared();
    }

  private:
    Application::MainArgs _args;
    std::vector<std::shared_ptr<void>> _shared_objects;

    void destruct_shared()
    {
        std::vector<std::shared_ptr<void>> defered = std::move(_shared_objects);
        std::reverse(std::begin(defered), std::end(defered));

        for (auto &obj : defered) {
            size_t users = obj.use_count();
            auto addr = obj.get();
            if (users > 1) {
                std::cout << "Defered obj at " << addr << " still has "
                          << std::to_string(users - 1) << "users" << '\n';
                continue;
            }

            std::shared_ptr<void> null{obj};
            obj.reset();
        }
    }
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

std::optional<Application::MainArgs> Application::get_main_args()
{
    return cast(_impl).args();
}

void Application::app_share_lifetime(std::shared_ptr<void> object)
{
    cast(_impl).app_share_lifetime(object);
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
        sigtrace_buffer.data(), sigtrace_buffer.size(), 0);
    signal_value = signo;
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

std::atomic_flag terminate_requested_flag = ATOMIC_FLAG_INIT;
bool setup_terminate_signals()
{
    struct sigaction sa{};
    sa.sa_handler = [](int) { terminate_requested_flag.test_and_set(); };
    bool status = true;
    status = status && sigaction(SIGTERM, &sa, NULL) == 0;
    status = status && sigaction(SIGINT, &sa, NULL) == 0;
    return status;
}

} // namespace

bool Application::terminate_requested() const
{
    bool out = terminate_requested_flag.test_and_set();
    if(!out) {
        terminate_requested_flag.clear();
    }
    return out;
}

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

    auto create_args = [&]() -> Application::MainArgs {
        Application::MainArgs args{};
        for (size_t argidx = 0; argidx != argc; ++argidx) {
            args.argv.push_back(argv[argidx]);
        }
        while (*envp != nullptr) {
            args.env.push_back(*envp);
            envp++;
        }
        return args;
    };

    Application app{CPPTraceApplication{create_args()}};

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

