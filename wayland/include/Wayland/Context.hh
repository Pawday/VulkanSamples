#pragma once

#include "Window.hh"

namespace Wayland {
struct Window;

struct Context
{
    Context();
    Context(Context &&);
    Context &operator=(Context &&);
    ~Context();

    Window create_window();

    void update();

    Context(const Context &) = delete;
    Context &operator=(const Context &) = delete;

  private:
    alignas(8) char _[16];
};

} // namespace Wayland
