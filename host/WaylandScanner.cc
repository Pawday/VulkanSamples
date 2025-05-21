#include <array>
#include <charconv>
#include <expected>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <expat.h>
#include <expat_external.h>

#include "Application.hh"

template <typename T>
struct FormatVectorWrap
{
    const std::vector<T> &val;
};

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

struct FormatterNoParseArgs
{
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext &ctx)
    {
        if (*ctx.begin() != '}') {
            throw std::format_error("Unexpected arguments");
        }

        return ctx.begin();
    }
};

template <typename T>
struct std::formatter<FormatVectorWrap<T>> : FormatterNoParseArgs
{
    template <class FmtContext>
    FmtContext::iterator format(FormatVectorWrap<T> s, FmtContext &ctx) const
    {
        bool first = true;
        std::format_to(ctx.out(), "[");
        for (auto &el : s.val) {
            if (!first) {
                std::format_to(ctx.out(), ",");
            }
            first = false;

            std::format_to(ctx.out(), "{}", el);
        }
        std::format_to(ctx.out(), "]");
        return ctx.out();
    }
};

template <>
struct std::formatter<WaylandArgType> : FormatterNoParseArgs
{
    template <typename FmtContext>
    struct WaylandArgTypeNameVisitor
    {
        explicit WaylandArgTypeNameVisitor(FmtContext &ctx) : _ctx{ctx}
        {
        }
#define ADD_OVERLOAD(TYPENAME, type_print_name)                                \
    auto operator()(const TYPENAME &)->FmtContext::iterator                    \
    {                                                                          \
        std::format_to(_ctx.out(), "{{\"name\":\"{}\"}}", type_print_name);    \
        return _ctx.out();                                                     \
    }

        ADD_OVERLOAD(WaylandArgTypeInt, "int")
        ADD_OVERLOAD(WaylandArgTypeUInt, "uint")

        auto operator()(const WaylandArgTypeUIntEnum &v) -> FmtContext::iterator
        {
            std::format_to(_ctx.out(), "{{");
            std::format_to(_ctx.out(), "\"name\":\"enum\"");
            if (v.interface_name) {
                std::format_to(_ctx.out(), ",");
                std::format_to(
                    _ctx.out(),
                    "\"interface\":\"{}\"",
                    v.interface_name.value());
            }
            std::format_to(_ctx.out(), ",");
            std::format_to(_ctx.out(), "\"enum_name\":\"{}\"", v.name);
            std::format_to(_ctx.out(), "}}");
            return _ctx.out();
        }

        ADD_OVERLOAD(WaylandArgTypeFixed, "fixed")
        ADD_OVERLOAD(WaylandArgTypeString, "string")
        ADD_OVERLOAD(WaylandArgTypeNullString, "?str")
        ADD_OVERLOAD(WaylandArgTypeObject, "obj")
        ADD_OVERLOAD(WaylandArgTypeNullObject, "?obj")
        ADD_OVERLOAD(WaylandArgTypeNewID, "id")
        ADD_OVERLOAD(WaylandArgTypeArray, "arr")
        ADD_OVERLOAD(WaylandArgTypeFD, "fd")
#undef ADD_OVERLOAD

      private:
        FmtContext &_ctx;
    };

    template <class FmtContext>
    FmtContext::iterator
        format(const WaylandArgType &type, FmtContext &ctx) const
    {
        WaylandArgTypeNameVisitor vis{ctx};
        return std::visit(vis, type);
    }
};

template <>
struct std::formatter<WaylandArg> : FormatterNoParseArgs
{
    template <class FmtContext>
    FmtContext::iterator format(const WaylandArg &s, FmtContext &ctx) const
    {
        std::format_to(ctx.out(), "{{");
        std::format_to(ctx.out(), "\"name\":\"{}\"", s.name);
        std::format_to(ctx.out(), ",");
        std::format_to(ctx.out(), "\"type\":{}", s.type);
        std::format_to(ctx.out(), "}}");
        return ctx.out();
    }
};

template <>
struct std::formatter<WaylandEnum::Entry> : FormatterNoParseArgs
{
    template <class FmtContext>
    FmtContext::iterator
        format(const WaylandEnum::Entry &entry, FmtContext &ctx) const
    {
        std::format_to(
            ctx.out(),
            "{{\"name\":\"{}\",\"value\":{}}}",
            entry.name,
            entry.value);
        return ctx.out();
    }
};

template <>
struct std::formatter<WaylandEnum> : FormatterNoParseArgs
{
    template <class FmtContext>
    FmtContext::iterator format(const WaylandEnum &s, FmtContext &ctx) const
    {
        FormatVectorWrap<WaylandEnum::Entry> fmt_entries(s.entries);
        std::format_to(
            ctx.out(),
            "{{\"name\":\"{}\",\"entries\":{}}}",
            s.name,
            fmt_entries);
        return ctx.out();
    }
};

const char *to_string(WaylandMessage::Type t)
{
    switch (t) {
    case WaylandMessage::Type::DESTRUCTOR:
        return "DESTRUCTOR";
    }
    return "?";
}

template <>
struct std::formatter<WaylandMessage> : FormatterNoParseArgs
{
    template <class FmtContext>
    FmtContext::iterator format(const WaylandMessage &s, FmtContext &ctx) const
    {
        std::format_to(ctx.out(), "{{");
        std::format_to(ctx.out(), "\"name\":\"{}\"", s.name);
        if (s.type) {
            std::format_to(ctx.out(), ",");
            std::format_to(
                ctx.out(), "\"type\":\"{}\"", to_string(s.type.value()));
        }
        FormatVectorWrap<WaylandArg> arg_fmt{s.args};
        std::format_to(ctx.out(), ",");
        std::format_to(ctx.out(), "\"args\":{}", arg_fmt);
        std::format_to(ctx.out(), "}}");
        return ctx.out();
    }
};

template <>
struct std::formatter<WaylandRequest> : FormatterNoParseArgs
{
    template <class FmtContext>
    FmtContext::iterator format(const WaylandRequest &s, FmtContext &ctx) const
    {
        std::format_to(ctx.out(), "{}", static_cast<const WaylandMessage &>(s));
        return ctx.out();
    }
};

template <>
struct std::formatter<WaylandEvent> : FormatterNoParseArgs
{
    template <class FmtContext>
    FmtContext::iterator format(const WaylandEvent &s, FmtContext &ctx) const
    {
        std::format_to(ctx.out(), "{}", static_cast<const WaylandMessage &>(s));
        return ctx.out();
    }
};

template <>
struct std::formatter<WaylandInterface> : FormatterNoParseArgs
{
    template <class FmtContext>
    FmtContext::iterator
        format(const WaylandInterface &i, FmtContext &ctx) const
    {
        std::format_to(ctx.out(), "{{");
        std::format_to(ctx.out(), "\"name\":\"{}\"", i.name);
        std::format_to(ctx.out(), ",");
        std::format_to(ctx.out(), "\"version\":{}", i.verison);
        FormatVectorWrap<WaylandRequest> request_fmt{i.requests};
        std::format_to(ctx.out(), ",");
        std::format_to(ctx.out(), "\"requests\":{}", request_fmt);
        FormatVectorWrap<WaylandEvent> event_fmt{i.events};
        std::format_to(ctx.out(), ",");
        std::format_to(ctx.out(), "\"events\":{}", event_fmt);
        FormatVectorWrap<WaylandEnum> enums_fmt{i.enums};
        std::format_to(ctx.out(), ",");
        std::format_to(ctx.out(), "\"enums\":{}", enums_fmt);
        std::format_to(ctx.out(), "}}");
        return ctx.out();
    }
};

struct Parser
{
    Parser()
    {
        handle = []() {
            auto parser = XML_ParserCreate(nullptr);
            if (!parser) {
                throw std::runtime_error("Cannot init parser");
            }
            return parser;
        }();
    }

    struct Attribute
    {
        std::string_view key;
        std::string_view value;
    };

    template <typename UserDataT>
    struct Callbacks
    {
        UserDataT &user_data;
        void (UserDataT::*start)(
            std::string_view el, const std::vector<Attribute> &attrs);
        void (UserDataT::*data)(std::string_view data);
        void (UserDataT::*end)(std::string_view el);
    };

    template <typename UserDataT>
    void parse(Callbacks<UserDataT> &callbacks, std::string_view data)
    {
        struct Context
        {
            Callbacks<UserDataT> &cb;
            std::vector<Attribute> attrs;
        } context{callbacks, {}};

        XML_ParserReset(handle, nullptr);
        XML_SetUserData(handle, &context);
        XML_SetCharacterDataHandler(
            handle, [](void *userData, const XML_Char *s, int len) {
                Context &ctx = *reinterpret_cast<Context *>(userData);
                (ctx.cb.user_data.*ctx.cb.data)(std::string_view{s, s + len});
            });
        XML_SetElementHandler(
            handle,
            [](void *userData, const XML_Char *name, const XML_Char **atts_p) {
                Context &ctx = *reinterpret_cast<Context *>(userData);

                auto &attrs = ctx.attrs;
                ctx.attrs.clear();

                size_t attr_offset = 0;
                while (atts_p[attr_offset]) {
                    std::string_view key{atts_p[attr_offset]};
                    std::string_view val{atts_p[attr_offset + 1]};
                    attr_offset += 2;
                    attrs.emplace_back(key, val);
                }

                (ctx.cb.user_data.*ctx.cb.start)(std::string_view{name}, attrs);
            },
            [](void *userData, const XML_Char *name) {
                Context &ctx = *reinterpret_cast<Context *>(userData);
                (ctx.cb.user_data.*ctx.cb.end)(std::string_view{name});
            });

        XML_Status status = XML_Parse(handle, data.data(), data.size(), true);
        if (status != XML_STATUS_OK) {
            XML_Error error = XML_GetErrorCode(handle);
            std::string message =
                std::format("XML_Parser error: ({})", XML_ErrorString(error));
            throw std::runtime_error(std::move(message));
        }
    }

    ~Parser()
    {
        if (handle) {
            XML_ParserFree(handle);
        }
    }

    Parser(Parser &&) = delete;
    Parser &operator=(Parser &&) = delete;
    Parser(const Parser &) = delete;
    Parser &operator=(const Parser &) = delete;

  private:
    XML_Parser handle = nullptr;
};

struct WaylandProtoParser
{
    // clang-format off
    using ParseTarget = std::variant<
        WaylandArg,
        WaylandEnum,
        WaylandEnum::Entry,
        WaylandMessage,
        WaylandRequest,
        WaylandEvent,
        WaylandInterface
    >;
    // clang-format on

    using AttributeMap = std::unordered_map<std::string, std::string>;

    auto make_attr_map(const std::vector<Parser::Attribute> &attrs)
        -> AttributeMap
    {
        AttributeMap out;

        for (auto &attr : attrs) {
            std::string key{attr.key};
            std::string val{attr.value};
            if (out.contains(key)) {
                std::string message = std::format(
                    "Duplicate interface attribute [{}=[{}]]", key, val);
                throw std::runtime_error(std::move(message));
            }
            out[std::move(key)] = std::move(val);
        }

        return out;
    };

    template <typename T>
    std::expected<T, std::string>
        parse_num(const std::string &str, int base = 10)
    {
        T out = 0;
        auto status =
            std::from_chars(str.data(), str.data() + str.size(), out, base);

        std::optional<std::string> error_string;

        if (!error_string && status.ec != std::errc{}) {
            error_string = std::make_error_code(status.ec).message();
        }

        if (!error_string && status.ptr != str.data() + str.size()) {
            error_string = std::format("Incomplete output [{}]", str);
        }

        if (error_string) {
            return std::unexpected{std::move(error_string.value())};
        }

        return out;
    }

    auto parse_interface(const AttributeMap &attrs) -> void
    {
        WaylandInterface new_interface{};

        std::string name = attrs.at("name");
        new_interface.name = std::move(name);
        std::string vesion_string = attrs.at("version");

        auto version_op =
            parse_num<decltype(WaylandInterface::verison)>(vesion_string);

        std::optional<std::string> error_string{};
        if (!version_op.has_value()) {
            error_string = std::move(version_op.error());
        }

        if (error_string) {
            std::string message = std::format(
                "Cannot parse version string [{}] of interface [{}] : "
                "status "
                "[{}]",
                vesion_string,
                new_interface.name,
                error_string.value());
            throw std::runtime_error(std::move(message));
        }

        new_interface.verison = version_op.value();

        targets.emplace(std::move(new_interface));
    }

    auto fin_interface() -> void
    {
        ParseTarget &active_target = targets.top();
        WaylandInterface &active_interface =
            std::get<WaylandInterface>(active_target);

        interfaces.emplace_back(std::move(active_interface));
        targets.pop();
    }

    auto parse_request(const AttributeMap &attrs) -> void
    {
        WaylandRequest new_request;
        std::string request_name = attrs.at("name");

        if (attrs.contains("type")) {
            std::string type_string = attrs.at("type");
            if (type_string == "destructor") {
                new_request.type = WaylandRequest::Type::DESTRUCTOR;
            } else {
                std::string message =
                    std::format("Unknown request type [{}]", type_string);
                throw std::runtime_error(std::move(message));
            }
        }

        new_request.name = std::move(request_name);
        targets.emplace(std::move(new_request));
    }

    struct FinRequestVisitor
    {
        explicit FinRequestVisitor(WaylandRequest &req) : _req(req)
        {
        }

        template <typename T>
        void operator()(T &t)
        {
            std::string message = std::format(
                "Attempt to add request field to {}", target_name(t));
            throw std::runtime_error(std::move(message));
        }

        void operator()(WaylandInterface &interface)
        {
            interface.requests.emplace_back(std::move(_req));
        }

      private:
        WaylandRequest &_req;
    };

    auto fin_request() -> void
    {
        ParseTarget &active_target = targets.top();
        WaylandRequest request =
            std::move(std::get<WaylandRequest>(active_target));
        targets.pop();

        ParseTarget &request_parent_target = targets.top();
        std::visit(FinRequestVisitor{request}, request_parent_target);
    }

    auto parse_event(const AttributeMap &attrs) -> void
    {
        WaylandEvent new_event;
        std::string event_name = attrs.at("name");

        new_event.name = std::move(event_name);
        targets.emplace(std::move(new_event));
    }

    struct FinEventVisitor
    {
        explicit FinEventVisitor(WaylandEvent &ev) : _ev(ev)
        {
        }

        template <typename T>
        void operator()(T &t)
        {
            std::string message = std::format(
                "Attempt to add request field to {}", target_name(t));
            throw std::runtime_error(std::move(message));
        }

        void operator()(WaylandInterface &interface)
        {
            interface.events.emplace_back(std::move(_ev));
        }

      private:
        WaylandEvent &_ev;
    };

    auto fin_event() -> void
    {
        ParseTarget &active_target = targets.top();
        WaylandEvent event = std::move(std::get<WaylandEvent>(active_target));
        targets.pop();

        ParseTarget &request_parent_target = targets.top();
        std::visit(FinEventVisitor{event}, request_parent_target);
    }

    auto parse_arg_type(
        [[maybe_unused]] const std::string &arg_type_string,
        [[maybe_unused]] const AttributeMap &attrs)
        -> std::expected<WaylandArgType, std::string>
    {

        /*
         * * * `i`: int
         * * `u`: uint
         * * `f`: fixed
         * * `s`: string
         * * `o`: object
         * * `n`: new_id
         * * `a`: array
         * * `h`: fd
         * * `?`: following argument (`o` or `s`) is nullable
         */

        if (arg_type_string == "int") {
            return WaylandArgTypeInt{};
        }

        if (arg_type_string == "uint") {

            if (!attrs.contains("enum")) {
                return WaylandArgTypeUInt{};
            }

            /*
             * "<interface_name>.<enum_name>"
             * or
             * "<enum_name>"
             */
            std::string enum_location = attrs.at("enum");

            bool has_interface_name =
                enum_location.find(".") != enum_location.npos;

            if (!has_interface_name) {
                WaylandArgTypeUIntEnum out{};
                out.name = std::move(enum_location);
                return out;
            }

            auto split_dot = [](const std::string &s)
                -> std::
                    expected<std::pair<std::string, std::string>, std::string> {
                        std::string first;
                        bool found_sep = false;
                        std::string second;

                        for (auto c : s) {
                            if (c == '.') {
                                found_sep = true;
                                continue;
                            }

                            if (!found_sep) {
                                first += c;
                                continue;
                            }

                            second += c;
                        }

                        if (!found_sep) {
                            std::string message = std::format(
                                "Cannot split string [{}] by dot", s);
                            return std::unexpected(std::move(message));
                        }

                        return std::make_pair(
                            std::move(first), std::move(second));
                    };

            auto sep_op = split_dot(enum_location);
            if (!sep_op.has_value()) {
                return std::unexpected(sep_op.error());
            }
            auto &sep = sep_op.value();

            WaylandArgTypeUIntEnum out{};
            out.interface_name = std::move(sep.first);
            out.name = std::move(sep.second);
            return out;
        }

        if (arg_type_string == "fixed") {
            return WaylandArgTypeFixed{};
        }

        if (arg_type_string == "string" || arg_type_string == "object") {

            WaylandArgType out_t;
            WaylandArgType null_out_t;

            if (arg_type_string == "string") {
                out_t = WaylandArgTypeString{};
                null_out_t = WaylandArgTypeNullString{};
            } else if (arg_type_string == "object") {
                out_t = WaylandArgTypeObject{};
                null_out_t = WaylandArgTypeNullObject{};
            } else {
                std::string message = std::format(
                    "Unexpected arg_type_string value change from"
                    " \"string\" or \"\" to [{}]",
                    arg_type_string);
                return std::unexpected(std::move(message));
            }

            if (!attrs.contains("allow-null")) {
                return out_t;
            }

            std::string allow_null_value = attrs.at("allow-null");
            if (allow_null_value != "true") {
                std::string message = std::format(
                    "for tag <arg> \"allow-null\" attribute value must be set "
                    "to \"true\", got [{}] instead",
                    allow_null_value);
                return std::unexpected(std::move(message));
            }

            return null_out_t;
        }

        if (arg_type_string == "new_id") {
            return WaylandArgTypeNewID{};
        }

        if (arg_type_string == "array") {
            return WaylandArgTypeArray{};
        }

        if (arg_type_string == "fd") {
            return WaylandArgTypeFD{};
        }

        return std::unexpected(
            std::format("[{}] is unknown type", arg_type_string));
    }

    auto parse_arg(const AttributeMap &attrs)
    {
        WaylandArg arg;
        std::string name = attrs.at("name");
        arg.name = std::move(name);

        std::string type_string = attrs.at("type");
        auto arg_type = parse_arg_type(type_string, attrs);
        if (!arg_type) {
            std::string message = std::format(
                "Parsing [{}] type failure [{}]",
                type_string,
                arg_type.error());
            throw std::runtime_error(std::move(message));
        }
        arg.type = std::move(arg_type.value());

        targets.emplace(std::move(arg));
    }

    struct FinArgVisitor
    {
        explicit FinArgVisitor(WaylandArg &arg) : _arg{arg}
        {
        }

        template <typename T>
        void operator()(T &t)
        {
            std::string message = std::format(
                "Attempt to add argument field to {}", target_name(t));
            throw std::runtime_error(std::move(message));
        }

        void operator()(WaylandRequest &req)
        {
            req.args.emplace_back(std::move(_arg));
        }

        void operator()(WaylandEvent &event)
        {
            event.args.emplace_back(std::move(_arg));
        }

      private:
        WaylandArg &_arg;
    };

    auto fin_arg()
    {
        ParseTarget &active_target = targets.top();
        WaylandArg arg_target = std::move(std::get<WaylandArg>(active_target));
        targets.pop();

        ParseTarget &request_parent_target = targets.top();
        std::visit(FinArgVisitor{arg_target}, request_parent_target);
    }

    auto parse_enum(const AttributeMap &attrs)
    {
        WaylandEnum new_enum{};
        if (!attrs.contains("name")) {

            std::string message = "Found unnamed enum tag";
            if (targets.size() != 0) {
                auto &parent = targets.top();
                message = std::format("{} in {}", message, target_name(parent));
            }
            throw std::runtime_error{std::move(message)};
        }
        new_enum.name = attrs.at("name");
        targets.emplace(std::move(new_enum));
    }

    void fin_enum()
    {
        ParseTarget &active_target = targets.top();
        WaylandEnum enum_target =
            std::move(std::get<WaylandEnum>(active_target));
        targets.pop();

        ParseTarget &interface_parent_target = targets.top();
        WaylandInterface &interface =
            std::get<WaylandInterface>(interface_parent_target);

        interface.enums.emplace_back(std::move(enum_target));
    }

    auto parse_entry(const AttributeMap &attrs)
    {
        WaylandEnum::Entry entry{};

        std::string name = attrs.at("name");
        std::string value_string = attrs.at("value");

        bool is_hex = true;
        is_hex = is_hex && value_string.size() > 2;
        is_hex = is_hex && value_string[0] == '0';
        is_hex = is_hex && (value_string[1] == 'x' || value_string[1] == 'X');

        int base = 10;
        if (is_hex) {
            base = 16;
            value_string = value_string.substr(2);
        }

        auto value_op =
            parse_num<decltype(WaylandEnum::Entry::value)>(value_string, base);

        std::optional<std::string> error_string{};
        if (!value_op.has_value()) {
            error_string = std::move(value_op.error());
        }

        if (error_string) {
            std::string message = std::format(
                "Cannot parse version string [{}] of entry [{}] : "
                "status "
                "[{}]",
                value_string,
                name,
                error_string.value());
            throw std::runtime_error(std::move(message));
        }

        entry.name = std::move(name);
        entry.value = value_op.value();

        targets.emplace(std::move(entry));
    }

    auto fin_entry()
    {
        ParseTarget &active_target = targets.top();
        WaylandEnum::Entry entry =
            std::move(std::get<WaylandEnum::Entry>(active_target));
        targets.pop();

        ParseTarget &enum_target = targets.top();
        WaylandEnum &wl_enum = std::get<WaylandEnum>(enum_target);

        wl_enum.entries.emplace_back(std::move(entry));
    }

    enum class TagEvent
    {
        START,
        END
    };

    using OptionalAttributeArray = std::optional<
        std::reference_wrapper<const std::vector<Parser::Attribute>>>;

    void dispatch_tag(
        TagEvent event,
        std::string_view tag,
        OptionalAttributeArray attrs = std::nullopt)
    {

        bool need_attrs_map = event == TagEvent::START;

        std::string_view known_tags[]{
            "interface", "request", "event", "arg", "enum", "entry"};
        if (need_attrs_map) {
            bool known = false;
            for (auto s : known_tags) {
                if (tag == s) {
                    known = true;
                    break;
                }
            }
            need_attrs_map = known;
        }

        auto attrs_map = [&attrs, this, &need_attrs_map]()
            -> std::optional<const AttributeMap> {
            if (attrs && need_attrs_map) {
                return make_attr_map(attrs.value());
            }
            return std::nullopt;
        }();

        if (tag == "interface") {
            switch (event) {
            case TagEvent::START:
                parse_interface(attrs_map.value());
                break;
            case TagEvent::END:
                fin_interface();
                break;
            }
            return;
        }

        if (tag == "request") {
            switch (event) {
            case TagEvent::START:
                parse_request(attrs_map.value());
                break;
            case TagEvent::END:
                fin_request();
                break;
            }
            return;
        }

        if (tag == "event") {
            switch (event) {
            case TagEvent::START:
                parse_event(attrs_map.value());
                break;
            case TagEvent::END:
                fin_event();
                break;
            }
            return;
        }

        if (tag == "arg") {
            switch (event) {
            case TagEvent::START:
                parse_arg(attrs_map.value());
                break;
            case TagEvent::END:
                fin_arg();
                break;
            }
            return;
        }

        if (tag == "enum") {
            switch (event) {
            case TagEvent::START:
                parse_enum(attrs_map.value());
                break;
            case TagEvent::END:
                fin_enum();
                break;
            }
            return;
        }

        if (tag == "entry") {
            switch (event) {
            case TagEvent::START:
                parse_entry(attrs_map.value());
                break;
            case TagEvent::END:
                fin_entry();
                break;
            }
            return;
        }
    }

    void
        start(std::string_view tag, const std::vector<Parser::Attribute> &attrs)
    {
        dispatch_tag(TagEvent::START, tag, std::ref(attrs));
    }

    auto data([[maybe_unused]] std::string_view data) -> void
    {
    }

    auto end(std::string_view tag) -> void
    {
        dispatch_tag(TagEvent::END, tag);
    }

    auto get() const -> const std::vector<WaylandInterface> &
    {
        return interfaces;
    };

  private:
    struct ParseTargetNameVisitor
    {
#define ADD_OVERLOAD(TYPENAME, TAG_NAME)                                       \
    std::string operator()(const TYPENAME &tgt)                                \
    {                                                                          \
        return std::format(                                                    \
            "ParseTarget::{} (<{} name=[{}] ...>)",                            \
            #TYPENAME,                                                         \
            #TAG_NAME,                                                         \
            tgt.name);                                                         \
    }
        ADD_OVERLOAD(WaylandArg, arg)
        ADD_OVERLOAD(WaylandEnum, enum)
        ADD_OVERLOAD(WaylandEnum::Entry, entry)
        ADD_OVERLOAD(WaylandMessage, event | request)
        ADD_OVERLOAD(WaylandRequest, request)
        ADD_OVERLOAD(WaylandEvent, event)
        ADD_OVERLOAD(WaylandInterface, interface)
#undef ADD_OVERLOAD
    };

    static std::string target_name(const ParseTarget &tgt)
    {
        return std::visit(ParseTargetNameVisitor{}, tgt);
    }

    std::vector<WaylandInterface> interfaces;

    std::stack<ParseTarget> targets;
};

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
