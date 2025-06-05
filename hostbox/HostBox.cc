#include <expected>
#include <format>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include <cstdlib>
#include <vector>

#include <wl_gena/GenaMain.hh>

#include "Application.hh"

namespace {

// clang-format off
struct HostToolType
{
    struct wl_gena {};
};

using HostTool = std::variant<
    HostToolType::wl_gena
>;
// clang-format on

std::expected<HostTool, std::string> host_tool_from_name(std::string_view str)
{
    if (str == "wl_gena") {
        return HostToolType::wl_gena{};
    }

    return std::unexpected{std::format("Unknown host tool [{}]", str)};
}

struct HostToolDriver
{
    explicit HostToolDriver(std::vector<std::string> &&argv)
        : _argv(std::move(argv))
    {
    }

    int operator()(HostToolType::wl_gena)
    {
        return wl_gena_main(_argv);
    }

  private:
    std::vector<std::string> _argv;
};

} // namespace

int Application::main()
{
    auto argv = get_libc_args().value().argv;
    if (argv.size() < 2) {
        std::cerr << "No args" << '\n';
        return EXIT_FAILURE;
    }

    argv.erase(std::begin(argv));
    std::string tool_name = argv[0];
    argv.erase(std::begin(argv));

    auto host_tool_op = host_tool_from_name(tool_name);
    if (!host_tool_op.has_value()) {
        std::cerr << host_tool_op.error() << '\n';
        return EXIT_FAILURE;
    }

    return std::visit(HostToolDriver{std::move(argv)}, host_tool_op.value());

    return EXIT_SUCCESS;
}
