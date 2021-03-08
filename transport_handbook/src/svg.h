#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
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

	/*
	 * Класс Circle моделирует элемент <circle> для отображения круга
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
	 */
	class Circle final : public Object
	{
	  public:
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

	class Document
	{
	  public:
		/*
		 Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
		 Пример использования:
		 Document doc;
		 doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
		*/

		template <typename Obj> void Add(Obj obj)
		{
			objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
		}

		// Добавляет в svg-документ объект-наследник svg::Object
		void AddPtr(std::unique_ptr<Object>&& obj)
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