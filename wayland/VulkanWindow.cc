#include <stdexcept>
#include <string>
#include <utility>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

#include <Wayland/VulkanWindow.hh>
#include <Wayland/Window.hh>

#include "VulkanWindowImpl.hh"
#include "WindowImpl.hh"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_to_string.hpp"

// Provided by VK_KHR_wayland_surface
using VkWaylandSurfaceCreateFlagsKHR = VkFlags;

struct VkWaylandSurfaceCreateInfoKHR
{
    VkStructureType sType;
    const void *pNext;
    VkWaylandSurfaceCreateFlagsKHR flags;
    struct wl_display *display;
    struct wl_surface *surface;
};

using vkCreateWaylandSurfaceKHR_PFN = VkResult (*)(
    VkInstance instance,
    const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkSurfaceKHR *pSurface);

namespace Wayland {

namespace Impl {

VulkanWindow::VulkanWindow(Impl::Window &w, SharedInstance instance)
    : _vk_instance(instance)
{
    auto vkCreateWaylandSurfaceKHR =
        reinterpret_cast<vkCreateWaylandSurfaceKHR_PFN>(
            instance->getProcAddr("vkCreateWaylandSurfaceKHR"));
    if (vkCreateWaylandSurfaceKHR == nullptr) {
        throw std::runtime_error("Cannot load vkCreateWaylandSurfaceKHR()");
    }

    auto &ctx = w._context;

    VkSurfaceKHR c_surface{};
    VkWaylandSurfaceCreateInfoKHR ci{};
    ci.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    ci.display = ctx->_h.display;
    ci.surface = w._h.surface;

    vk::raii::Instance &raii_instance = *instance;
    vk::Instance i{raii_instance};
    vk::Result res{vkCreateWaylandSurfaceKHR(i, &ci, nullptr, &c_surface)};
    if (res != vk::Result::eSuccess) {
        std::string message = "Cannot create vulkang-wayland surface";
        message = message + " status " + vk::to_string(res);
        throw std::runtime_error(message);
    }
    vk::raii::SurfaceKHR cxx_surface{raii_instance, c_surface};

    _surface = std::move(cxx_surface);
    _w = std::move(w);
}

}; // namespace Impl

VulkanWindow::VulkanWindow(Wayland::Window &w, SharedInstance instance)
{
    Impl::Window &impl_w = Impl::cast_window(w._);
    new (_) Impl::VulkanWindow{impl_w, instance};
}

VulkanWindow::VulkanWindow(VulkanWindow &&o)
{
    new (_) Impl::VulkanWindow{std::move(Impl::cast_vulkan_window(o._))};
}

VulkanWindow &VulkanWindow::operator=(VulkanWindow &&o)
{
    Impl::cast_vulkan_window(_).operator=(
        std::move(Impl::cast_vulkan_window(o._)));
    return *this;
}

VulkanWindow::~VulkanWindow()
{
    Impl::cast_vulkan_window(_).~VulkanWindow();
}

vk::raii::SurfaceKHR &VulkanWindow::surface()
{
    return Impl::cast_vulkan_window(_).surface();
}

} // namespace Wayland
