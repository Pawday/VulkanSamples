#include <memory>
#include <stdexcept>
#include <utility>

#include <cstring>

#include <Wayland/Context.hh>

#include "ClientCoreLibrary.hh"

namespace Wayland {

namespace {

struct Impl
{
    std::unique_ptr<ClientCoreLibrary> lib =
        std::make_unique<ClientCoreLibrary>();
};

Impl &cast(Context::ImplT &impl)
{
    return *reinterpret_cast<Impl *>(impl());
}

}; // namespace

Context::Context()
{
    static_assert(sizeof(Context::ImplT) >= sizeof(Impl));
    static_assert(alignof(Context::ImplT) >= alignof(Impl));
    new (impl()) Impl;
}

Context::Context(Context &&o)
{
    new (impl()) Impl{std::move(cast(o.impl))};
}

Context &Context::operator=(Context &&o)
{
    cast(impl).operator=(std::move(cast(o.impl)));
    return *this;
}

Context::~Context()
{
    cast(impl).~Impl();
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
