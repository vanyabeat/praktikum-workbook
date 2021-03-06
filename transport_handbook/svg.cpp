#include "svg.h"
#include <sstream>

static std::string join_coords(const std::vector<svg::Point> &points) {
    std::stringstream ss;
    auto size = points.size();
    if (points.empty()) {
        return {};
    }
    size_t counter = 0;
    for (const auto &p : points) {
        if (counter != size - 1) {
            ss << p << " ";
        }
        ++counter;
    }
    ss << points[size - 1];

    return ss.str();
}

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext &context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle &Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle &Circle::SetRadius(double radius) {

        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << "<circle cx="sv << _q_(center_.x) << " cy="sv << _q_(center_.y) << " "sv;
        out << "r="sv << _q_(radius_);
        if (fill_color_.has_value()) {
            std::string c = "";
            std::visit(ColorString{c}, fill_color_.value());
            out << " fill=\"" << c << "\"";
            if (PathPropsIsEmpty()) {
                out << "/>"sv;
                return;
            }
        } else {
            out << " fill=\"none\"";
            if (PathPropsIsEmpty()) {
                out << "/>"sv;
                return;
            }
        }
        if (!PathPropsIsEmpty()) {
            RenderAttrs(out);
        }

        out << "/>"sv;
    }

    Polyline &Polyline::AddPoint(Point point) {
        points_.push_back(std::move(point));
        return *this;
    }

    void Polyline::RenderObject(const RenderContext &context) const {
        auto &out = context.out;

        out << R"(<polyline points=)"sv << _q_(join_coords(points_));
        if (fill_color_.has_value()) {
            std::string c = "";
            std::visit(ColorString{c}, fill_color_.value());
            out << " fill=\"" << c << "\"";
            if (PathPropsIsEmpty()) {
                out << R"(/>)"sv;
                return;
            }
        } else {
            out << " fill=\"none\"";
            if (PathPropsIsEmpty()) {
                out << R"(/>)"sv;
                return;
            }
        }
        if (!PathPropsIsEmpty()) {
            RenderAttrs(out);
        }
        out << R"(/>)"sv;
    }

    void Text::RenderObject(const RenderContext &context) const {
        auto &out = context.out;

        out << R"(<text)";
        if (fill_color_.has_value()) {
            std::string c = "";
            std::visit(ColorString{c}, fill_color_.value());
            out << " fill=\"" << c << "\"";
            if (PathPropsIsEmpty()) {
                out << " ";
            }
        } else {
            out << " fill=\"none\"";
            if (PathPropsIsEmpty()) {
                out << " ";
            }
        }
        if (!PathPropsIsEmpty()) {
            RenderAttrs(out);
            out << " ";
        }

        out << R"(x=)"sv << _q_(position_.x) << R"( y=)"sv << _q_(position_.y) << R"( dx=)"sv << _q_(offset_.x)
            << R"( dy=)"sv << _q_(offset_.y);

        out << R"( font-size=)"sv << _q_(font_size_);
        if (!font_family_.empty()) {
            out << R"( font-family=)"sv << _q_(font_family_);
        }
        if (!font_weight_.empty()) {
            out << R"( font-weight=)"sv << _q_(font_weight_);
        }
        out << R"(>)"sv;
        out << data_ << R"(</text>)";
    }

    Text &Text::SetPosition(Point pos) {
        position_ = std::move(pos);
        return *this;
    }

    Text &Text::SetOffset(Point offset) {
        offset_ = std::move(offset);
        return *this;
    }

    Text &Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text &Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text &Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text &Text::SetData(std::string data) {
        data_ = std::move(data);
        for (const auto c : data_) {
            for (auto&[key, value] : defs_) {
                if (key[0] == c) {
                    replace_all_(data_, key, value);
                }
                continue;
            }
        }
        return *this;
    }

    std::ostream &operator<<(std::ostream &out, StrokeLineCap cap) {
        using namespace std;
        switch (cap) {
            case StrokeLineCap::ROUND: {
                out << "round"sv;
                break;
            }
            case StrokeLineCap::SQUARE: {
                out << "square"sv;
                break;
            }
            case StrokeLineCap::BUTT: {
                out << "butt"sv;
                break;
            }
        }
        return out;
    }

    std::ostream &operator<<(std::ostream &out, StrokeLineJoin cap) {
        using namespace std;

        switch (cap) {
            case StrokeLineJoin::ARCS: {
                out << "arcs"sv;
                break;
            }
            case StrokeLineJoin::MITER: {
                out << "miter"sv;
                break;
            }
            case StrokeLineJoin::ROUND: {
                out << "round"sv;
                break;
            }
            case StrokeLineJoin::BEVEL: {
                out << "bevel"sv;
                break;
            }
            case StrokeLineJoin::MITER_CLIP: {
                out << "miter-clip"sv;
                break;
            }
        }
        return out;
    }

} // namespace svg

void shapes::Snowman::Draw(svg::ObjectContainer &container) const {
    container.Add(svg::Circle(center_.x, (center_.y + (head_radius_ * 5)), head_radius_ * 2)
                          .SetFillColor(fill_color_)
                          .SetStrokeColor(stroke_color_));
    container.Add(svg::Circle(center_.x, (center_.y + (head_radius_ * 2)), head_radius_ * 1.5)
                          .SetFillColor(fill_color_)
                          .SetStrokeColor(stroke_color_));
    container.Add(
            svg::Circle(center_.x, center_.y, head_radius_).SetFillColor(fill_color_).SetStrokeColor(stroke_color_));
}

void shapes::Star::Draw(svg::ObjectContainer &container) const {
    svg::Polyline polyline;
    for (int i = 0; i <= num_rays_; ++i) {
        double angle = 2 * M_PI * (i % num_rays_) / num_rays_;
        polyline.AddPoint({center_.x + outer_radius_ * sin(angle), center_.y - outer_radius_ * cos(angle)});
        if (i == num_rays_) {
            break;
        }
        angle += M_PI / num_rays_;
        polyline.AddPoint({center_.x + inner_radius_ * sin(angle), center_.y - inner_radius_ * cos(angle)});
    }
    container.Add(polyline.SetFillColor(fill_color_).SetStrokeColor(stroke_color_));
}
