#include "transport_catalogue.h"

void TransportCatalogue::AddRequest(Request* request)
{
	switch (request->getRequestType())
	{
	case RequestType::IsBus: {
		Bus* bus = static_cast<Bus*>(request);
		bus_to_stops_[bus->getName()] = bus->getStops();
		break;
	}
	case RequestType::IsStop: {
	}
		Stop* stop = static_cast<Stop*>(request);
		stops_[stop->getName()] = stop->coordinates;
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
