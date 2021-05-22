#include "transport_catalogue.h"

double Handbook::Data::Bus::GetCoordinateLength() const
{
	double length = std::transform_reduce(
		next(stops.begin()), stops.end(), stops.begin(), 0.0, std::plus<>{}, [](const auto stop1, const auto stop2) {
			return Handbook::Utilities::ComputeDistance(stop1->coordinates, stop2->coordinates);
		});

	return is_roundtrip ? length : length * 2;
}

const Handbook::Data::Stop* Handbook::Data::TransportCatalogue::FindStop(std::string_view name) const
{
	const auto stop_it = stops_by_name_.find(name);
	if (stop_it == stops_by_name_.end())
	{
		return nullptr;
	}
	else
	{
		return stop_it->second;
	}
}

void Handbook::Data::TransportCatalogue::AddStop(std::string_view name, Handbook::Utilities::Coordinates coordinates)
{
	const auto& stop = stops_.emplace_back(Stop{std::string(name), coordinates});
	stops_by_name_.insert({stop.name, &stop});
}

const std::unordered_set<Handbook::Data::BusPtr>* Handbook::Data::TransportCatalogue::GetBusesOnStop(
	const Handbook::Data::Stop* stop) const
{
	const auto needle = buses_by_stop_.find(stop);

	if (needle == buses_by_stop_.end())
	{
		static const std::unordered_set<BusPtr> empty_set;
		return &empty_set;
	}

	return &needle->second;
}

void Handbook::Data::TransportCatalogue::AddStopsDistance(std::string_view from_stop, std::string_view to_stop,
														  int distance)
{
	auto p = std::make_pair(FindStop(from_stop), FindStop(to_stop));
	auto res = std::make_pair(p, distance);

	stop_distances_.insert(res);
}

std::unordered_set<Handbook::Data::BusPtr> Handbook::Data::TransportCatalogue::GetBusesWithStops() const
{
	std::unordered_set<BusPtr> buses;

	for (const auto& bus : buses_)
	{
		if (!bus.stops.empty())
		{
			buses.insert(&bus);
		}
	}

	return buses;
}

int Handbook::Data::TransportCatalogue::FindStopsDistance(const Handbook::Data::Stop* from_stop_ptr,
														  const Handbook::Data::Stop* to_stop_ptr) const
{
	auto distance_it = stop_distances_.find({from_stop_ptr, to_stop_ptr});

	if (distance_it == stop_distances_.end())
	{
		distance_it = stop_distances_.find({to_stop_ptr, from_stop_ptr});
	}

	return distance_it->second;
}
void Handbook::Data::TransportCatalogue::AddBus(std::string_view name, const std::vector<std::string>& bus_stops,
												bool is_roundtrip)
{
	std::vector<const Stop*> stops;
	stops.reserve(bus_stops.size());

	auto& bus = buses_.emplace_back(Bus{std::string(name), is_roundtrip, stops});

	for (const auto& stop_name : bus_stops)
	{
		const auto* stop_ptr = FindStop(stop_name);
		bus.stops.push_back(stop_ptr);
		buses_by_stop_[stop_ptr].emplace(&bus);
	}

	buses_by_name_.insert({bus.name, &bus});
}

Handbook::Data::BusStat Handbook::Data::TransportCatalogue::GetBusStat(const Handbook::Data::Bus* bus) const
{
	std::vector<const Stop*> stop_ptrs = {bus->stops.begin(), bus->stops.end()};

	std::unordered_set<const Stop*> unique_stop_ptrs;
	unique_stop_ptrs.reserve(stop_ptrs.size());

	for_each(stop_ptrs.begin(), stop_ptrs.end(), [&unique_stop_ptrs](const auto stop) {
		if (!unique_stop_ptrs.count(stop))
		{
			unique_stop_ptrs.insert(stop);
		}
	});

	if (!bus->is_roundtrip)
	{
		stop_ptrs.insert(stop_ptrs.end(), next(bus->stops.rbegin()), bus->stops.rend());
	}

	double route_length = transform_reduce(
		next(stop_ptrs.begin()), stop_ptrs.end(), stop_ptrs.begin(), 0.0, std::plus<>{},
		[this](const auto stop_to, const auto stop_from) { return FindStopsDistance(stop_from, stop_to); });

	double curvature = route_length / bus->GetCoordinateLength();

	return {static_cast<int>(stop_ptrs.size()), static_cast<int>(unique_stop_ptrs.size()), route_length, curvature};
}

const Handbook::Data::Bus* Handbook::Data::TransportCatalogue::FindBus(std::string_view name) const
{
	const auto bus_it = buses_by_name_.find(name);
	return bus_it != buses_by_name_.end() ? bus_it->second : nullptr;
}
std::vector<Handbook::Data::BusPtr> Handbook::Data::TransportCatalogue::AllBuses()
{
	std::vector<BusPtr> result;
	for (const auto [_, ptr] : buses_by_name_)
	{
		result.push_back(ptr);
	}
	return result;
}
std::vector<Handbook::Data::StopPtr> Handbook::Data::TransportCatalogue::AllStops()
{
	std::vector<StopPtr> result;
	for (const auto [_, ptr] : stops_by_name_)
	{
		result.push_back(ptr);
	}
	return result;
}

std::pair<std::unordered_map<std::string, int>, std::string> Handbook::Data::TransportCatalogue::AllBayanedStops()
{
	std::unordered_map<std::string, int> result;
	for (auto& [pair, dist] : stop_distances_)
	{
		result[pair.first->name + bayan_bitch + pair.second->name] = dist;
	}
	return {result, bayan_bitch};
}
