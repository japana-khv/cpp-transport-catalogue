#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace tc {

namespace detail {

// trim spaces from begin and end of string_view
std::string_view trimSpaces(std::string_view line);

// distance info block from BusStop configuration
struct DistanceInfo {
    size_t distance = 0; // distance to destination BusStop
    std::string destination; // destination BusStop name
};

// read BusStop info from line
std::istream& operator>>(std::istream &is, DistanceInfo &di);

} //namespace detail

using DistanceInfoVector = std::vector<std::pair<std::string, detail::DistanceInfo>>;

} // namespace tc
