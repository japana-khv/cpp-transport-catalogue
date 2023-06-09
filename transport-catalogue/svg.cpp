#include "svg.h"
#include <exception>

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext &context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

std::ostream& operator<<(std::ostream &os, StrokeLineCap cap) {
    using namespace std::literals;
    switch (cap) {
    case StrokeLineCap::BUTT:
        return os << "butt"sv;
    case StrokeLineCap::ROUND:
        return os << "round"sv;
    case StrokeLineCap::SQUARE:
        return os << "square"sv;
    }
    return os;
}

std::ostream& operator<<(std::ostream &os, StrokeLineJoin join) {
    using namespace std::literals;
    switch (join) {
    case StrokeLineJoin::ARCS:
        return os << "arcs"sv;
    case StrokeLineJoin::BEVEL:
        return os << "bevel"sv;
    case StrokeLineJoin::MITER:
        return os << "miter"sv;
    case StrokeLineJoin::MITER_CLIP:
        return os << "miter-clip"sv;
    case StrokeLineJoin::ROUND:
        return os << "round"sv;
    }
    return os;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext &context) const {
    auto &out = context.out;
    out << "<circle";
    out << " cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point) {
    points_.emplace_back(std::move(point));
    return *this;
}

void Polyline::RenderObject(const RenderContext &context) const {
    auto &out = context.out;
    out << "<polyline";
    out << " points=\""sv;
    {
        bool first = true;
        for (const auto &point : points_) {
            if (first) {
                first = false;
            } else {
                out << " ";
            }
            out << point.x << "," << point.y;
        }
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

Text& Text::SetPosition(Point pos) {
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext &context) const {
    auto &out = context.out;
    out << "<text";
    RenderAttrs(out);
    out << " x=\""sv << position_.x << "\" y=\"" << position_.y << "\"";
    out << " dx=\""sv << offset_.x << "\" dy=\"" << offset_.y << "\"";
    out << " font-size=\""sv << font_size_ << "\"";
    if (font_family_.size()) {
        out << " font-family=\""sv << font_family_ << "\"";
    }
    if (font_weight_.size()) {
        out << " font-weight=\""sv << font_weight_ << "\"";
    }
    out << ">"sv;
    // print data here
    for (const auto ch : data_) {
        // screening special chars if required
        if (auto search = scr::spec_chars.find(ch); search != std::string_view::npos) {
            out << GetScreenSeq(ch);
        } else {
            out << ch;
        }
    }
    out << "</text>"sv;
}

const std::string_view Text::GetScreenSeq(char ch) const {
    switch (ch) {
    case '"':
        return scr::quot;
    case '\'':
        return scr::apos;
    case '<':
        return scr::lt;
    case '>':
        return scr::gt;
    case '&':
        return scr::amp;
    default:
        throw std::invalid_argument("Unknown screen char");
    }
    return {};
}

void Document::AddPtr(std::unique_ptr<Object> &&obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream &out) const {
    RenderContext ctx(out, 2, 2);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (const auto &ptr : objects_) {
        ptr->Render(ctx);
    }
    out << "</svg>"sv;

}

Polyline CreateStar(Point center, double outer_rad, double inner_rad, int num_rays) {
    Polyline polyline;
    for (int i = 0; i <= num_rays; ++i) {
        double angle = 2 * M_PI * (i % num_rays) / num_rays;
        polyline.AddPoint( { center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle) });
        if (i == num_rays) {
            break;
        }
        angle += M_PI / num_rays;
        polyline.AddPoint( { center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle) });
    }
    return polyline;
}

namespace shapes {

void Triangle::Draw(ObjectContainer &obj_container) const {
    auto p = std::make_unique<Polyline>();
    for (const auto &point : points_) {
        p->AddPoint(point);
    }
    p->AddPoint(points_[0]);
    obj_container.AddPtr(std::move(p));
}

Snowman::Snowman(const Point &head_center, const double radius) :
        head_center_(head_center), radius_(radius) {
}

void Snowman::Draw(ObjectContainer &obj_container) const {
    const double k_y[] = { 5, 2, 0 };
    const double k_r[] = { 2, 1.5, 1 };
    for (int i = 0; i < 3; ++i) {
        Point current_center = head_center_;
        current_center.y += radius_ * k_y[i];
        double current_radius = radius_ * k_r[i];

        obj_container.Add(
                Circle().SetCenter(current_center).SetRadius(current_radius).SetFillColor("rgb(240,240,240)"s).SetStrokeColor(
                        "black"s));
    }
}

Star::Star(const Point &center, const double outer_radius, const double inner_radius, const int num_rays) :
        center_(center), outer_radius_(outer_radius), inner_radius_(inner_radius), num_rays_(num_rays) {
}

void Star::Draw(ObjectContainer &obj_container) const {
    obj_container.AddPtr(
            std::make_unique<Polyline>(
                    CreateStar(center_, outer_radius_, inner_radius_, num_rays_).SetFillColor("red"s).SetStrokeColor(
                            "black"s)));
}

} //namespace shapes

std::ostream& operator <<(std::ostream &os, const svg::Color &c) {
    // std::get_if вернёт указатель на значение нужного типа
    // либо nullptr, если variant содержит значение другого типа.
    if (std::holds_alternative<std::monostate>(c)) {
        os << "none";
    } else if (const auto *color_ptr = std::get_if<std::string>(&c)) {
        os << *color_ptr;
    } else if (const auto *rgb_ptr = std::get_if<svg::Rgb>(&c)) {
        os << "rgb(" << (int) rgb_ptr->red << ",";
        os << (int) rgb_ptr->green << ",";
        os << (int) rgb_ptr->blue << ")";
    } else if (const auto *rgba_ptr = std::get_if<svg::Rgba>(&c)) {
        os << "rgba(" << (int) rgba_ptr->red << ",";
        os << (int) rgba_ptr->green << ",";
        os << (int) rgba_ptr->blue << ",";
        os << rgba_ptr->opacity << ")";
    }
    return os;
}

}  // namespace svg

