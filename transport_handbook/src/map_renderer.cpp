#include "map_renderer.h"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std::literals;

bool Handbook::Renderer::IsZero(double value)
{
	return std::abs(value) < EPSILON;
}

Handbook::Renderer::SphereProjector Handbook::Renderer::CreatorSphereProjector(
	std::deque<Handbook::Data::Stop*>& stops, Handbook::Renderer::RenderSettings& settings)
{
	std::vector<Handbook::Utilities::Coordinates> points;
	for (auto stop : stops)
	{
		points.push_back(std::get<1>(*stop));
	}
	return Handbook::Renderer::SphereProjector(points.begin(), points.end(), settings.width, settings.height,
											   settings.padding);
}

std::vector<svg::Polyline> Handbook::Renderer::DrawLineofRoad(const std::deque<Handbook::Data::Bus*>& buses,
															  Handbook::Renderer::RenderSettings& settings,
															  Handbook::Renderer::SphereProjector& projector)
{
	std::vector<svg::Polyline> lines;
	size_t i = 0;
	for (const auto bus : buses)
	{
		if (!std::get<1>(*bus).empty() && !settings.color_palette.empty())
		{
			for (const auto stop : std::get<1>(*bus))
			{
				size_t color_index = i % settings.color_palette.size();
				svg::Polyline polyline;
				polyline.SetFillColor(svg::NoneColor)
					.SetStrokeColor(settings.color_palette.at(color_index))
					.SetStrokeWidth(settings.line_width)
					.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
					.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
					.AddPoint(projector(std::get<1>(stop)));
				lines.push_back(polyline);
			}
		}

		++i;
	}
	return lines;
}

std::vector<svg::Text> Handbook::Renderer::DrawNameOfRoad(const std::deque<Handbook::Data::Bus*>& buses,
														  Handbook::Renderer::RenderSettings& settings,
														  Handbook::Renderer::SphereProjector& projector)
{
	std::vector<svg::Text> NameOfRoad;
	size_t cnt_color_palette = 0;

	for (const auto bus : buses)
	{
		auto buses = std::get<1>(*bus);
		auto is_roundtrip = std::get<6>(*bus);
		if (!buses.empty())
		{
			if (!is_roundtrip && buses[0] != buses[(buses.size() / 2)])
			{
				svg::Text text_first_first_stop;
				svg::Text text_second_first_stop;
				svg::Text text_first_secon_stop;
				svg::Text text_second_second_stop;

				text_first_first_stop.SetPosition(projector(std::get<1>(buses[0])))
					.SetOffset(settings.bus_label_offset)
					.SetFontSize(settings.bus_label_font_size)
					.SetFontFamily("Verdana"s)
					.SetFontWeight("bold")
					.SetData(std::get<0>(*bus))
					.SetFillColor(settings.color_palette[cnt_color_palette]);

				text_second_first_stop.SetPosition(projector(std::get<1>(buses[0])))
					.SetOffset(settings.bus_label_offset)
					.SetFontSize(settings.bus_label_font_size)
					.SetFontFamily("Verdana"s)
					.SetFontWeight("bold")
					.SetData(std::get<0>(*bus))
					.SetFillColor(settings.underlayer_color)
					.SetStrokeColor(settings.underlayer_color)
					.SetStrokeWidth(settings.underlayer_width)
					.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
					.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

				text_first_secon_stop.SetPosition(projector(std::get<1>(buses[(buses.size() / 2)])))
					.SetOffset(settings.bus_label_offset)
					.SetFontSize(settings.bus_label_font_size)
					.SetFontFamily("Verdana"s)
					.SetFontWeight("bold")
					.SetData(std::get<0>(*bus))
					.SetFillColor(settings.color_palette[cnt_color_palette]);

				text_second_second_stop.SetPosition(projector(std::get<1>(buses[(buses.size() / 2)])))
					.SetOffset(settings.bus_label_offset)
					.SetFontSize(settings.bus_label_font_size)
					.SetFontFamily("Verdana"s)
					.SetFontWeight("bold")
					.SetData(std::get<0>(*bus))
					.SetFillColor(settings.underlayer_color)
					.SetStrokeColor(settings.underlayer_color)
					.SetStrokeWidth(settings.underlayer_width)
					.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
					.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

				NameOfRoad.push_back(text_second_first_stop);
				NameOfRoad.push_back(text_first_first_stop);

				NameOfRoad.push_back(text_second_second_stop);
				NameOfRoad.push_back(text_first_secon_stop);
			}
			else
			{
				svg::Text text_first;
				svg::Text text_second;
				text_first.SetPosition(projector(std::get<1>(buses[0])))
					.SetOffset(settings.bus_label_offset)
					.SetFontSize(settings.bus_label_font_size)
					.SetFontFamily("Verdana"s)
					.SetFontWeight("bold")
					.SetData(std::get<0>(*bus))
					.SetFillColor(settings.color_palette[cnt_color_palette]);

				text_second.SetPosition(projector(std::get<1>(buses[0])))
					.SetOffset(settings.bus_label_offset)
					.SetFontSize(settings.bus_label_font_size)
					.SetFontFamily("Verdana"s)
					.SetFontWeight("bold")
					.SetData(std::get<0>(*bus))
					.SetFillColor(settings.underlayer_color)
					.SetStrokeColor(settings.underlayer_color)
					.SetStrokeWidth(settings.underlayer_width)
					.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
					.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

				NameOfRoad.push_back(text_second);
				NameOfRoad.push_back(text_first);
			}

			if (cnt_color_palette < settings.color_palette.size())
			{
				++cnt_color_palette;
			}
			else
			{
				cnt_color_palette = 0;
			}
		}
	}
	return NameOfRoad;
}

std::vector<svg::Circle> Handbook::Renderer::DrawStop(const std::deque<Handbook::Data::Stop*>& stops,
													  Handbook::Renderer::RenderSettings& settings,
													  Handbook::Renderer::SphereProjector& projector)
{
	std::vector<svg::Circle> stops_point;
	for (const auto stop : stops)
	{
		svg::Circle stop_point;
		stop_point.SetCenter(projector(std::get<1>(*stop))).SetRadius(settings.stop_radius).SetFillColor("white"s);
		stops_point.push_back(stop_point);
	}

	return stops_point;
}

std::vector<svg::Text> Handbook::Renderer::DrawStopName(const std::deque<Handbook::Data::Stop*>& stops,
														Handbook::Renderer::RenderSettings& settings,
														Handbook::Renderer::SphereProjector& projector)
{
	std::vector<svg::Text> stops_name;
	for (const auto stop : stops)
	{
		svg::Text first_text;
		svg::Text second_text;
		first_text.SetPosition(projector(std::get<1>(*stop)))
			.SetOffset(settings.stop_label_offset)
			.SetFontSize(settings.stop_label_font_size)
			.SetFontFamily("Verdana"s)
			.SetData(std::get<0>(*stop))
			.SetFillColor("black"s);
		second_text.SetPosition(projector(std::get<1>(*stop)))
			.SetOffset(settings.stop_label_offset)
			.SetFontSize(settings.stop_label_font_size)
			.SetFontFamily("Verdana"s)
			.SetData(std::get<0>(*stop))
			.SetFillColor(settings.underlayer_color)
			.SetStrokeColor(settings.underlayer_color)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
			.SetStrokeWidth(settings.underlayer_width);
		stops_name.push_back(second_text);
		stops_name.push_back(first_text);
	}
	return stops_name;
}
#include <fstream>
json::Node Handbook::Renderer::GetMapOfRoad(const Handbook::Data::TransportCatalogue& transport_catalog,
											Handbook::Renderer::RenderSettings& settings, int id)
{
	auto raw_stops = transport_catalog.GetStops();
	std::deque<Handbook::Data::Stop*> stops;
	for (auto stop : raw_stops)
	{
		if (!transport_catalog.GetBusOfStop(std::get<0>(*stop)).empty())
		{
			stops.push_back(stop);
		}
	}

	Handbook::Renderer::SphereProjector projector = Handbook::Renderer::CreatorSphereProjector(stops, settings);

	auto buses_ = transport_catalog.GetBuses();
	std::deque<Handbook::Data::Bus*> buses;
	buses.resize(buses.size());
	std::move(begin(buses_), end(buses_), back_inserter(buses));
	std::sort(buses.begin(), buses.end(), [](auto& left, auto& right) {
		return std::lexicographical_compare(std::get<0>(*left).begin(), std::get<0>(*left).end(),
											std::get<0>(*right).begin(), std::get<0>(*right).end());
		/*return left->name_ < right->name_;*/
	});

	std::vector<svg::Polyline> lines = Handbook::Renderer::DrawLineofRoad(buses, settings, projector);
	std::vector<svg::Text> NamesOfRoad = Handbook::Renderer::DrawNameOfRoad(buses, settings, projector);

	std::sort(stops.begin(), stops.end(), [](auto left, auto right) {
		return std::lexicographical_compare(std::get<0>(*left).begin(), std::get<0>(*left).end(),
											std::get<0>(*right).begin(), std::get<0>(*right).end());
		/*return left->name_ < right->name_;*/
	});

	std::vector<svg::Circle> stop_points = Handbook::Renderer::DrawStop(stops, settings, projector);

	std::vector<svg::Text> stop_names = Handbook::Renderer::DrawStopName(stops, settings, projector);

	svg::Document doc;

	for (auto& line : lines)
	{
		doc.Add(std::move(line));
	}

	for (auto& name : NamesOfRoad)
	{
		doc.Add(std::move(name));
	}

	for (auto& point : stop_points)
	{
		doc.Add(std::move(point));
	}

	for (auto& name : stop_names)
	{
		doc.Add(std::move(name));
	}

	//	std::ofstream myfile("result.svg");
	//	if (myfile.is_open())
	//	{
	//		doc.Render(myfile);
	//	}
	//
	//
	//  return json::Node();

	std::stringstream svg_data;
	doc.Render(svg_data);
	json::Dict out;
	out.insert({"request_id", json::Node(id)});
	out.insert({"map", json::Node(svg_data.str())});
	return json::Node(out);
}
