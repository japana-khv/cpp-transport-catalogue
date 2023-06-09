#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <string_view>
#include <array>
#include <optional>
#include <variant>

namespace svg {

namespace scr {
using namespace std::literals;

const std::string_view spec_chars = R"("'<>&)"sv;
const std::string_view quot = "&quot"sv;
const std::string_view apos = "&apos"sv;
const std::string_view lt = "<&lt"sv;
const std::string_view gt = "&gt"sv;
const std::string_view amp = "&amp"sv;

}

struct Point {
    Point() = default;
    Point(double x, double y) :
            x(x), y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream &out) :
            out(out) {
    }

    RenderContext(std::ostream &out, int indent_step, int indent = 0) :
            out(out), indent_step(indent_step), indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream &out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext &context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext &context) const = 0;
};

class ObjectContainer {
public:
    /*    Метод Add добавляет вконтейнер  любой объект-наследник svg::Object.
     Пример использования:
     Document doc;
     doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
     */
    template<typename Element>
    void Add(Element e) {
        objects_.emplace_back(std::move(std::make_unique<Element>(std::move(e))));
    }
    virtual void AddPtr(std::unique_ptr<Object> &&obj) = 0;
    virtual ~ObjectContainer() {
    }
protected:
    std::vector<std::unique_ptr<Object>> objects_;
};

//-------------------Rgb-------------------------
struct Rgb {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    Rgb(unsigned int r, unsigned int g, unsigned int b) :
            red(r), green(g), blue(b) {
    }
    Rgb() = default;
};
//-------------------Rgba-------------------------
struct Rgba {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
    Rgba(unsigned int r, unsigned int g, unsigned int b, double o) :
            red(r), green(g), blue(b), opacity(o) {
    }
    Rgba() = default;
};

// ------------------Color-----------------------

using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;

std::ostream& operator<<(std::ostream &os, const Color &c);

inline const Color NoneColor;
//------------------------------------------------

enum class StrokeLineCap {
    BUTT, ROUND, SQUARE,
};

std::ostream& operator<<(std::ostream &os, StrokeLineCap cap);

enum class StrokeLineJoin {
    ARCS, BEVEL, MITER, MITER_CLIP, ROUND,
};

std::ostream& operator<<(std::ostream &os, StrokeLineJoin join);

template<typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        stroke_linecap_ = line_cap;
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        stroke_linejoin_ = line_join;
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream &out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }
        if (stroke_width_) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }
        if (stroke_linecap_) {
            out << " stroke-linecap=\""sv << *stroke_linecap_ << "\""sv;
        }
        if (stroke_linejoin_) {
            out << " stroke-linejoin=\""sv << *stroke_linejoin_ << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_linecap_;
    std::optional<StrokeLineJoin> stroke_linejoin_;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext &context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline: public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext &context) const override;

private:
    std::vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text: public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    // Прочие данные и методы, необходимые для реализации элемента <text>
private:
    Point position_;
    Point offset_;
    uint32_t font_size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;
private:
    void RenderObject(const RenderContext &context) const override;
    const std::string_view GetScreenSeq(char ch) const;
};

class Document: public ObjectContainer {
public:

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object> &&obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream &out) const;

};
/*
 * Интерфейс Drawable имеет метод Draw позволяющий нарисовать себя в ObjectContainer-е
 * */
class Drawable {
public:
    virtual void Draw(ObjectContainer&) const = 0;
    virtual ~Drawable() {
    }
};

namespace shapes {

class Triangle: public Drawable {
public:
    Triangle(const Point &p1, const Point &p2, const Point &p3) :
            points_( { p1, p2, p3 }) {
    }
    ;
    void Draw(ObjectContainer &obj_container) const override;
private:
    std::array<Point, 3> points_;
};

class Snowman: public Drawable {
public:
    Snowman(const Point &head_center, const double radius);
    void Draw(ObjectContainer &obj_container) const override;
private:
    Point head_center_;
    double radius_;
};

class Star: public Drawable {
public:
    Star(const Point &center, const double outer_radius, const double inner_radius, const int num_rays);
    void Draw(ObjectContainer &obj_container) const override;
private:
    Point center_;
    double outer_radius_;
    double inner_radius_;
    int num_rays_;
};

} //namespace shapes{

Polyline CreateStar(Point center, double outer_rad, double inner_rad, int num_rays);

}  // namespace svg
