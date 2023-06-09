#include <string_view>
#include <iomanip>

#include "stat_reader.h"

#include "input_reader.h"

namespace tc {

namespace reader {
// read queries from stream , process it and output results
void Stat::operator ()(std::istream &in, std::ostream &out, TransportCatalogue &catalog) {
    // read queries number
    int records;
    in >> records;

    std::string line;
    std::getline(in, line); // eat endl after records

    for (int i = 0; i < records; ++i) {
        std::getline(in, line);
        if (auto pos = line.find("Bus "); pos == 0) {
            // process BusQuery
            ProcessBusQuery(line, out, catalog);
        } else if (auto pos = line.find("Stop "); pos == 0) {
            ProcessBusStopQuery(line, out, catalog);
        }
    }
}

void Stat::ProcessBusQuery(const std::string &line, std::ostream &out, TransportCatalogue &catalog) {
    // parse bus query line
    std::string_view name(line);
    name.remove_prefix(4); // "Bus ";
    // now we know the bus name
    name = detail::trimSpaces(name);
    // process bus query by transport catalog
    auto result = catalog.ProcessBusQuery(name);
    out << "Bus " << result.name << ":";
    // if request result exists
    if (result.valid) {
        // remember old stream  output precision
        const auto default_precision { out.precision() };

        out << " " << result.stops << " stops on route, " << result.unique_stops << " unique stops, "
                << std::setprecision(6) << result.length << " route length, " << result.curvature << " curvature";
        // restore default stream precision
        out << std::setprecision(default_precision);
    } else {
        out << " not found";
    }

    out << std::endl;
}

void Stat::ProcessBusStopQuery(const std::string &line, std::ostream &out, TransportCatalogue &catalog) {
    // parse bus stop query
    std::string_view name(line);
    name.remove_prefix(5); // "Stop ";
    name = detail::trimSpaces(name);
    // process query on transport catalog
    auto result = catalog.ProcessBusStopQuery(name);

    out << "Stop " << result.name << ":";
    // if result exists
    if (result.valid) {
        // if there are not buses through bus stop
        if (result.buses_names.size() == 0) {
            out << " no buses";
        } else { // we have buses
            out << " buses";
            for (auto bus : result.buses_names) {
                out << " " << bus;
            }
        }
    } else { // no busStop
        out << " not found";
    }
    out << std::endl;
}

} // namespace reader

} //namespace tc{
