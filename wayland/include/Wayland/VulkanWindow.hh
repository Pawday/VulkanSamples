#pragma once

#include <memory>

#include <vulkan/vulkan_raii.hpp>

#include "Window.hh"

namespace Wayland {

struct VulkanWindow
{
    using SharedInstance = std::shared_ptr<vk::raii::Instance>;
    VulkanWindow(Window &w, SharedInstance instance);
    VulkanWindow(VulkanWindow &&);
    VulkanWindow &operator=(VulkanWindow &&);
    ~VulkanWindow();

    VulkanWindow(const VulkanWindow &) = delete;
    VulkanWindow &operator=(const VulkanWindow &) = delete;

    vk::raii::SurfaceKHR &surface();

  private:
    alignas(16) char _[256];
};

} // namespace Wayland
