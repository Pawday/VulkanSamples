#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdlib>

#include <expat.h>
#include <expat_external.h>

#include "Application.hh"

#include "Format.hh"
#include "Types.hh"
#include "WaylandProtoParser.hh"

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

    WaylandProtoParser ctx;
    Parser::Callbacks<WaylandProtoParser> pcbs{
        ctx,
        &WaylandProtoParser::start,
        &WaylandProtoParser::data,
        &WaylandProtoParser::end};

    Parser p;
    p.parse(pcbs, protocol_xml);

    std::vector<WaylandInterface> interfaces = ctx.get();

    std::cout << std::format("{}\n", FormatVectorWrap{interfaces});

    return EXIT_SUCCESS;
}
