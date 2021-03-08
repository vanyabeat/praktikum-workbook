#include "svg.h"
#include <sstream>

static std::string join_coords(const std::vector<svg::Point>& points)
{
	std::stringstream ss;
	auto size = points.size();
	if (points.empty())
	{
		return "";
	}
	size_t counter = 0;
	for (const auto& p : points)
	{
		if (counter != size - 1)
		{
			ss << p << " ";
		}
		++counter;
	}
	ss << points[size - 1];

	return ss.str();
}

namespace svg
{

	using namespace std::literals;

	void Object::Render(const RenderContext& context) const
	{
		context.RenderIndent();

		// Делегируем вывод тега своим подклассам
		RenderObject(context);

		context.out << std::endl;
	}

	// ---------- Circle ------------------

	Circle& Circle::SetCenter(Point center)
	{
		center_ = center;
		return *this;
	}

	Circle& Circle::SetRadius(double radius)
	{
		radius_ = radius;
		return *this;
	}

	void Circle::RenderObject(const RenderContext& context) const
	{
		auto& out = context.out;
		out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
		out << "r=\""sv << radius_ << "\" "sv;
		out << "/>"sv;
	}

	Polyline& Polyline::AddPoint(Point point)
	{
		points_.push_back(std::move(point));
		return *this;
	}

	void Polyline::RenderObject(const RenderContext& context) const
	{
		auto& out = context.out;

		out << R"(<polyline points=")"sv << join_coords(points_);

		out << R"(" />)"sv;
	}

	void Text::RenderObject(const RenderContext& context) const
	{
		auto& out = context.out;

		out << R"(<text x=)"sv << _q_(position_.x) << R"( y=)"sv << _q_(position_.y) << R"( dx=)"sv << _q_(offset_.x)
			<< R"( dy=)"sv << _q_(offset_.y) << R"( font-size=)"sv << _q_(font_size_);
		if (!font_family_.empty())
		{
			out << R"( font-family=)"sv << _q_(font_family_);
		}
		if (!font_weight_.empty())
		{
			out << R"( font-weight=)"sv << _q_(font_weight_);
		}
		out << R"(>)"sv;
		out << data_ << R"(</text>)";
	}

	Text& Text::SetPosition(Point pos)
	{
		position_ = std::move(pos);
		return *this;
	}

	Text& Text::SetOffset(Point offset)
	{
		offset_ = std::move(offset);
		return *this;
	}
	Text& Text::SetFontSize(uint32_t size)
	{
		font_size_ = size;
		return *this;
	}
	Text& Text::SetFontFamily(std::string font_family)
	{
		font_family_ = std::move(font_family);
		return *this;
	}
	Text& Text::SetFontWeight(std::string font_weight)
	{
		font_weight_ = std::move(font_weight);
		return *this;
	}
	Text& Text::SetData(std::string data)
	{
		data_ = std::move(data);
		for (auto c : data_)
		{
			for (auto& [key, value] : defs_)
			{
				if (key[0] == c)
				{
					replace_all_(data_, key, value);
				}
				continue;
			}
		}
		return *this;
	}

} // namespace svg