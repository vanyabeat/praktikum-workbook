#include "view_data.h"
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
json::Document Handbook::Views::GetData(const json::Document& stat, const Handbook::Data::TransportCatalogue& t_q)
{
	using namespace std;
	json::Node result;
	int id = stat.GetRoot().AsMap().at("id"s).AsInt();
	std::string type = stat.GetRoot().AsMap().at("type"s).AsString();
	std::string name = stat.GetRoot().AsMap().at("name"s).AsString();
	if (type == "Bus"s)
	{
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
		auto info = t_q.GetBusInfo(name);
		if (info.has_value())
		{
			std::vector<json::Node> buses(info.value().begin(), info.value().end());
			result = json::Dict{{"request_id"s, id}, {"buses"s, buses}};
			return json::Document(result);
		}

	}
	result = json::Dict{{"request_id"s, id}, {"error_message"s, "not found"s}};
	return json::Document(result);
}
