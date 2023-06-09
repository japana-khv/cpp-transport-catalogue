/*
 * map_renderer.cpp
 *
 *  Created on: May 16, 2023
 *      Author: rvk
 */

#include "map_renderer.h"
#include <string>
using namespace std::literals;

namespace tc {

namespace renderer {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

void Map::InitProjector(const std::vector<geo::Coordinates> &points) {
    projector_ = SphereProjector(points.begin(), points.end(), settings_.width, settings_.height, settings_.padding);
}

void Map::RenderLine(const std::vector<geo::Coordinates> &points, svg::Document &output) {
    svg::Polyline poly_line;
    poly_line.SetStrokeWidth(settings_.line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(
            svg::StrokeLineJoin::ROUND).SetStrokeColor(GetGolorFromPalette());

    poly_line.SetFillColor(svg::NoneColor);

    for (const auto &point : points) {
        poly_line.AddPoint(projector_(point));
    }
    output.AddPtr(std::make_unique<svg::Polyline>(poly_line));
}

void Map::SetNextColor() {
    current_color = (current_color + 1) % settings_.color_palette.size();
}

void Map::RenderBusStopPoint(const geo::Coordinates &point, svg::Document &output) {
    svg::Circle circle;
    circle.SetCenter(projector_(point));
    circle.SetRadius(settings_.stop_radius);
    circle.SetFillColor("white"s);

    output.AddPtr(std::make_unique<svg::Circle>(circle));
}

void Map::RenderBusStopName(const geo::Coordinates &point, const std::string_view &bus_stop_name,
        svg::Document &output) {
    svg::Text text;
    text.SetData(std::string { bus_stop_name });
    text.SetPosition(projector_(point));
    text.SetOffset(settings_.stop_label_offset);
    text.SetFontSize(settings_.stop_label_font_size);
    text.SetFontFamily("Verdana"s);

    svg::Text bottom = text;

    text.SetFillColor("black"s);

    bottom.SetFillColor(settings_.underlayer_color);
    bottom.SetStrokeColor(settings_.underlayer_color);
    bottom.SetStrokeWidth(settings_.underlayer_width);
    bottom.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    bottom.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    output.AddPtr(std::make_unique<svg::Text>(bottom));
    output.AddPtr(std::make_unique<svg::Text>(text));

}

svg::Color Map::GetGolorFromPalette() {
    return settings_.color_palette[current_color];

}

void Map::RenderBusName(const geo::Coordinates &point, const std::string_view &bus_name, svg::Document &output) {

    svg::Text text;
    text.SetData(std::string { bus_name });
    text.SetPosition(projector_(point));
    text.SetOffset(settings_.bus_label_offset);
    text.SetFontSize(settings_.bus_label_font_size);
    text.SetFontFamily("Verdana"s);
    text.SetFontWeight("bold"s);

    svg::Text bottom = text;

    text.SetFillColor(GetGolorFromPalette());

    bottom.SetFillColor(settings_.underlayer_color);
    bottom.SetStrokeColor(settings_.underlayer_color);
    bottom.SetStrokeWidth(settings_.underlayer_width);
    bottom.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    bottom.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    output.AddPtr(std::make_unique<svg::Text>(bottom));
    output.AddPtr(std::make_unique<svg::Text>(text));
}

void Map::InitPaletteColor() {
    current_color = 0;
}

}
}
