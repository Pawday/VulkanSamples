#include <cstdlib>
#include <exception>
#include <iostream>

#include "wayland/WaylandContext.hh"

int main()
try {
    WaylandContext c;

} catch (std::exception &e) {
    std::cout << e.what() << '\n';
    return EXIT_FAILURE;
}
