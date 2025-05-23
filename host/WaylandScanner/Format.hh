#pragma once

#include <format>
#include <vector>

#include "Types.hh"

template <typename T>
struct FormatVectorWrap
{
    const std::vector<T> &val;
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
        std::format_to(ctx.out(), "{{");
        std::format_to(ctx.out(), "\"name\":\"{}\"", entry.name);
        std::format_to(ctx.out(), ",");
        std::format_to(ctx.out(), "\"value\":{}", entry.value);
        if (entry.is_hex) {
            std::format_to(ctx.out(), ",");
            std::format_to(ctx.out(), "\"value_hex\":\"{:x}\"", entry.value);
        }
        std::format_to(ctx.out(), "}}");
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
