#pragma once

#include <exception>
#include <iostream>
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h"

namespace tc {

namespace handler {

class RequestHandler {
public:
    json::Document HandleQueries(const tc::TransportCatalogue &catalog, const json::Document &queries_document,
            tc::renderer::Map &renderer) const;

    void HandleBusQuery(const tc::TransportCatalogue &catalog, const json::Node &query, json::Builder &builder) const;

    void HandleBusStopQuery(const tc::TransportCatalogue &catalog, const json::Node &query,
            json::Builder &builder) const;

    void HandleMapQuery(const tc::TransportCatalogue &catalog, const json::Node &query, tc::renderer::Map &renderer,
            json::Builder &builder) const;

    void RenderBusRoutesMap(const tc::TransportCatalogue &catalog, tc::renderer::Map &renderer,
            std::ostream &out) const;
};

}

}
