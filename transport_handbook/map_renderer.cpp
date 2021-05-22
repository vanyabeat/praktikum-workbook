#include "map_renderer.h"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std::literals;

bool Handbook::Renderer::IsZero(double value)
{
	return std::abs(value) < EPSILON;
}

namespace
{
	static std::map<std::string_view, Handbook::Data::StopPtr> GetUniqueStopsByName(
		const Handbook::Renderer::BusesByName& buses)
	{
		std::map<std::string_view, Handbook::Data::StopPtr> stops;

		for (const auto& [_, bus] : buses)
		{
			for (const auto stop : bus->stops)
			{
				stops.emplace(stop->name, stop);
			}
		}

		return stops;
	}
} // namespace

void Handbook::Renderer::Map::Render(const BusesByName& buses_by_name, std::ostream& out) const
{
	const auto unique_stops = ::GetUniqueStopsByName(buses_by_name);
	if (unique_stops.empty())
	{
		return;
	}

	std::unordered_set<Handbook::Utilities::Coordinates, Handbook::Utilities::CoordinatesHash> coordinates;
	for (const auto stop : unique_stops)
	{
		coordinates.insert(stop.second->coordinates);
	}

	SphereProjector projector(coordinates.begin(), coordinates.end(), render_props_.width, render_props_.height,
							  render_props_.padding);

	svg::Document document;

	DrawLineOfRoad(document, projector, buses_by_name);
	DrawBusTitles(document, projector, buses_by_name);
	DrawStops(document, projector, unique_stops);
	DrawStopTitles(document, projector, unique_stops);

	document.Render(out);
}
void Handbook::Renderer::Map::DrawLineOfRoad(svg::Document& document,
											 const Handbook::Renderer::SphereProjector& projector,
											 const BusesByName& buses) const
{
	using namespace std;
	int i = 0;
	for (const auto& [_, bus] : buses)
	{
		std::vector<const Handbook::Data::Stop*> stop_ptrs = {bus->stops.begin(), bus->stops.end()};

		if (!bus->is_roundtrip)
		{
			stop_ptrs.insert(stop_ptrs.end(), next(bus->stops.rbegin()), bus->stops.rend());
		}

		size_t color_index = i % render_props_.color_palette.size();

		svg::Polyline polyline;
		polyline.SetFillColor(svg::NoneColor)
			.SetStrokeColor(render_props_.color_palette[color_index])
			.SetStrokeWidth(render_props_.line_width)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		for (const auto stop : stop_ptrs)
		{
			polyline.AddPoint(projector(stop->coordinates));
		}

		document.Add(polyline);
		++i;
	}
}
void Handbook::Renderer::Map::DrawBusTitles(svg::Document& document,
											  const Handbook::Renderer::SphereProjector& projector,
											  const BusesByName& buses) const
{
	int i = 0;
	for (const auto& [name, bus] : buses)
	{
		size_t color_index = i % render_props_.color_palette.size();

		std::vector<Handbook::Data::StopPtr> end_stops = {bus->stops.front()};
		if (!bus->is_roundtrip && bus->stops.back() != bus->stops.front())
		{
			end_stops.emplace_back(bus->stops.back());
		}

		for (const auto stop : end_stops)
		{
			DrawBusTitle(document, name, projector(stop->coordinates), render_props_.color_palette[color_index]);
		}

		++i;
	}
}
void Handbook::Renderer::Map::DrawBusTitle(svg::Document& document, std::string_view name, svg::Point pos,
											 const svg::Color& color) const
{
	svg::Text underlayer;
	document.Add(underlayer.SetData(std::string{name})
					 .SetFontFamily("Verdana"s)
					 .SetFontWeight("bold"s)
					 .SetFontSize(render_props_.bus_label_font_size)
					 .SetFillColor(render_props_.underlayer_color)
					 .SetStrokeColor(render_props_.underlayer_color)
					 .SetStrokeWidth(render_props_.underlayer_width)
					 .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
					 .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
					 .SetPosition(pos)
					 .SetOffset(render_props_.bus_label_offset));

	svg::Text title;
	document.Add(title.SetData(std::string{name})
					 .SetFontFamily("Verdana"s)
					 .SetFontWeight("bold"s)
					 .SetFontSize(render_props_.bus_label_font_size)
					 .SetFillColor(color)
					 .SetPosition(pos)
					 .SetOffset(render_props_.bus_label_offset));
}
void Handbook::Renderer::Map::DrawStops(svg::Document& document, const Handbook::Renderer::SphereProjector& projector,
										  const std::map<std::string_view, Handbook::Data::StopPtr>& stops) const
{
	for (const auto& [name, stop] : stops)
	{
		svg::Circle circle;
		document.Add(
			circle.SetRadius(render_props_.stop_radius).SetFillColor("white"s).SetCenter(projector(stop->coordinates)));
	}
}
void Handbook::Renderer::Map::DrawStopTitles(svg::Document& document,
											   const Handbook::Renderer::SphereProjector& projector,
											   const std::map<std::string_view, Handbook::Data::StopPtr>& stops) const
{
	for (const auto& [name, stop] : stops)
	{
		svg::Text underlayer;
		document.Add(underlayer.SetData(std::string{name})
						 .SetFontFamily("Verdana"s)
						 .SetFontSize(render_props_.stop_label_font_size)
						 .SetFillColor(render_props_.underlayer_color)
						 .SetStrokeColor(render_props_.underlayer_color)
						 .SetStrokeWidth(render_props_.underlayer_width)
						 .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
						 .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
						 .SetPosition(projector(stop->coordinates))
						 .SetOffset(render_props_.stop_label_offset));

		svg::Text title;
		document.Add(title.SetData(std::string{name})
						 .SetFontFamily("Verdana"s)
						 .SetFontSize(render_props_.stop_label_font_size)
						 .SetFillColor("black"s)
						 .SetPosition(projector(stop->coordinates))
						 .SetOffset(render_props_.stop_label_offset));
	}
}
