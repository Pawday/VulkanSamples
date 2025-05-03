#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Wayland {

struct Window
{
    Window(Window &&);
    Window &operator=(Window &&);
    ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

  private:
    friend struct Context;
    friend struct VulkanWindow;
    Window();
    alignas(16) char _[128];
};

} // namespace Wayland
