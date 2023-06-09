/*
 * request_handler.cpp
 *
 *  Created on: May 16, 2023
 *      Author: rvk
 */

#include "request_handler.h"
#include <sstream>

using namespace std::literals;

namespace tc {

namespace handler {
json::Document RequestHandler::HandleQueries(const tc::TransportCatalogue &catalog,
        const json::Document &queries_document, tc::renderer::Map &renderer) const {

    auto queries = queries_document.GetRoot().AsDict().at("stat_requests"s).AsArray();

    json::Builder builder;
    builder.StartArray();

    json::Array results;

    for (const auto &query : queries) {
        if (query.AsDict().at("type"s) == "Bus"s) {
            HandleBusQuery(catalog, query, builder);
        } else if (query.AsDict().at("type"s) == "Stop"s) {
            HandleBusStopQuery(catalog, query, builder);
        } else if (query.AsDict().at("type"s) == "Map"s) {
            HandleMapQuery(catalog, query, renderer, builder);
        }

    }
    builder.EndArray();

    return json::Document(builder.Build());
}

void RequestHandler::RenderBusRoutesMap(const tc::TransportCatalogue &catalog, tc::renderer::Map &renderer,
        std::ostream &out) const {
    svg::Document bus_map;

    // init renderer by all stops points
    auto points = catalog.GetAllBusStopsCoordinates();
    // initialize renderer
    renderer.InitProjector(points);
    renderer.InitPaletteColor();

    // render bus lines
    auto buses = catalog.GetSortedBusNames();
    // render buses in sorted by name order
    for (const auto bus_name : buses) {
        auto points = catalog.GetBusStopsCoordinates(bus_name);
        if (points.size()) {
            renderer.RenderLine(points, bus_map);
        }
        renderer.SetNextColor();
    }
    // initialize renderer palette color to start
    renderer.InitPaletteColor();
    // render bus names in sorted by name order
    for (const auto bus_name : buses) {
        auto stops_points = catalog.GetBusStopsForName(bus_name);
        for (const auto &point : stops_points) {
            renderer.RenderBusName(point, bus_name, bus_map);
        }
        renderer.SetNextColor(); // switch palette to next color
    }
    // render bus_stops circles
    auto bus_stops = catalog.GetAllBusStopsNamesAndCoordinatesSortedByName();
    for (const auto& [_, point] : bus_stops) {
        renderer.RenderBusStopPoint(point, bus_map);
    }

    // render bus stops names;
    for (const auto& [name, point] : bus_stops) {
        renderer.RenderBusStopName(point, name, bus_map);
    }

    // output result document
    bus_map.Render(out);
}

void RequestHandler::HandleMapQuery(const tc::TransportCatalogue &catalog, const json::Node &query,
        tc::renderer::Map &renderer, json::Builder &builder) const {

    int id = query.AsDict().at("id"s).AsInt();

    builder.StartDict().Key("request_id"s).Value(id);

    std::ostringstream out;

    RenderBusRoutesMap(catalog, renderer, out);
    builder.Key("map"s).Value(std::move(out).str());

    builder.EndDict();
}

void RequestHandler::HandleBusQuery(const tc::TransportCatalogue &catalog, const json::Node &query,
        json::Builder &builder) const {

    int id = query.AsDict().at("id"s).AsInt();

    builder.StartDict().Key("request_id"s).Value(id);

    auto query_result = catalog.ProcessBusQuery(query.AsDict().at("name"s).AsString());

    if (query_result.valid) { // bus was found
        builder.Key("curvature"s).Value(query_result.curvature);
        builder.Key("route_length"s).Value(static_cast<int>(query_result.length));
        builder.Key("stop_count"s).Value(static_cast<int>(query_result.stops));
        builder.Key("unique_stop_count"s).Value(static_cast<int>(query_result.unique_stops));
    } else { // bus not found
        builder.Key("error_message"s).Value("not found"s);
    }

    builder.EndDict();
}

void RequestHandler::HandleBusStopQuery(const tc::TransportCatalogue &catalog, const json::Node &query,
        json::Builder &builder) const {

    int id = query.AsDict().at("id"s).AsInt();
    builder.StartDict().Key("request_id"s).Value(id);

    auto query_result = catalog.ProcessBusStopQuery(query.AsDict().at("name"s).AsString());

    if (query_result.valid) { // bus stop found

        builder.Key("buses"s).StartArray();

        for (const auto &bus_name : query_result.buses_names) {
            builder.Value(std::string(bus_name));
        }

        builder.EndArray();

    } else { // bus stop not found
        builder.Key("error_message"s).Value("not found"s);
    }

    builder.EndDict();
}

} // namespace handler

} // namespace tc
