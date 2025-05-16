#include <stdexcept>
#include <string>
#include <utility>

#include <cstring>

#include <Wayland/Context.hh>

namespace Wayland {

Context::Context()
{
    std::string message =
        "Not implemented and impl with size=" + std::to_string(sizeof(impl)) +
        " is ignored";
    throw std::runtime_error(std::move(message));
}

Context::Context(Context &&)
{
    throw std::runtime_error("Not implemented");
}

Context &Context::operator=(Context &&)
{
    throw std::runtime_error("Not implemented");
    return *this;
}

Context::~Context()
{
}

Window Context::create_window()
{
    throw std::runtime_error("Not implemented");
}

void Context::update()
{
    throw std::runtime_error("Not implemented");
}

} // namespace Wayland
