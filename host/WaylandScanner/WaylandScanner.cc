#include <algorithm>
#include <expected>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

#include <cstddef>
#include <cstdlib>

#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "Application.hh"

#include "Format.hh"
#include "ParseProtocol.hh"

namespace {

struct JsonModeArgs
{
    std::string proto_file_name;
};

std::expected<JsonModeArgs, std::string>
    parse_json_mode_args(std::vector<std::string> &args)
{

    auto json_flag_it = std::ranges::find(args, "--json");
    if (json_flag_it == std::end(args)) {
        return std::unexpected("No --json flag");
    }

    if (args.size() != 2) {
        std::string message =
            "Expected only one [<filename>] argument with --json flag";
        message += std::format(", got {} instead", args.size() - 1);
        return std::unexpected(std::move(message));
    }
    args.erase(json_flag_it);

    std::string input_proto_filename = *args.begin();
    args.clear();

    return JsonModeArgs{input_proto_filename};
}

} // namespace

int Application::main()
{
    auto args = get_libc_args().value();

    if (args.argv.size() == 0) {
        std::cout << "Hacked args";
        return EXIT_FAILURE;
    }

    args.argv.erase(args.argv.begin());

    auto json_mode_args_op = parse_json_mode_args(args.argv);
    if (!json_mode_args_op) {
        std::cout << "JSON Mode: " << json_mode_args_op.error() << '\n';
        return EXIT_FAILURE;
    }
    auto &json_mode_args = json_mode_args_op.value();

    std::string protocol_xml = [&]() {
        std::ifstream file{json_mode_args.proto_file_name};
        file.exceptions(std::ifstream::failbit);
        file.exceptions(std::ifstream::badbit);
        std::stringstream content;
        content << file.rdbuf();
        return content.str();
    }();

    auto interfaces = Wayland::parse_protocol(protocol_xml);

    std::cout << std::format("{}\n", FormatVectorWrap{interfaces});

    return EXIT_SUCCESS;
}
