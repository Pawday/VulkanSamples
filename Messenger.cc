#include <format>
#include <iostream>
#include <iterator>
#include <span>

#include <cstddef>

#include "Messenger.hh"

void Messenger::msg_prefixed(std::span<const char> msg)
{
    if (m_name != nullptr) {
        auto ostream_it = std::ostream_iterator<char>(std::cout);
        std::format_to(ostream_it, "{} | ", m_name);
    }

    std::cout.write(msg.data(), msg.size());
    std::cout.write("\n", 1);
}

void Messenger::message(const char *msg)
{
    const char *start = msg;
    size_t len = 0;

    while (true) {
        len = 0;
        while (start[len] != '\n' && start[len] != 0) {
            len++;
        }

        if (start[len] == 0) {
            std::span<const char> msg_line{start, len};
            msg_prefixed(msg_line);
            return;
        }

        if (len == 0) {
            continue;
        }

        std::span<const char> msg_line{start, len};
        msg_prefixed(msg_line);

        if (start[len] == 0) {
            return;
        }

        start += len + 1;
    }
}

