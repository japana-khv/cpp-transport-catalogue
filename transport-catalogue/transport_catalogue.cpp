#include <algorithm>
#include <numeric>
#include <cassert>
#include "transport_catalogue.h"

namespace tc {

BusStop::BusStop(const std::string &name, const geo::Coordinates &coords) :
        name_(name), coord_(coords) {
}

BusStop::BusStop(const BusStop &other) :
        name_(other.name_), coord_(other.coord_) {
}

BusStop::BusStop(BusStop &&other) noexcept :
        name_(std::move(other.name_)), coord_(other.coord_) {
}

BusStop& BusStop::operator =(const BusStop &rhs) {
    if (&rhs != this) {
        name_ = rhs.name_;
        coord_ = rhs.coord_;
    }
    return *this;
}

BusStop& BusStop::operator =(BusStop &&rhs) noexcept {
    if (&rhs != this) {
        name_ = std::move(rhs.name_);
        coord_ = rhs.coord_;
    }
    return *this;
}

Bus::Bus(const std::string &name, BusType t) :
        name_(name), type_(t) {
}

Bus::Bus(const Bus &bus) :
        name_(bus.name_), type_(bus.type_), bus_stops_(bus.bus_stops_) {
}

Bus::Bus(Bus &&other) noexcept :
        name_(std::move(other.name_)), type_(other.type_), bus_stops_(std::move(other.bus_stops_)) {
}

Bus& Bus::operator =(const Bus &rhs) {
    if (&rhs != this) {
        name_ = rhs.name_;
        bus_stops_ = rhs.bus_stops_;
    }
    return *this;
}

Bus& Bus::operator =(Bus &&rhs) noexcept {
    if (&rhs != this) {
        name_ = std::move(rhs.name_);
        bus_stops_ = std::move(rhs.bus_stops_);
    }
    return *this;
}

void Bus::AddBusStop(const BusStop *stop) {
    this->bus_stops_.push_back(stop);
}

const BusStops& Bus::GetBusStops() const {
    return bus_stops_;
}

size_t Bus::GetBusStopsNumber() const {
    if (bus_stops_.size() == 0)
        return 0;
    if (GetType() == BusType::LINEAR) {
        return bus_stops_.size() * 2 - 1;
    } else if (GetType() == BusType::CIRCULAR) {
        return bus_stops_.size();
    }
    return 0;
}

size_t Bus::GetUniqueBusStops() const {
    BusStops temp = bus_stops_;
    std::sort(temp.begin(), temp.end());
    auto uniq_end = std::unique(temp.begin(), temp.end());
    auto distance = std::distance(temp.begin(), uniq_end);

    return distance;
}

double Bus::GetGeoLength() const {
    double result = 0.0;
    result = std::transform_reduce(++bus_stops_.cbegin(), bus_stops_.cend(), bus_stops_.cbegin(), 0.0, std::plus { },
            [](auto ptr1, auto ptr2) {
                return ComputeDistance(ptr1->getCoordinates(), ptr2->getCoordinates());
            });

    if (GetType() == BusType::LINEAR) {
        result *= 2.0;
    }
    return result;
}

double Bus::GetRouteLength(const TransportCatalogue &calc) const {
    double result = 0.0;
    if (this->GetType() == BusType::CIRCULAR) {
        result = std::transform_reduce(bus_stops_.cbegin() + 1, bus_stops_.cend(), bus_stops_.cbegin(), 0.0,
                std::plus { }, [&calc](auto ptr1, auto ptr2) {
                    return calc.GetSegmentDistance(ptr2->getName(), ptr1->getName());
                });
    } else { // LINEAR
        result = std::transform_reduce(bus_stops_.cbegin() + 1, bus_stops_.cend(), bus_stops_.cbegin(), 0.0,
                std::plus { }, [&calc](auto ptr1, auto ptr2) {
                    return calc.GetSegmentDistance(ptr2->getName(), ptr1->getName());
                });
        result += std::transform_reduce(bus_stops_.crbegin() + 1, bus_stops_.crend(), bus_stops_.crbegin(), 0.0,
                std::plus { }, [&calc](auto ptr1, auto ptr2) {
                    return calc.GetSegmentDistance(ptr2->getName(), ptr1->getName());
                });
    }
    return result;
}

detail::RouteLengthResult Bus::GetLength(const TransportCatalogue &calc) const {
    double geoLength = GetGeoLength();
    double length = GetRouteLength(calc);
    return {length, length / geoLength};;
}

detail::BusQueryResult TransportCatalogue::ProcessBusQuery(const std::string_view name) const {

    auto bus = GetBus(name);

    if (bus == nullptr) {
        return {false, name};
    } else {
        auto stops_on_route = bus->GetBusStopsNumber();
        auto unique_stops = bus->GetUniqueBusStops();
        auto lengthResult = bus->GetLength(*this);
        return {true, name, stops_on_route, unique_stops, lengthResult.lentgh , lengthResult.curvature};
    }
}

detail::BusStopQueryResult TransportCatalogue::ProcessBusStopQuery(const std::string_view name) const {

    auto bus_stop = this->GetBusStop(name);

    detail::BusStopQueryResult result { false, name, { } };

    if (bus_stop != nullptr) {
        result.valid = true;
        if (auto search = idx_bus_stops_to_buses.find(name); search != idx_bus_stops_to_buses.end()) {
            for (const auto &bus : search->second) {
                result.buses_names.push_back(bus->GetName());
            }
            // sort results by bus names
            std::sort(result.buses_names.begin(), result.buses_names.end(), [](auto lhs, auto rhs) {
                return lhs < rhs;
            });
            // erase duplicates
            auto fit = std::unique(result.buses_names.begin(), result.buses_names.end());
            result.buses_names.erase(fit, result.buses_names.end());
        }

    }

    return result;
}

void TransportCatalogue::AddBus(Bus &&bus) {
    buses_.push_back(bus);
    UpdateBusesIndexesByBackBus();
}

void TransportCatalogue::AddBus(const Bus &bus) {

    buses_.push_back(bus);
    UpdateBusesIndexesByBackBus();
}

void TransportCatalogue::UpdateBusesIndexesByBackBus() {

    auto bus_ptr = &buses_.back();
    buses_by_name_[buses_.back().GetName()] = bus_ptr;
    idx_bus_name_to_bus_[buses_.back().GetName()] = bus_ptr;

    for (const auto &bus_stop : buses_.back().GetBusStops()) {
        idx_bus_stops_to_buses[bus_stop->getName()].push_back(bus_ptr);
    }
}

void TransportCatalogue::AddBusStop(BusStop &&busStop) {

    bus_stops_.push_back(busStop);
    UpdateBusStopIndexesByBackBusStop();
}

void TransportCatalogue::AddBusStop(const BusStop &busStop) {

    bus_stops_.push_back(busStop);
    UpdateBusStopIndexesByBackBusStop();
}

double TransportCatalogue::GetSegmentDistance(const std::string_view stop1, const std::string_view stop2) const {

    double result = 0;
    if (auto search = segment_distances_.find( { stop1, stop2 }); search != segment_distances_.end()) {
        result = search->second;
    } else if (auto search = segment_distances_.find( { stop2, stop1 }); search != segment_distances_.end()) {
        result = search->second;
    } else {
        result = ComputeDistance(bus_stops_by_name_.at(stop1)->getCoordinates(),
                bus_stops_by_name_.at(stop2)->getCoordinates());
    }
    return result;
}

void TransportCatalogue::SetSegmentDistance(const std::string_view stop1_name, const std::string_view stop2_name,
        const size_t distance) {

    auto stop1 = GetBusStop(stop1_name);
    auto stop2 = GetBusStop(stop2_name);
    assert(stop1 != nullptr);
    assert(stop1 != nullptr);

    segment_distances_[ { stop1->getName(), stop2->getName() }] = distance;
}

std::vector<geo::Coordinates> TransportCatalogue::GetAllBusStopsCoordinates() const {

    std::vector<geo::Coordinates> points;

    points.reserve(bus_stops_.size());

    for (const auto &bus_stop : this->bus_stops_) {
        // include bus stop only if it have a buses
        if (auto search = idx_bus_stops_to_buses.find(bus_stop.getName()); search != idx_bus_stops_to_buses.end()) {
            if (search->second.size()) {
                points.emplace_back(bus_stop.getCoordinates());
            }
        }
    }

    return points;
}

void TransportCatalogue::UpdateBusStopIndexesByBackBusStop() {

    bus_stops_by_name_[bus_stops_.back().getName()] = &bus_stops_.back();
}

const Bus* TransportCatalogue::GetBus(const std::string_view name) const {
    if (auto search = buses_by_name_.find(name); search != buses_by_name_.end()) {
        return search->second;
    } else {
        return nullptr;
    }
}

const BusStop* TransportCatalogue::GetBusStop(const std::string_view name) const {
    if (auto search = bus_stops_by_name_.find(name); search != bus_stops_by_name_.end()) {
        return search->second;
    } else {
        return nullptr;
    }
}

void Bus::PrintBusStops(const TransportCatalogue &calc, std::ostream &os) const {
    auto first = this->bus_stops_.cbegin();
    auto second = first + 1;
    os << "<<< Bus " << this->name_ << ":\n";
    double result = 0;
    while (second != this->bus_stops_.cend()) {
        double segment = calc.GetSegmentDistance((*first)->getName(), (*second)->getName());
        os << "<<< Stop :" << (*first)->getName() << " to " << (*second)->getName() << " " << segment << "m \n";
        ++first;
        ++second;
        result += segment;
    }
    os << "<<< Length =  " << result << "m\n";
}

std::vector<geo::Coordinates> TransportCatalogue::GetBusStopsCoordinates(const std::string_view bus_name) const {
    std::vector<geo::Coordinates> result;
    if (auto search = buses_by_name_.find(bus_name); search != buses_by_name_.end()) {
        const tc::Bus &bus = *(search->second);
        result.reserve(bus.GetBusStopsNumber());
        // add all stops in forward direction
        for (const auto &stop : bus.GetBusStops()) {
            result.push_back(stop->getCoordinates());
        }

        if (bus.GetType() == tc::BusType::LINEAR) {

            // if linear - we need to add all stops from finish to start
            auto it = bus.GetBusStops().rbegin();
            for (++it; it != bus.GetBusStops().rend(); ++it) {
                result.push_back((*it)->getCoordinates());
            }
        }

    }

    return result;
}

std::vector<std::string_view> TransportCatalogue::GetSortedBusNames() const {

    std::vector<std::string_view> result;
    result.reserve(idx_bus_name_to_bus_.size());

    for (const auto [name, _] : this->idx_bus_name_to_bus_) {
        result.emplace_back(name);
    }

    return result;
}

std::vector<geo::Coordinates> TransportCatalogue::GetBusStopsForName(const std::string_view name) const {

    std::vector<geo::Coordinates> result;

    const auto &bus = *GetBus(name);
    result.reserve(bus.GetBusStopsNumber());

    if (bus.GetBusStopsNumber() > 0) {
        const auto &first_bus_stop = *(*bus.GetBusStops().begin());
        result.push_back(first_bus_stop.getCoordinates());
        if (bus.GetType() == tc::BusType::LINEAR) {
            const auto &last_bus_stop = *(*bus.GetBusStops().rbegin());
            if (first_bus_stop.getName() != last_bus_stop.getName()) {
                result.push_back(last_bus_stop.getCoordinates());
            }
        }
    }

    return result;
}

std::vector<std::pair<std::string_view, geo::Coordinates>> TransportCatalogue::GetAllBusStopsNamesAndCoordinatesSortedByName() const {

    std::vector<std::pair<std::string_view, geo::Coordinates>> result;
    result.reserve(bus_stops_.size());

    for (const auto &bus_stop : this->bus_stops_) {
        // include bus stop only if it have a buses
        if (auto search = idx_bus_stops_to_buses.find(bus_stop.getName()); search != idx_bus_stops_to_buses.end()) {
            if (search->second.size()) { // this bus stop is used by some buses
                result.push_back( { bus_stop.getName(), bus_stop.getCoordinates() });
            }
        }
    }
    std::sort(result.begin(), result.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.first < rhs.first;
    });

    return result;
}

} // namespace tc
