#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <math.h>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace svg
{
	using Color = std::string;

	inline const Color NoneColor{"none"};

	class Point
	{
	  public:
		Point() : x(0), y(0){};
		Point(double x, double y) : x(x), y(y)
		{
		}
		double x;
		double y;

		friend std::ostream& operator<<(std::ostream& out, const Point& p)
		{
			//			out << std::setprecision(1) << p.x << "," << p.y;
			out << p.x << "," << p.y;
			return out;
		}
	};

	/*
	 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
	 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
	 */
	struct RenderContext
	{
		RenderContext(std::ostream& out) : out(out)
		{
		}

		RenderContext(std::ostream& out, int indent_step, int indent = 0)
			: out(out), indent_step(indent_step), indent(indent)
		{
		}

		RenderContext Indented() const
		{
			return {out, indent_step, indent + indent_step};
		}

		void RenderIndent() const
		{
			for (int i = 0; i < indent; ++i)
			{
				out.put(' ');
			}
		}

		std::ostream& out;
		int indent_step = 0;
		int indent = 0;
	};
	enum class StrokeLineCap
	{
		BUTT,
		ROUND,
		SQUARE
	};

	std::ostream& operator<<(std::ostream& out, StrokeLineCap cap);

	enum class StrokeLineJoin
	{
		ARCS,
		BEVEL,
		MITER,
		MITER_CLIP,
		ROUND,
	};

	std::ostream& operator<<(std::ostream& out, StrokeLineJoin cap);

	/*
	 * Абстрактный базовый класс Object служит для унифицированного хранения
	 * конкретных тегов SVG-документа
	 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
	 */
	class Object
	{
	  public:
		void Render(const RenderContext& context) const;

		virtual ~Object() = default;

	  private:
		virtual void RenderObject(const RenderContext& context) const = 0;
	};

	template <typename C> class PathProps
	{
	  public:
		PathProps() = default;
		virtual ~PathProps() = default;

		C& SetFillColor(const std::optional<Color>& fillColor)
		{
			fill_color_ = fillColor;
			return static_cast<C&>(*this);
		}
		C& SetStrokeColor(const std::optional<Color>& strokeColor)
		{
			stroke_color_ = strokeColor;
			return static_cast<C&>(*this);
		}
		C& SetStrokeWidth(const std::optional<double>& strokeWidth)
		{
			stroke_width_ = strokeWidth;
			return static_cast<C&>(*this);
		}
		C& SetStrokeLineCap(const std::optional<StrokeLineCap>& strokeLineCap)
		{
			stroke_line_cap_ = strokeLineCap;
			return static_cast<C&>(*this);
		}
		C& SetStrokeLineJoin(const std::optional<StrokeLineJoin>& strokeLineJoin)
		{
			stroke_line_join = strokeLineJoin;
			return static_cast<C&>(*this);
		}
		void RenderAttrs(std::ostream& out) const
		{
			out << " ";
			if (fill_color_.has_value())
			{
				out << "fill=\"" << fill_color_.value() << "\" ";
			}
			if (stroke_color_.has_value())
			{
				out << "stroke=\"" << stroke_color_.value() << "\" ";
			}
			if (stroke_width_.has_value())
			{
				out << "stroke-width=\"" << stroke_width_.value() << "\" ";
			}
			if (stroke_line_cap_.has_value())
			{
				out << "stroke-linecap=";
				out << "\"" << stroke_line_cap_.value() << "\"";
				out << " ";
			}
			if (stroke_line_join.has_value())
			{
				//  | miter-clip |  |  |
				out << "stroke-linejoin=";
				out << "\"" << stroke_line_join.value() << "\"";
				out << " ";
			}
		}

		bool PathPropsIsEmpty() const
		{
			return !(fill_color_.has_value() || stroke_color_.has_value() || stroke_width_.has_value() ||
					 stroke_line_join.has_value() || stroke_line_cap_.has_value());
		}

	  protected:
		std::optional<Color> fill_color_;
		std::optional<Color> stroke_color_;
		std::optional<double> stroke_width_;
		std::optional<StrokeLineCap> stroke_line_cap_;
		std::optional<StrokeLineJoin> stroke_line_join;
	};
	class ObjectContainer
	{
	  public:
		template <typename T> void Add(T obj)
		{
			AddPtr(std::make_unique<T>(std::move(obj)));
		}

		virtual void AddPtr(std::unique_ptr<Object>&& object) = 0;

		virtual ~ObjectContainer() = default;
	};

	class Drawable
	{
	  public:
		virtual void Draw(ObjectContainer& container) const = 0;
		virtual ~Drawable() = default;
	};

	class Quotable
	{
	  public:
		const std::string_view _Q_{R"(")"};

		template <typename T> std::string _q_(T o) const
		{
			std::stringstream ss;
			ss << _Q_ << o << _Q_;
			return ss.str();
		}
	};

	class Circle final : public Object, public PathProps<Circle>, private Quotable
	{
	  public:
		Circle() = default;
		Circle(double x, double y, double r) : center_({x, y}), radius_(r)
		{
		}
		Circle& SetCenter(Point center);
		Circle& SetRadius(double radius);
		void RenderObject(const RenderContext& context) const override;

		Point center_{0.0, 0.0};
		double radius_{1.0};
	};

	class Polyline : public Object, public PathProps<Polyline>, private Quotable
	{
	  public:
		// Добавляет очередную вершину к ломаной линии
		Polyline& AddPoint(Point point);

	  private:
		void RenderObject(const RenderContext& context) const override;

		/*
		 * Прочие методы и данные, необходимые для реализации элемента <polyline>
		 */
	  private:
		std::vector<Point> points_{};
	};

	class Text : public Object, public PathProps<Text>, private Quotable
	{
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

	  private:
		void RenderObject(const RenderContext& context) const override;

		// Прочие данные и методы, необходимые для реализации элемента <text>
	  private:
		Point position_{0.0, 0.0};
		Point offset_{0.0, 0.0};
		uint32_t font_size_{1};
		std::string font_family_;
		std::string font_weight_;
		std::string data_;

		const std::map<std::string, std::string> defs_ = {
			{R"(")", R"(&quot;)"}, {R"(')", R"(&apos;)"}, {R"(<)", R"(&lt;)"},
			{R"(>)", R"(&gt;)"},   {R"(&)", R"(&amp;)"},
		};

		std::string replace_all_(std::string& str, const std::string& from, const std::string& to)
		{
			size_t start_pos = 0;
			while ((start_pos = str.find(from, start_pos)) != std::string::npos)
			{
				str.replace(start_pos, from.length(), to);
				start_pos += to.length();
			}
			return str;
		}
	};

	class Document : public ObjectContainer
	{
	  public:
		/*
		 Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
		 Пример использования:
		 Document doc;
		 doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
		*/

		// Добавляет в svg-документ объект-наследник svg::Object
		void AddPtr(std::unique_ptr<Object>&& obj) override
		{
			objects_.push_back(std::move(obj));
		}

		// Выводит в ostream svg-представление документа
		void Render(std::ostream& out) const
		{
			using namespace std;
			svg::RenderContext ctx(out, 2, 2);
			out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
			out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
			for (const auto& rep : objects_)
			{
				rep->Render(ctx);
			}
			out << "</svg>"sv;
		}

		// Прочие методы и данные, необходимые для реализации класса Document

	  private:
		std::vector<std::unique_ptr<Object>> objects_;
	};

} // namespace svg

namespace shapes
{
	class Star : public svg::Drawable, public svg::PathProps<Star>
	{
	  public:
		Star(svg::Point center, double outer_radius, double inner_radius, int num_rays)
			: center_(center), outer_radius_(outer_radius), inner_radius_(inner_radius), num_rays_(num_rays)
		{
			fill_color_ = "red";
			stroke_color_ = "black";
		}

		void Draw(svg::ObjectContainer& container) const override;

	  private:
		svg::Point center_;
		double outer_radius_;
		double inner_radius_;
		int num_rays_;
	};

	class Snowman : public svg::Drawable, public svg::PathProps<Star>
	{
	  public:
		Snowman(svg::Point center, double head_radius) : center_(center), head_radius_(head_radius)
		{
			fill_color_ = "rgb(240,240,240)";
			stroke_color_ = "black";
		}

		void Draw(svg::ObjectContainer& container) const override;

	  private:
		svg::Point center_;
		double head_radius_;
	};

	class Triangle : public svg::Drawable, public svg::PathProps<Triangle>
	{
	  public:
		Triangle(svg::Point p1, svg::Point p2, svg::Point p3) : p1_(p1), p2_(p2), p3_(p3)
		{
		}

		// Реализует метод Draw интерфейса svg::Drawable
		void Draw(svg::ObjectContainer& container) const override
		{
			container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
		}

	  private:
		svg::Point p1_, p2_, p3_;
	};
} // namespace shapes