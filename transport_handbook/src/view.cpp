#include "map_renderer.h"
#include "view.h"
#include <iomanip>
#include <numeric>
#include <sstream>

static std::string DoubleToString(double num, bool fixed = false, size_t p = 6)
{
	std::ostringstream ss1;
	if (!fixed)
	{
		ss1 << std::setprecision(p) << num;
	}
	else
	{
		ss1 << std::fixed << std::setprecision(p) << num;
	}

	return ss1.str();
}

std::string Handbook::Views::GetData(const std::string& stat, const Handbook::Data::TransportCatalogue& t_q)
{
	using namespace std;
	auto bus_substr = "Bus "s;
	auto stop_substr = "Stop "s;
	if (stat.find(bus_substr) != stat.npos)
	{
		std::string bus(stat.begin() + bus_substr.size(), stat.end());
		auto info = t_q.GetRouteInfo(bus);
		if (info.has_value())
		{
			return "Bus "s + bus + ": " + std::to_string(std::get<0>(info.value())) + " stops on route, "s +
				   std::to_string(std::get<1>(info.value())) + " unique stops, " +
				   DoubleToString(double(std::get<2>(info.value())), false) + " route length, " +
				   DoubleToString(std::get<4>(info.value()), false) + " curvature";
		}
		else
		{
			return "Bus "s + bus + ": not found";
		}
	}
	else if (stat.find(stop_substr) != stat.npos)
	{
		std::string stop(stat.begin() + stop_substr.size(), stat.end());
		auto info = t_q.GetBusInfo(stop);
		if (info.has_value())
		{
			if (info.value().empty())
			{
				return "Stop "s + stop + ": no buses";
			}
			else
			{
				std::vector<std::string> buses(info.value().begin(), info.value().end());
				return "Stop "s + stop + ": buses " +
					   std::accumulate(buses.begin(), buses.end(), std::string(),
									   [](std::string& ss, std::string& s) { return ss.empty() ? s : ss + " " + s; });
			}
		}
		else
		{
			return "Stop "s + stop + ": not found";
		}
	}
	return {};
}
static svg::Color ParsingColor(json::Node& color)
{
	svg::Color out;
	if (color.IsString())
	{
		return color.AsString();
	}

	if (color.AsArray().size() == 3)
	{
		return svg::Rgb(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt());
	}

	return svg::Rgba(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt(),
					 color.AsArray()[3].AsDouble());
}

static Handbook::Renderer::RenderSettings ReadRenderSettings(json::Dict data)
{
	using namespace std;
	Handbook::Renderer::RenderSettings settings;
	settings.width = data["width"s].AsDouble();
	settings.height = data["height"s].AsDouble();
	settings.padding = data["padding"s].AsDouble();
	settings.line_width = data["line_width"s].AsDouble();
	settings.stop_radius = data["stop_radius"s].AsDouble();
	settings.bus_label_font_size = data["bus_label_font_size"s].AsDouble();
	settings.bus_label_offset = svg::Point(data["bus_label_offset"s].AsArray()[0].AsDouble(),
										   data["bus_label_offset"s].AsArray()[1].AsDouble());
	settings.stop_label_font_size = data["stop_label_font_size"s].AsInt();
	settings.stop_label_offset = svg::Point(data["stop_label_offset"s].AsArray()[0].AsDouble(),
											data["stop_label_offset"s].AsArray()[1].AsDouble());
	settings.underlayer_color = ParsingColor(data["underlayer_color"s]);
	settings.underlayer_width = data["underlayer_width"s].AsDouble();
	for (auto& color : data["color_palette"s].AsArray())
	{
		settings.color_palette.push_back(ParsingColor(color));
	}
	return settings;
}
json::Document Handbook::Views::GetData(const json::Document& stat, const Handbook::Data::TransportCatalogue& t_q)
{
	using namespace std;
	json::Node result;
	int id = stat.GetRoot().AsMap().at("id"s).AsInt();
	std::string type = stat.GetRoot().AsMap().at("type"s).AsString();

	if (type == "Bus"s)
	{
		std::string name = stat.GetRoot().AsMap().at("name"s).AsString();
		auto info = t_q.GetRouteInfo(name);
		if (info.has_value())
		{
			result = json::Dict{{"curvature"s, std::get<4>(info.value())},
								{"request_id"s, id},
								{"route_length"s, static_cast<int>(std::get<2>(info.value()))},
								{"stop_count"s, static_cast<int>(std::get<0>(info.value()))},
								{"unique_stop_count"s, static_cast<int>(std::get<1>(info.value()))}};
			return json::Document(result);
		}
	}
	if (type == "Stop"s)
	{
		std::string name = stat.GetRoot().AsMap().at("name"s).AsString();
		auto info = t_q.GetBusInfo(name);
		if (info.has_value())
		{
			std::vector<json::Node> buses(info.value().begin(), info.value().end());
			result = json::Dict{{"request_id"s, id}, {"buses"s, buses}};
			return json::Document(result);
		}
	}
	if (type == "Map"s)
	{

		Handbook::Renderer::RenderSettings renderSettings = ReadRenderSettings(stat.GetRoot().AsMap().at("render_settings").AsMap());

		return json::Document(Handbook::Renderer::GetMapOfRoad(t_q, renderSettings, id));
	}
	result = json::Dict{{"request_id"s, id}, {"error_message"s, "not found"s}};
	return json::Document(result);
}
