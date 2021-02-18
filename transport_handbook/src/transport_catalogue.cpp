#include "transport_catalogue.h"

void TransportCatalogue::AddRequest(Request* request)
{
	switch (request->getRequestType())
	{
	case RequestType::IsBus: {
		Bus* bus = static_cast<Bus*>(request);
		bus_to_stops_[bus->getName()] = bus->getStops();
		for (const auto& stop : bus->getStops())
		{
			stop_to_bus_[stop].insert(bus->getName());
		}
		break;
	}
	case RequestType::IsStop: {
	}
		Stop* stop = static_cast<Stop*>(request);
		stops_[stop->getName()] = stop->coordinates;
		if (stop_to_bus_.find(stop->getName()) == stop_to_bus_.end())
		{
			stop_to_bus_[stop->getName()] = {};
		}
		for (const auto& [name, distance] : stop->getDistanceToOtherStop())
		{
			distance_between_stops_[stop->getName()].insert({name, distance});
		}
		break;
	}
}

double TransportCatalogue::RoutePathSize(const std::vector<std::string>& stops) const
{
	double result = 0.0;
	for (auto i = 0; i < stops.size() - 1; ++i)
	{
		result += ComputeDistance(stops_.at(stops[i]), stops_.at(stops[i + 1]));
	}
	return result;
}

std::optional<std::tuple<size_t, size_t, double, std::vector<std::string>>> TransportCatalogue::GetRouteInfo(
	const std::string& bus) const
{
	auto it = bus_to_stops_.find(bus);
	if (it == bus_to_stops_.end())
	{
		return {};
	}
	else
	{
		auto stops = bus_to_stops_.at(bus);

		return std::make_tuple(stops.size(), std::set<std::string>(stops.begin(), stops.end()).size(),
							   RoutePathSize(stops), stops);
	}
}

std::optional<std::set<std::string>> TransportCatalogue::GetBusInfo(const std::string& stop) const
{
	auto it = stop_to_bus_.find(stop);
	if (it == stop_to_bus_.end())
	{
		return {};
	}
	else
	{
		return stop_to_bus_.at(stop);
	}
}
