#pragma once

struct WaylandContext
{
    WaylandContext();
    WaylandContext(const WaylandContext &) = delete;
    WaylandContext(WaylandContext &&);
    WaylandContext &operator=(const WaylandContext &) = delete;
    WaylandContext &operator=(WaylandContext &&);
    ~WaylandContext();

  private:
    alignas(8) char _[16];
};
