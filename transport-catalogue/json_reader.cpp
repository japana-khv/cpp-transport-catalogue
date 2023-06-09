#include "json_reader.h"

using namespace std::literals;

namespace tc {

namespace reader {
json::Document Json::read_config(tc::TransportCatalogue &catalog, std::istream &input) const {
    json::Document jdoc = json::Load(input);

    LoadStops(jdoc, catalog);
    LoadBuses(jdoc, catalog);

    return jdoc;
}

json::Document Json::read_config(tc::TransportCatalogue &catalog, tc::renderer::Map &renderer,
        std::istream &input) const {
    json::Document jdoc = json::Load(input);

    LoadStops(jdoc, catalog);
    LoadBuses(jdoc, catalog);
    LoadRendererSettins(jdoc, renderer);

    return jdoc;
}

void Json::LoadBuses(const json::Document &doc, tc::TransportCatalogue &catalog) const {
    const auto &root = doc.GetRoot();

    if (auto search = root.AsDict().find("base_requests"s); search != root.AsDict().end()) {
        const auto &base_requests = search->second;
        // all Bus stops added to catalog
        // now we need to add all busses from string buffer
        for (const auto &element : base_requests.AsArray()) {
            if (element.AsDict().at("type"s) == "Bus"s) {
                catalog.AddBus(LoadBus(element, catalog));
            }
        }

    } else {
        throw JsonError("\"base_requests\" not found in json config"s);
    }

}

void Json::LoadStops(const json::Document &doc, tc::TransportCatalogue &catalog) const {
    const auto &root = doc.GetRoot();

    if (auto search = root.AsDict().find("base_requests"s); search != root.AsDict().end()) {
        const auto &base_requests = search->second;
        // distances info for all bus stops
        tc::DistanceInfoVector distances; // vector of pairs <name, distance_dinfo >

        // find bus stops and add to catalog
        for (const auto &element : base_requests.AsArray()) {
            if (element.AsDict().at("type"s) == "Stop"s) {
                catalog.AddBusStop(LoadBusStop(element, distances));
            }
        }
        // add distance information to catalog
        for (const auto& [name, distance_info] : distances) {
            catalog.SetSegmentDistance(name, distance_info.destination, distance_info.distance);
        }

    } else {
        throw JsonError("\"base_requests\" not found in json config"s);
    }

}

tc::BusStop Json::LoadBusStop(const json::Node &node, tc::DistanceInfoVector &distances) const {
    // extract bus_stop attributes
    auto latitude = node.AsDict().at("latitude"s).AsDouble();
    auto longitude = node.AsDict().at("longitude"s).AsDouble();
    auto name_start = node.AsDict().at("name"s).AsString();

    // if road_distance exists in this bus stop
    if (auto result = node.AsDict().find("road_distances"s); result != node.AsDict().end()) {
        const auto &road_distances = result->second;

        // extract distance info for current bus stop
        for (const auto& [name_dest, distance] : road_distances.AsDict()) {
            detail::DistanceInfo distance_info;
            distance_info.destination = name_dest;
            distance_info.distance = distance.AsInt();
            distances.emplace_back(name_start, distance_info);
        }
    }

    return {name_start, {latitude, longitude}};
}

tc::Bus Json::LoadBus(const json::Node &node, tc::TransportCatalogue &catalog) const {
    auto name = node.AsDict().at("name"s).AsString();
    tc::Bus bus(name);

    tc::BusType type = node.AsDict().at("is_roundtrip"s).AsBool() ? BusType::CIRCULAR : BusType::LINEAR;
    bus.SetType(type);
    // if stops exists
    if (auto search = node.AsDict().find("stops"s); search != node.AsDict().end()) {
        const auto &stops = search->second.AsArray();
        for (const auto &bus_stop : stops) {
            bus.AddBusStop(catalog.GetBusStop(bus_stop.AsString()));
        }
    }

    return bus;
}

void Json::LoadRendererSettins(const json::Document &doc, renderer::Map &renderer) const {
    auto config = doc.GetRoot().AsDict();
    if (auto search = config.find("render_settings"s); search != config.end()) {
        auto &config_map = search->second;
        renderer::Settings settings;
        settings.width = config_map.AsDict().at("width"s).AsDouble();
        settings.height = config_map.AsDict().at("height"s).AsDouble();
        settings.padding = config_map.AsDict().at("padding"s).AsDouble();
        settings.line_width = config_map.AsDict().at("line_width"s).AsDouble();
        settings.stop_radius = config_map.AsDict().at("stop_radius"s).AsDouble();
        settings.bus_label_font_size = config_map.AsDict().at("bus_label_font_size"s).AsInt();
        settings.bus_label_offset = LoadPoint(config_map.AsDict().at("bus_label_offset"s));
        settings.stop_label_font_size = config_map.AsDict().at("stop_label_font_size"s).AsInt();
        settings.stop_label_offset = LoadPoint(config_map.AsDict().at("stop_label_offset"s));
        settings.underlayer_color = LoadColor(config_map.AsDict().at("underlayer_color"s));
        settings.underlayer_width = config_map.AsDict().at("underlayer_width"s).AsDouble();
        settings.color_palette = LoadColorPalette(config_map.AsDict().at("color_palette"s));

        renderer.SetSettings(settings);

    }
}

svg::Point Json::LoadPoint(const json::Node &node) const {
    svg::Point result;
    result.x = node.AsArray()[0].AsDouble();
    result.y = node.AsArray()[1].AsDouble();
    return result;
}

svg::Color Json::LoadColor(const json::Node &node) const {
    if (node.IsString()) {
        return {node.AsString()};
    } else if (node.IsArray()) {
        auto &array = node.AsArray();
        if (array.size() == 3) {
            return {svg::Rgb {(uint8_t)array[0].AsInt(), (uint8_t)array[1].AsInt(), (uint8_t)array[2].AsInt()}};
        } else if (array.size() == 4) {
            return {svg::Rgba {(uint8_t)array[0].AsInt(), (uint8_t)array[1].AsInt(), (uint8_t)array[2].AsInt(), array[3].AsDouble()}};
        }
    }
    throw JsonError("LoadColor error");
}

std::vector<svg::Color> Json::LoadColorPalette(const json::Node &node) const {
    std::vector<svg::Color> result;
    for (const auto color_node : node.AsArray()) {
        result.emplace_back(LoadColor(color_node));
    }
    return result;
}

} //namespace reader
} // namespace tc
