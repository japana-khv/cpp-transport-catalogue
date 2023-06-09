#pragma once

#include <iostream>
#include <string>

#include "transport_catalogue.h"

namespace tc {

namespace reader {

class Stat {
public:
    // read queries from stream , process it and output results
    void operator()(std::istream &in, std::ostream &out, TransportCatalogue &catalog);
private:

    void ProcessBusQuery(const std::string &line, std::ostream &out, TransportCatalogue &catalog);

    void ProcessBusStopQuery(const std::string &line, std::ostream &out, TransportCatalogue &catalog);
};

} //namespace reader

} //namespace tc
