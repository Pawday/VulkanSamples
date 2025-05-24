#include <atomic>

#include <csignal>
#include <cstddef>

#include <signal.h>

#include "Application.hh"
#include "TerminateSignal.hh"

namespace {
std::atomic_flag terminate_requested_flag = ATOMIC_FLAG_INIT;
}

bool Application::terminate_requested() const
{
    bool out = terminate_requested_flag.test_and_set();
    if (!out) {
        terminate_requested_flag.clear();
    }
    return out;
}

bool setup_terminate_signals()
{
    struct sigaction sa{};
    sa.sa_handler = [](int) { terminate_requested_flag.test_and_set(); };
    bool status = true;
    status = status && sigaction(SIGTERM, &sa, NULL) == 0;
    status = status && sigaction(SIGINT, &sa, NULL) == 0;
    return status;
}
