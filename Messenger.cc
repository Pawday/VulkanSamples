#include <format>
#include <iostream>
#include <iterator>
#include <span>
#include <string_view>

#include <cstddef>

#include "Messenger.hh"

namespace {
void msg_prefixed(const char *prefix, std::string_view msg)
{
    if (prefix != nullptr) {
        auto ostream_it = std::ostream_iterator<char>(std::cout);
        std::format_to(ostream_it, "{} | ", prefix);
    }

    std::cout.write(msg.data(), msg.size());
    std::cout.write("\n", 1);
}
} // namespace

void Messenger::message(std::string_view msg)
{
    size_t offset = 0;
    size_t len = 0;

    while (true) {
        len = 0;

        auto can_expand = [&]() {
            bool o = true;
            o = o && offset + len != msg.size();
            o = o && msg[offset + len] != '\n';
            o = o && msg[offset + len] != 0;
            return o;
        };

        while (can_expand()) {
            len++;
        }

        if (msg[offset + len] == 0) {
            msg_prefixed(m_name, msg.substr(offset, len));
            return;
        }

        if (len == 0) {
            continue;
        }

        msg_prefixed(m_name, msg.substr(offset, len));

        if (msg[offset + len] == 0) {
            return;
        }

        offset += len + 1;
    }
}

