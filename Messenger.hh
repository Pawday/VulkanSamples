#pragma once

#include <span>

#include <cstddef>
#include <string_view>

struct Messenger
{
    Messenger(const char *name) : m_name(name)
    {
    }

    void message(std::string_view msg);

  private:
    const char *m_name = nullptr;
};
