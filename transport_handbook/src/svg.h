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
#include <vector>

namespace svg
{

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
	/*
	 * Класс Circle моделирует элемент <circle> для отображения круга
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
	 */
	class Circle final : public Object
	{
	  public:
		Circle() = default;
		Circle(double x, double y, double r) : center_({x, y}), radius_(r)
		{
		}
		Circle& SetCenter(Point center);
		Circle& SetRadius(double radius);

	  private:
		void RenderObject(const RenderContext& context) const override;

		Point center_{0.0, 0.0};
		double radius_{1.0};
	};

	/*
	 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
	 */
	class Polyline : public Object
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

	/*
	 * Класс Text моделирует элемент <text> для отображения текста
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
	 */
	class Text : public Object
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
		const std::string_view _Q_{R"(")"};

		template <typename T> std::string _q_(T o) const
		{
			std::stringstream ss;
			ss << _Q_ << o << _Q_;
			return ss.str();
		}
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
	class Star : public svg::Drawable
	{
	  public:
		Star(svg::Point center, double outer_radius, double inner_radius, int num_rays)
			: center_(center), outer_radius_(outer_radius), inner_radius_(inner_radius), num_rays_(num_rays)
		{
		}

		void Draw(svg::ObjectContainer& container) const override
		{
			svg::Polyline polyline;
			for (int i = 0; i <= num_rays_; ++i)
			{
				double angle = 2 * M_PI * (i % num_rays_) / num_rays_;
				polyline.AddPoint({center_.x + outer_radius_ * sin(angle), center_.y - outer_radius_ * cos(angle)});
				if (i == num_rays_)
				{
					break;
				}
				angle += M_PI / num_rays_;
				polyline.AddPoint({center_.x + inner_radius_ * sin(angle), center_.y - inner_radius_ * cos(angle)});
			}
			container.Add(polyline);
		}

	  private:
		svg::Point center_;
		double outer_radius_;
		double inner_radius_;
		int num_rays_;
	};

	class Snowman : public svg::Drawable
	{
	  public:
		Snowman(svg::Point center, double head_radius) : center_(center), head_radius_(head_radius)
		{
		}

		void Draw(svg::ObjectContainer& container) const override
		{
			container.Add(svg::Circle(center_.x, (center_.y + (head_radius_ * 5)), head_radius_ * 2));
			container.Add(svg::Circle(center_.x, (center_.y + (head_radius_ * 2)), head_radius_ * 1.5));
			container.Add(svg::Circle(center_.x, center_.y, head_radius_));
		}

	  private:
		svg::Point center_;
		double head_radius_;
	};

	class Triangle : public svg::Drawable
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