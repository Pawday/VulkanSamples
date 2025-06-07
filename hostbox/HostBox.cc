#include <array>
#include <expected>
#include <format>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include <cstdlib>
#include <vector>

#include <wl_gena/GenaMain.hh>

#include "Application.hh"

#include "GLSL.hh"

template <typename VariantT>
struct VariantArray_t;

template <typename... T>
struct VariantArray_t<std::variant<T...>>
{
    using VariantT = std::variant<T...>;
    static constexpr std::array<VariantT, sizeof...(T)> value{T{}...};
};

template <typename VariantT>
constexpr auto VariantArray = VariantArray_t<VariantT>::value;

namespace {

// clang-format off
struct HostToolType
{
    struct wl_gena {};
    struct glsl {};
};

using HostTool = std::variant<
    HostToolType::wl_gena,
    HostToolType::glsl
>;
// clang-format on

static constexpr auto host_tool_array = VariantArray<HostTool>;

constexpr std::expected<HostTool, std::string>
    host_tool_from_name(std::string_view str)
{
    struct
    {
#define OVERLOAD(tool_name)                                                    \
    constexpr std::string operator()(HostToolType::tool_name)                  \
    {                                                                          \
        return #tool_name;                                                     \
    }

        OVERLOAD(wl_gena)
        OVERLOAD(glsl)

#undef OVERLOAD

    } vis;

    for (auto &tool : host_tool_array) {
        std::string tool_name = std::visit(vis, tool);
        if (tool_name == str) {
            return tool;
        }
    }

    std::string all_tools;
    all_tools += '[';

    bool first = true;
    for (auto &tool : host_tool_array) {
        std::string tool_name = std::visit(vis, tool);
        if (!first) {
            all_tools += ", ";
        }
        first = false;

        all_tools = all_tools + '[' + tool_name + ']';
    }
    all_tools += ']';

    auto msg = std::format(
        "Unknown host tool [{}] - expected any of {}", str, all_tools);

    return std::unexpected{std::move(msg)};
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

    int operator()(HostToolType::glsl)
    {
        return glsl_main(_argv);
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
}
