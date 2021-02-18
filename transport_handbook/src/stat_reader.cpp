#include "stat_reader.h"
#include <numeric>
#include <sstream>

std::string DoubleToString(double num)
{
	std::ostringstream ss1;
	ss1.precision(6);
	ss1 << num;
	return ss1.str();
}
std::string ReadStat(const std::string& stat, const TransportCatalogue& t_q)
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
				   DoubleToString(size_t(std::get<2>(info.value()))) + " route length, " + DoubleToString(std::get<4>(info.value())) + " curvature";
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
				;
			}
		}
		else
		{
			return "Stop "s + stop + ": not found";
		}
	}
	return {};
}
