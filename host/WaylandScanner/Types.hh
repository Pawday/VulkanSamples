#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace Wayland {
namespace ScannerTypes {

// clang-format off
struct ArgTypeInt {};
struct ArgTypeUInt {};
struct ArgTypeUIntEnum
{
    std::optional<std::string> interface_name;
    std::string name;
};
struct ArgTypeFixed {};
struct ArgTypeString {};
struct ArgTypeNullString {};
struct ArgTypeObject {};
struct ArgTypeNullObject {};
struct ArgTypeNewID {};
struct ArgTypeArray {};
struct ArgTypeFD {};

using ArgType = std::variant<
    ArgTypeInt,
    ArgTypeUInt,
    ArgTypeUIntEnum,
    ArgTypeFixed,
    ArgTypeString,
    ArgTypeNullString,
    ArgTypeObject,
    ArgTypeNullObject,
    ArgTypeNewID,
    ArgTypeArray,
    ArgTypeFD
>;
// clang-format on

struct Arg
{
    std::string name;
    ArgType type;
};

struct Enum
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

struct Message
{
    enum class Type
    {
        DESTRUCTOR
    };

    std::string name;
    std::string description;
    std::optional<Type> type;
    std::string summary;
    std::vector<Arg> args;
};

// clang-format off
struct Request : Message {};
struct Event : Message {};
// clang-format on

struct Interface
{
    std::string name;
    uint32_t verison;
    std::string description;
    std::vector<Request> requests;
    std::vector<Event> events;
    std::vector<Enum> enums;
};

} // namespace ScannerTypes
} // namespace Wayland
