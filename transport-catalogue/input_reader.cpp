#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <cassert>
#include "input_reader.h"

namespace tc {

namespace reader {
//
// read data from stream and insert it in TransportCatalogue
//
void Input::Process(std::istream &in, TransportCatalogue &catalog) {
    // read number of configuration lines
    int records;
    in >> records;
    // remember all Bus strings for future processing after all BusStops became known
    std::vector<std::string> bus_buffer;
    bus_buffer.reserve(records);
    // read line buffer
    std::string line;
    std::getline(in, line); // eat endl of first line

    // distances info for
    DistanceInfoVector distances; // vector of pairs <name, distance_dinfo >

    for (int i = 0; i < records; ++i) {
        std::getline(in, line);
        if (auto pos = line.find("Bus "); pos == 0) {
            // store bus line in buffer for future processing
            bus_buffer.push_back(std::move(line));
        } else if (auto pos = line.find("Stop "); pos == 0) {
            catalog.AddBusStop(ReadBusStop(line, distances));
        }
    }
    // add distance information to catalog
    for (const auto& [name, distance_info] : distances) {
        catalog.SetSegmentDistance(name, distance_info.destination, distance_info.distance);
    }
    // all Bus stops added to catalog
    // now we need to add all busses from string buffer
    for (const auto &bus_line : bus_buffer) {
        catalog.AddBus(ReadBus(bus_line, catalog));
    }
}

BusStop Input::ReadBusStop(const std::string &line, DistanceInfoVector &distances) {
    // read bus stop "Stop X: latitude, longitude, D1m to stop1, D2m to stop2, ...
    using namespace std::string_literals;
    std::string name;

    std::stringstream str_in(line);
    str_in >> name; // read word "Stop"
    assert(name == "Stop"s);
    while (str_in.peek() == ' ') { // read all spaces
        str_in.get();
    }
    std::getline(str_in, name, ':'); // read busStop name up to ':'

    double latitude;
    double longtitude;
    char ch;
    // read coordinates
    str_in >> latitude >> ch >> longtitude;

    // extraction distances info
    if (str_in >> ch && ch == ',') { // if ',' than distance info exists
        detail::DistanceInfo distance_info;
        while (str_in.good()) {
            str_in >> distance_info;
            distances.emplace_back(name, distance_info); // remember distance info in vector for future process
            str_in >> ch; // read ','
        }
    }

    return {name, {latitude, longtitude}};
}

Bus Input::ReadBus(const std::string &line, const TransportCatalogue &catalog) {
    // read bus from line : "Bus X:stop1 - stop2 - ... stopN"
    //                      "Bus X:stop1 > stop2 > ... > stopN > stop1"
    using namespace std::string_literals;
    std::string name;

    std::stringstream str_in(line);
    str_in >> name; // "Bus"
    assert(name == "Bus"s);
    while (str_in.peek() == ' ') { // eat spaces
        str_in.get();
    }
    std::getline(str_in, name, ':'); // name of bus has been read
    while (str_in.peek() == ' ') { // eat spaces
        str_in.get();
    }

    char delimiter = 0; // delimiter busStops names. '>' or  '-'

    Bus bus(name);

    // now we use variable name as BusStop name buffer;
    char ch;
    name.resize(0);
    name.reserve(line.size());
    // read BusStop names and find delimiter of names
    while (str_in.get(ch)) {
        // if we don't know busStop name delimiter - remember it
        if (delimiter == 0 && (ch == '-' || ch == '>')) {
            delimiter = ch;
        }
        // if char if not delimiter - remember it
        if (delimiter == 0 || (delimiter != 0 && ch != delimiter)) {
            name.push_back(ch);
        } else { // char if delimiter - than busStop name has been read - remember it in bus info
            bus.AddBusStop(catalog.GetBusStop(detail::trimSpaces(name)));
            name.resize(0); // reset buffer for next busStop
        }
    } // while
      // process last busStop name in line
    name = detail::trimSpaces(name);
    if (name.length() > 0) {
        bus.AddBusStop(catalog.GetBusStop(name));
    }
    //set the bus type LINEAR or CIRCULAR according to the delimiter symbol
    BusType type = delimiter == '>' ? BusType::CIRCULAR : BusType::LINEAR;
    bus.SetType(type);

    return bus;
}

}    // namespace reader{

} // namespace tc
