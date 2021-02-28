#include "transport_catalogue.h"

void TransportCatalogue::AddRequest(Request* request)
{
	switch (request->getRequestType())
	{
	case RequestType::IsBus: {
		Bus* bus = static_cast<Bus*>(request);
		AddBus(bus->getName(), bus->getStops());
		break;
	}
	case RequestType::IsStop: {
	}
		Stop* stop = static_cast<Stop*>(request);
		AddStop(stop->getName(), stop->coordinates, stop->getDistanceToOtherStop());
		break;
	}
}

double TransportCatalogue::RoutePathSizeNaive(const std::vector<std::string>& stops) const
{
	double result = 0.0;
	for (auto i = 0; i < stops.size() - 1; ++i)
	{
		result += ComputeDistance(stops_.at(stops[i]), stops_.at(stops[i + 1]));
	}
	return result;
}

std::optional<std::tuple<size_t, size_t, size_t, std::vector<std::string>, double>> TransportCatalogue::GetRouteInfo(
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
		size_t path_size = RoutePathSize(stops);
		double naive_size = RoutePathSizeNaive(stops);
		return std::make_tuple(stops.size(), std::set<std::string>(stops.begin(), stops.end()).size(), path_size, stops,
							   (path_size / naive_size));
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
size_t TransportCatalogue::GetDistanceBetweenStop(const std::string& stop_l, const std::string& stop_r) const
{

	if (distance_between_stops_.find(stop_l) != distance_between_stops_.end())
	{
		if (distance_between_stops_.at(stop_l).find(stop_r) != distance_between_stops_.at(stop_l).end())
		{
			return distance_between_stops_.at(stop_l).at(stop_r);
		}
		else
		{
			return GetDistanceBetweenStop(stop_r, stop_l);
		}
	}
	return GetDistanceBetweenStop(stop_r, stop_l);
}
size_t TransportCatalogue::RoutePathSize(const std::vector<std::string>& stops) const
{
	double result = 0.0;
	for (auto i = 0; i < stops.size() - 1; ++i)
	{
		result += GetDistanceBetweenStop(stops[i], stops[i + 1]);
	}
	return result;
}

void TransportCatalogue::AddStop(std::string stop_name, Coordinates coordinates,
								 std::vector<std::pair<std::string, size_t>> vector_distances_to_other_stop)
{
	stops_[stop_name] = std::move(coordinates);
	if (stop_to_bus_.find(stop_name) == stop_to_bus_.end())
	{
		stop_to_bus_[stop_name] = {};
	}
	for (const auto& [name, distance] : vector_distances_to_other_stop)
	{
		distance_between_stops_[stop_name].insert({std::move(name), std::move(distance)});
	}
}

void TransportCatalogue::AddBus(std::string bus_name, std::vector<std::string> stops)
{
	bus_to_stops_[bus_name] = std::move(stops);
	for (const auto& stop : bus_to_stops_[bus_name])
	{
		stop_to_bus_[stop].insert(bus_name);
	}
}
