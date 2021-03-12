#include "map_renderer.h"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std::literals;

bool IsZero(double value)
{
	return std::abs(value) < EPSILON;
}

SphereProjector CreatorSphereProjector(std::deque<Handbook::Data::Stop*>& stops, RenderSettings& settings)
{
	std::vector<Handbook::Utilities::Coordinates> points;
	for (auto stop : stops)
	{
		points.push_back(std::get<1>(*stop));
	}
	return SphereProjector(points.begin(), points.end(), settings.width, settings.height, settings.padding);
}

std::vector<svg::Polyline> DrawLineofRoad(const std::deque<Handbook::Data::Bus*>& buses, RenderSettings& settings,
										  SphereProjector& projector)
{
	std::vector<svg::Polyline> lines;
	size_t cnt_color_palette = 0;
	for (const auto bus : buses)
	{
		if (!std::get<1>(*bus).empty())
		{
			svg::Polyline line;
			for (auto& stop : std::get<1>(*bus))
			{
				line.AddPoint(projector(std::get<1>(stop)));
				line.SetStrokeColor(settings.color_palette[cnt_color_palette]);
				line.SetStrokeWidth(settings.line_width);
				line.SetFillColor(svg::NoneColor);
				line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
				line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			}

			//			if (!std::get<6>(bus))
			//			{
			//				for (auto itr = bus->route_stops_.rbegin() + 1; itr < bus->route_stops_.rend(); ++itr)
			//				{
			//					line.AddPoint(projector((*itr)->coordinates_));
			//				}
			//			}

			//      if (bus->route_stops_.size() == 1) {
			//	  line.AddPoint(projector(bus->route_stops_[0]->coordinates_));
			//      }

			if (cnt_color_palette < settings.color_palette.size())
			{
				++cnt_color_palette;
			}
			else
			{
				cnt_color_palette = 0;
			}
			lines.push_back(line);
		}
	}
	return lines;
}

std::vector<svg::Text> DrawNameOfRoad(const std::deque<Handbook::Data::Bus*>& buses, RenderSettings& settings,
									  SphereProjector& projector)
{
	std::vector<svg::Text> NameOfRoad;
	size_t cnt_color_palette = 0;

	for (const auto bus : buses)
	{
		auto buses = std::get<1>(*bus);
		auto is_roundtrip = std::get<6>(*bus);
		if (!buses.empty())
		{
			if (is_roundtrip)
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
			else if (!is_roundtrip && buses[0] != buses[(buses.size() / 2)])
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

std::vector<svg::Circle> DrawStop(const std::deque<Handbook::Data::Stop*>& stops, RenderSettings& settings,
								  SphereProjector& projector)
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

std::vector<svg::Text> DrawStopName(const std::deque<Handbook::Data::Stop*>& stops, RenderSettings& settings,
									SphereProjector& projector)
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
json::Node GetMapOfRoad(const Handbook::Data::TransportCatalogue& transport_catalog, RenderSettings& settings, int id)
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

	SphereProjector projector = CreatorSphereProjector(stops, settings);

	auto buses_ = transport_catalog.GetBuses();
	std::deque<Handbook::Data::Bus*> buses;
	buses.resize(buses.size());
	std::move(begin(buses_), end(buses_), back_inserter(buses));
	std::sort(buses.begin(), buses.end(), [](auto& left, auto& right) {
		return std::lexicographical_compare(std::get<0>(*left).begin(), std::get<0>(*left).end(),
											std::get<0>(*right).begin(), std::get<0>(*right).end());
		/*return left->name_ < right->name_;*/
	});

	std::vector<svg::Polyline> lines = DrawLineofRoad(buses, settings, projector);
	std::vector<svg::Text> NamesOfRoad = DrawNameOfRoad(buses, settings, projector);

	std::sort(stops.begin(), stops.end(), [](auto left, auto right) {
		return std::lexicographical_compare(std::get<0>(*left).begin(), std::get<0>(*left).end(),
											std::get<0>(*right).begin(), std::get<0>(*right).end());
		/*return left->name_ < right->name_;*/
	});

	std::vector<svg::Circle> stop_points = DrawStop(stops, settings, projector);

	std::vector<svg::Text> stop_names = DrawStopName(stops, settings, projector);

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

	std::ofstream myfile("result.svg");
	if (myfile.is_open())
	{
		doc.Render(myfile);
	}

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
