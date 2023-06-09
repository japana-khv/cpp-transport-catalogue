/*
 * geo.cpp
 *
 *  Created on: May 16, 2023
 *      Author: rvk
 */

#include "geo.h"

namespace tc {

namespace geo {

double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    static const double dr = 3.1415926535 / 180.;

    return acos(
            sin(from.lat * dr) * sin(to.lat * dr)
                    + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr)) * EARTH_RADIUS;
}

}
}
