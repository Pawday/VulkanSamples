#include <iostream>
#include <iterator>
#include <string>

#include <cstdlib>

#include <wl_gena/GenaMain.hh>

#include "Application.hh"

int Application::main()
{
    auto argv = get_libc_args().value().argv;
    if (argv.size() < 1) {
        std::cerr << "No args" << '\n';
        return EXIT_FAILURE;
    }

    argv.erase(std::begin(argv));
    return wl_gena_main(argv);
}
