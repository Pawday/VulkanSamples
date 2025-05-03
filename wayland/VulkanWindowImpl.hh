#pragma once

#include <cstddef>
#include <memory>
#include <optional>

#include <Wayland/VulkanWindow.hh>

#include "Wayland/Window.hh"
#include "WindowImpl.hh"
#include "vulkan/vulkan_raii.hpp"

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
template <size_t S>
VulkanWindow &cast_vulkan_window(char (&data)[S])
{
    return *reinterpret_cast<VulkanWindow *>(data);
}
} // namespace

} // namespace Impl
} // namespace Wayland
