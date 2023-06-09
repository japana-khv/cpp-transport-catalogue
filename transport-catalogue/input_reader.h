#pragma once

#include <iostream>
#include "domain.h"
#include "transport_catalogue.h"

namespace tc {

namespace reader {

// Bus and Bus Stop reader from text files

class Input {
public:
    void Process(std::istream &in, TransportCatalogue &catalog);
private:
    // read BusStop information from string
    BusStop ReadBusStop(const std::string &line, DistanceInfoVector &distances);
    // read Bus information from string
    Bus ReadBus(const std::string &line, const TransportCatalogue &catalog);

};

} // namespace reader

} // namespace tc
