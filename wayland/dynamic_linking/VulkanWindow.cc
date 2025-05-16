#include <stdexcept>
#include <string>
#include <utility>

#include <vulkan/vulkan_raii.hpp>

#include <Wayland/VulkanWindow.hh>
#include <Wayland/Window.hh>

namespace Wayland {

VulkanWindow::VulkanWindow(
    [[maybe_unused]] Window &w, [[maybe_unused]] SharedInstance instance)
{
    std::string message =
        "Not implemented and impl with size=" + std::to_string(sizeof(impl)) +
        " is ignored";
    throw std::runtime_error(std::move(message));
}

VulkanWindow::VulkanWindow(VulkanWindow &&)
{
    throw std::runtime_error("Not implemented");
}

VulkanWindow &VulkanWindow::operator=(VulkanWindow &&)
{
    return *this;
}
VulkanWindow::~VulkanWindow()
{
}

vk::raii::SurfaceKHR &VulkanWindow::surface()
{
    throw std::runtime_error("Not implemented");
}

} // namespace Wayland
