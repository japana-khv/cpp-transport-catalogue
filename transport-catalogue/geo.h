#pragma once

#include <cmath>

namespace tc {

namespace geo {

const double EARTH_RADIUS = 6371000.0;

struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates &other) const {
        return lat == other.lat && lng == other.lng;
    }
    bool operator!=(const Coordinates &other) const {
        return !(*this == other);
    }
};

double ComputeDistance(Coordinates from, Coordinates to);

} //namespace geo

} // namespace tc
