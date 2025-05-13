#pragma once

#include "Data.hh"

namespace Wayland {

struct Window
{
    Window(Window &&);
    Window &operator=(Window &&);
    ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    using ImplT = Data<128, 16>;

  private:
    friend struct Context;
    friend struct VulkanWindow;
    Window();
    ImplT impl;
};

} // namespace Wayland
