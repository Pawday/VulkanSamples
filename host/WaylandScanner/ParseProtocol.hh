#pragma once

#include <string_view>
#include <vector>

#include "Types.hh"

namespace Wayland {

std::vector<ScannerTypes::Interface>
    parse_protocol(std::string_view protocol_xml);
}
