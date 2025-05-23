#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

// clang-format off
struct WaylandArgTypeInt {};
struct WaylandArgTypeUInt {};
struct WaylandArgTypeUIntEnum
{
    std::optional<std::string> interface_name;
    std::string name;
};
struct WaylandArgTypeFixed {};
struct WaylandArgTypeString {};
struct WaylandArgTypeNullString {};
struct WaylandArgTypeObject {};
struct WaylandArgTypeNullObject {};
struct WaylandArgTypeNewID {};
struct WaylandArgTypeArray {};
struct WaylandArgTypeFD {};

using WaylandArgType = std::variant<
    WaylandArgTypeInt,
    WaylandArgTypeUInt,
    WaylandArgTypeUIntEnum,
    WaylandArgTypeFixed,
    WaylandArgTypeString,
    WaylandArgTypeNullString,
    WaylandArgTypeObject,
    WaylandArgTypeNullObject,
    WaylandArgTypeNewID,
    WaylandArgTypeArray,
    WaylandArgTypeFD
>;
// clang-format on

struct WaylandArg
{
    std::string name;
    WaylandArgType type;
};

struct WaylandEnum
{
    std::string name;
    std::string description;
    struct Entry
    {
        std::string name;
        std::string summary;
        uint32_t value;
        bool is_hex;
    };
    std::vector<Entry> entries;
};

struct WaylandMessage
{
    enum class Type
    {
        DESTRUCTOR
    };

    std::string name;
    std::string description;
    std::optional<Type> type;
    std::string summary;
    std::vector<WaylandArg> args;
};

// clang-format off
struct WaylandRequest : WaylandMessage {};
struct WaylandEvent : WaylandMessage {};
// clang-format on

struct WaylandInterface
{
    std::string name;
    uint32_t verison;
    std::string description;
    std::vector<WaylandRequest> requests;
    std::vector<WaylandEvent> events;
    std::vector<WaylandEnum> enums;
};
