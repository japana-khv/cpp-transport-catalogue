#pragma once

#include <string>
#include <string_view>
#include <iostream>
#include <vector>
#include <deque>
#include <unordered_map>
#include <string_view>
#include <map>

#include "geo.h"

namespace tc {

namespace detail {

struct BusQueryResult {
    bool valid;
    std::string_view name;
    size_t stops = 0;
    size_t unique_stops = 0;
    double length = 0;
    double curvature = 0;
};

struct BusStopQueryResult {
    bool valid;
    std::string_view name;
    std::vector<std::string_view> buses_names;
};

struct RouteLengthResult {
    double lentgh;
    double curvature;
};

} //  namespace detail

class BusStop {
    std::string name_;
    geo::Coordinates coord_;
public:
    BusStop(const std::string &name, const geo::Coordinates &coords);
    BusStop(const BusStop &other);
    BusStop(BusStop &&other) noexcept;
    BusStop& operator=(const BusStop &rhs);
    BusStop& operator=(BusStop &&rhs) noexcept;

    const geo::Coordinates& getCoordinates() const {
        return coord_;
    }

    const std::string& getName() const {
        return name_;
    }
};

using BusStops = std::vector<const BusStop*>;

enum class BusType {
    LINEAR, CIRCULAR
};

class TransportCatalogue;

class Bus {
    std::string name_;
    BusType type_;
    BusStops bus_stops_;
public:
    Bus(const std::string &name, BusType type = BusType::LINEAR);
    Bus(const Bus &bus);
    Bus(Bus &&other) noexcept;
    Bus& operator=(const Bus &rhs);
    Bus& operator=(Bus &&rhs) noexcept;

    void AddBusStop(const BusStop *stop);
    // bus stops on for  bus
    const BusStops& GetBusStops() const;
    // get bus stops number for this bus
    size_t GetBusStopsNumber() const;
    // get unique bus stops number
    size_t GetUniqueBusStops() const;
    // get route length by GeoCoordinates of bus stops
    double GetGeoLength() const;
    // get real length based on distances information of segments
    double GetRouteLength(const TransportCatalogue &calc) const;
    // get final route length information
    detail::RouteLengthResult GetLength(const TransportCatalogue &calc) const;

    // debug output method
    void PrintBusStops(const TransportCatalogue &calc, std::ostream &os) const;

    const std::string& GetName() const {
        return name_;
    }

    BusType GetType() const {
        return type_;
    }

    void SetType(BusType type) {
        type_ = type;
    }
};

struct pair_hash {
    template<class T1, class T2>
    std::size_t operator ()(const std::pair<T1, T2> &p) const {
        auto h1 = std::hash<T1> { }(p.first);
        auto h2 = std::hash<T2> { }(p.second);
        return h1 * 37 + h2;
    }
};

using MapSegmentDistances = std::unordered_map<std::pair<std::string_view, std::string_view>, size_t, pair_hash>;

class TransportCatalogue {
    std::deque<BusStop> bus_stops_;
    std::deque<Bus> buses_;

    // buses by name index
    std::unordered_map<std::string_view, const Bus*> buses_by_name_;

    // bus tops by name index
    std::unordered_map<std::string_view, const BusStop*> bus_stops_by_name_;

    // index on bus stop name to bus for bus stop usage checking
    std::unordered_map<std::string_view, std::vector<Bus*>> idx_bus_stops_to_buses;

// index for sorted by name bus list
    std::map<std::string_view, const Bus*> idx_bus_name_to_bus_;

    MapSegmentDistances segment_distances_;
public:

    void AddBus(Bus &&bus);

    void AddBus(const Bus &bus);

    void AddBusStop(BusStop &&busStop);

    void AddBusStop(const BusStop &busStop);

    const Bus* GetBus(const std::string_view name) const;

    const BusStop* GetBusStop(const std::string_view name) const;

    // returns result on bus request
    detail::BusQueryResult ProcessBusQuery(const std::string_view name) const;

    // returns information on bus stop requies
    detail::BusStopQueryResult ProcessBusStopQuery(const std::string_view name) const;

    // get distance between to bus stops (the length of  bus line segment )
    double GetSegmentDistance(const std::string_view stop1, const std::string_view stop2) const;

    // fill distance information of bus line segments
    void SetSegmentDistance(const std::string_view stop1, const std::string_view stop2, const size_t distance);

    // returns bus stops only used by bus lines
    std::vector<geo::Coordinates> GetAllBusStopsCoordinates() const;

    // returns bus stops coordinates for bus.
    // if BusType::LINEAR - returned bus stops points for both directions
    std::vector<geo::Coordinates> GetBusStopsCoordinates(const std::string_view bus_name) const;

    // returns bus names vector sorted by name
    std::vector<std::string_view> GetSortedBusNames() const;

    // returns vector of pairs  bus stops name and position , sorted by name
    std::vector<std::pair<std::string_view, geo::Coordinates>> GetAllBusStopsNamesAndCoordinatesSortedByName() const;

    // return bus stops positions for bus rendering
    std::vector<geo::Coordinates> GetBusStopsForName(const std::string_view name) const;

private:
// update indexes after pushBACK new bus in dequeue
    void UpdateBusesIndexesByBackBus();
    // update indexes after pushBACK new bus stop in dequeue
    void UpdateBusStopIndexesByBackBusStop();
};

} //namespace tc
