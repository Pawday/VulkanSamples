#pragma once

#include <algorithm>
#include <format>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include <cstddef>

inline void format_table(
    const std::string &title,
    const std::vector<std::string> &elems,
    std::function<void(std::string_view)> output)
{
    if (elems.empty()) {
        return;
    }

    size_t max_len = elems[0].size();
    max_len = std::max(max_len, title.size() + 2);
    for (auto &ext_name : elems) {
        max_len = std::max(max_len, ext_name.size());
    }
    max_len += 4;

    std::string header = std::format("+{:-^{}}+", title, max_len - 2);
    output(header);

    for (auto &ext_name : elems) {
        std::string ext_f = std::format("| {: <{}} |", ext_name, max_len - 4);
        output(ext_f);
    }

    std::string footer = std::format("+{:-<{}}+", "", max_len - 2);
    output(footer);
}
