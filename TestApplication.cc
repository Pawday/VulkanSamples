#include <cstdlib>
#include <stdexcept>

#include "Application.hh"

int Application::main()
{
    throw std::runtime_error("Hello error");
    return EXIT_SUCCESS;
}

