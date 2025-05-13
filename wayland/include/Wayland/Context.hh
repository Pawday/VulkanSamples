#pragma once

#include <cstddef>

#include "Data.hh"
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

    using ImplT = Data<8>;

  private:
    ImplT impl;
};

} // namespace Wayland
