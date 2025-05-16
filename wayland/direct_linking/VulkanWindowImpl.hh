#pragma once

#include <memory>
#include <optional>

#include <cstddef>

#include <vulkan/vulkan_raii.hpp>

#include "Wayland/VulkanWindow.hh"
#include "Wayland/Window.hh"
#include "WindowImpl.hh"

namespace Wayland {
namespace Impl {

struct VulkanWindow
{
    using SharedInstance = std::shared_ptr<vk::raii::Instance>;

    VulkanWindow(Impl::Window &w, SharedInstance instance);

    vk::raii::SurfaceKHR &surface()
    {
        return _surface.value();
    }

    Wayland::VulkanWindow::SharedInstance _vk_instance;
    std::optional<Impl::Window> _w;
    std::optional<vk::raii::SurfaceKHR> _surface;
};

namespace {
inline VulkanWindow &cast_vulkan_window(Wayland::VulkanWindow::ImplT &impl)
{
    return *reinterpret_cast<VulkanWindow *>(impl());
} // namespace

} // namespace
} // namespace Impl
} // namespace Wayland
