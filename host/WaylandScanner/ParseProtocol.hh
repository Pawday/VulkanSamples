#pragma once

#include <string_view>
#include <vector>

#include "Types.hh"

std::vector<WaylandInterface> parse_protocol(std::string_view protocol_xml);
