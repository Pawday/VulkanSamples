#include <stdexcept>
#include <string>
#include <utility>

#include <Wayland/Window.hh>

namespace Wayland {

Window::Window(Window &&)
{
    std::string message =
        "Not implemented and impl with size=" + std::to_string(sizeof(impl)) +
        " is ignored";
    throw std::runtime_error(std::move(message));
}

Window &Window::operator=(Window &&)
{
    throw std::runtime_error("Not implemented");
    return *this;
}

Window::~Window()
{
}

Window::Window()
{
    throw std::runtime_error("Not implemented");
}

} // namespace Wayland
