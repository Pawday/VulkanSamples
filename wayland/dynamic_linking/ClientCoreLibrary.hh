#pragma once

#include "Wayland/Data.hh"

#include "ClientCore.hh"

namespace Wayland {

struct ClientCoreLibrary : ClientCore
{
    ClientCoreLibrary();
    ClientCoreLibrary(ClientCoreLibrary &&);
    ClientCoreLibrary &operator=(ClientCoreLibrary &&);
    ~ClientCoreLibrary();

    ClientCoreLibrary(const ClientCoreLibrary &) = delete;
    ClientCoreLibrary &operator=(const ClientCoreLibrary &) = delete;

    using ImplT = Data<16, 16>;

  private:
    ImplT impl;
};

} // namespace Wayland
