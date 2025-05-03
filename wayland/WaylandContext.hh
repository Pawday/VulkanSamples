#pragma once

#include <memory>
#include <vulkan/vulkan_raii.hpp>

namespace Wayland {
struct Window;

struct Context
{
    using SharedInstance = std::shared_ptr<vk::raii::Instance>;
    Context(SharedInstance I);
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

struct Window
{
    Window(Window &&);
    Window &operator=(Window &&);
    ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    vk::raii::SurfaceKHR &surface();

  private:
    friend struct Context;
    Window();
    alignas(16) char _[128];
};

} // namespace Wayland
