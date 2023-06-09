#pragma once

#include <exception>
#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "domain.h"

namespace tc {

namespace reader {

class JsonError: public std::invalid_argument {
public:
    JsonError(const std::string &what) :
            std::invalid_argument(what) {
    }
};

// reader::Json - reads configurations and requests from JSON file

class Json {
public:
    // reads configuration from JSON formatted stream
    // returns json::Document
    json::Document read_config(tc::TransportCatalogue &catalog, std::istream &input) const;

    // reads configuration from JSON formatted stream , loads configuration in renderer
    // returns json::Document
    json::Document read_config(tc::TransportCatalogue &catalog, tc::renderer::Map &renderer, std::istream &input) const;

private:
    // load buses into catalog
    void LoadBuses(const json::Document &doc, tc::TransportCatalogue &catalog) const;
    // load bus stops into catalog
    void LoadStops(const json::Document &doc, tc::TransportCatalogue &catalog) const;
    // load one bus stop into catalog
    tc::BusStop LoadBusStop(const json::Node &node, tc::DistanceInfoVector &distances) const;
    // load one bus into catalog
    tc::Bus LoadBus(const json::Node &node, tc::TransportCatalogue &catalog) const;
    // load renderer settings
    void LoadRendererSettins(const json::Document &doc, renderer::Map &renderer) const;
    // load Point
    svg::Point LoadPoint(const json::Node &node) const;
    //
    svg::Color LoadColor(const json::Node &node) const;

    std::vector<svg::Color> LoadColorPalette(const json::Node &node) const;
};

} //namespace reader
} //namespace tc
