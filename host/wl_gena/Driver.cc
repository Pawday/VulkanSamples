#include <cstdlib>

#include <iostream>

#include <wl_gena/GenaMain.hh>

#include "Application.hh"

int Application::main()
{
    auto argv = get_libc_args().value().argv;
    if (argv.size() == 0) {
        std::cerr << "No args";
        return EXIT_FAILURE;
    }

    argv.erase(argv.begin());

    return wl_gena_main(argv);
}
