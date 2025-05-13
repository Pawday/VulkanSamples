#pragma once

#include <cstddef>

namespace Wayland {

template <size_t S, size_t A = alignof(std::max_align_t)>
struct Data
{
    char *operator()()
    {
        return _;
    };

  private:
    alignas(A) char _[S];
};

} // namespace Wayland
