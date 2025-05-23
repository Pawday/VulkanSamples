#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <cstddef>
#include <cstdlib>

#include <expat.h>
#include <expat_external.h>

#include "Application.hh"

#include "Format.hh"
#include "ParseProtocol.hh"

int Application::main()
{
    std::string protocol_xml = [&]() {
        auto args = get_libc_args().value();
        std::string file_name = args.argv.at(1);

        std::ifstream file{file_name};
        file.exceptions(std::ifstream::failbit);
        file.exceptions(std::ifstream::badbit);
        std::stringstream content;
        content << file.rdbuf();
        return content.str();
    }();

    auto interfaces = parse_protocol(protocol_xml);

    std::cout << std::format("{}\n", FormatVectorWrap{interfaces});

    return EXIT_SUCCESS;
}
