//============================================================================
// Name        : transport_catalog.cpp
// Author      : rvk
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>

#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

using namespace std;

int main() {

    tc::TransportCatalogue catalog;
    tc::reader::Json config_reader;
    tc::renderer::Map map_renderer;
    // read configuration for catalog and renderer and returns full json configuration document
    json::Document jdoc = config_reader.read_config(catalog, map_renderer, cin);

    tc::handler::RequestHandler handler;
    // handle requests from configuration document
    json::Document results = handler.HandleQueries(catalog, jdoc, map_renderer);

    json::Print(results, cout);

    return 0;
}
