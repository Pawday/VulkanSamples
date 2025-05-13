#pragma once

#include <memory>

#include <vulkan/vulkan_raii.hpp>

#include "Data.hh"
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

    using ImplT = Data<256, 16>;

  private:
    ImplT impl;
};

} // namespace Wayland
