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
#include "Types.hh"

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
        size_t nb_json_mod_flags = args.size() - 1;

        std::string message;
        message += "Expected";
        if (nb_json_mod_flags != 0) {
            message += " only one";
        }
        message += " <protocol_file> argument with --json flag";
        if (nb_json_mod_flags != 0) {
            message +=
                std::format(", got {} arguments instead", nb_json_mod_flags);
        }
        return std::unexpected(std::move(message));
    }
    args.erase(json_flag_it);

    std::string input_proto_filename = *args.begin();
    args.clear();

    return JsonModeArgs{input_proto_filename};
}

std::string read_file(const std::string &filename)
{
    std::ifstream file{filename};
    file.exceptions(std::ifstream::failbit);
    file.exceptions(std::ifstream::badbit);
    std::stringstream content;
    content << file.rdbuf();
    return content.str();
}

void process_json_mode(const JsonModeArgs &args)
{
    std::string protocol_xml = read_file(args.proto_file_name);
    auto protocol_op = Wayland::parse_protocol(protocol_xml);
    if (!protocol_op) {
        std::cerr << protocol_op.error();
        return;
    }
    auto &protocol = protocol_op.value();

    std::cout << std::format("{}\n", protocol);
}

struct EnumsModeArgs
{
    std::string proto_file_name;
    std::string output_file_name;
};

std::expected<EnumsModeArgs, std::string>
    parse_enums_mode(std::vector<std::string> args)
{
    auto flag_it = std::ranges::find(args, "--enums");
    if (flag_it == std::end(args)) {
        return std::unexpected("No --enums flag");
    }
    args.erase(flag_it);

    if (args.size() != 2) {
        std::string message;
        message += "Expected only";
        message += " <protocol_file> and <output_file> arguments";
        message += " with --enums flag";

        std::vector<std::string> dec_args;
        std::ranges::copy(args, std::back_inserter(dec_args));
        for (auto &dec_arg : dec_args) {
            dec_arg = std::format("({})", dec_arg);
        }
        FormatVectorWrap args_f{dec_args};

        message += std::format(", got {} instead", args_f);

        return std::unexpected(std::move(message));
    }

    EnumsModeArgs out{};

    out.proto_file_name = args.at(0);
    out.output_file_name = args.at(1);

    return out;
}

std::vector<std::string> emit_enum(const Wayland::ScannerTypes::Enum &e)
{
    std::vector<std::string> out;

    out.emplace_back(std::format("enum class {}", e.name));
    out.emplace_back("{");
    out.emplace_back("};");

    return out;
}

void process_enums_mode([[maybe_unused]] const EnumsModeArgs &args)
{
    std::string protocol_xml = read_file(args.proto_file_name);
    std::ofstream output_file{args.output_file_name};
    output_file.exceptions(std::ifstream::failbit);
    output_file.exceptions(std::ifstream::badbit);
    auto protocol_op = Wayland::parse_protocol(protocol_xml);
    if (!protocol_op) {
        std::cerr << protocol_op.error();
        return;
    }
    auto &protocol = protocol_op.value();
    auto &interfaces = protocol.interfaces;

    std::vector<std::string> o_lines;
    o_lines.emplace_back("#pragma once");
    o_lines.emplace_back("");

    bool first_interface = true;
    for (auto &interface : interfaces) {
        if (!first_interface) {
            o_lines.emplace_back("");
        }
        first_interface = false;

        o_lines.emplace_back(std::format("namespace {}", interface.name));
        o_lines.emplace_back("{");

        for (auto &eenum : interface.enums) {
            auto enum_lines = emit_enum(eenum);

            for (auto &enum_line : enum_lines) {
                o_lines.emplace_back(std::move(enum_line));
            }
        }

        o_lines.emplace_back("}");
    }

    std::string output;
    for (auto &o_line : o_lines) {
        output += o_line;
        output += '\n';
    }

    output_file << output;
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

    std::vector<std::string> failue_msgs;

    auto json_mode_args_op = parse_json_mode_args(args.argv);
    if (json_mode_args_op) {
        process_json_mode(json_mode_args_op.value());
        return EXIT_SUCCESS;
    }
    std::string json_mode_message =
        std::format("JSON Mode: [{}]", json_mode_args_op.error());
    failue_msgs.emplace_back(std::move(json_mode_message));

    auto enums_mode_args_op = parse_enums_mode(args.argv);
    if (enums_mode_args_op) {
        process_enums_mode(enums_mode_args_op.value());
        return EXIT_SUCCESS;
    }
    std::string enums_mode_message =
        std::format("ENUMS Mode: [{}]", enums_mode_args_op.error());
    failue_msgs.emplace_back(std::move(enums_mode_message));

    std::cout << "Failue:" << '\n';
    for (auto &msg : failue_msgs) {
        std::cout << msg << '\n';
    }
    return EXIT_FAILURE;
}
