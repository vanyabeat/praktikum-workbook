#include "stat_reader.h"
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
	std::string bus(stat.begin() + "Bus "s.size(), stat.end());
	auto info = t_q.GetRouteInfo(bus);
	if (info.has_value())
	{

		return "Bus "s + bus + ": " + std::to_string(std::get<0>(info.value())) + " stops on route, "s +
			   std::to_string(std::get<1>(info.value())) + " unique stops, " + DoubleToString(std::get<2>(info.value())) + " route length";
	}
	else
	{
		return "Bus "s + bus + ": not found";
	}
}
