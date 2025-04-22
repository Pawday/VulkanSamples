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
    alignas(32) char _[1024];
};
