#pragma once

#include <span>

#include <cstddef>

struct Messenger
{
    Messenger(const char *name) : m_name(name)
    {
    }

    void message(const char *msg);

  private:
    void msg_prefixed(std::span<const char> msg);

    const char *m_name = nullptr;
};
