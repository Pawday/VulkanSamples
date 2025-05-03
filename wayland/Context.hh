#pragma once

#include <memory>
#include <vulkan/vulkan_raii.hpp>

#include "Window.hh"

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

} // namespace Wayland
