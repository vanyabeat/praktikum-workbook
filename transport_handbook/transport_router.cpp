#include "transport_router.h"

#include <iostream>

namespace transport
{

	TripSpending operator+(const TripSpending& lhs, const TripSpending& rhs)
	{
		return {lhs.stop_count + rhs.stop_count, lhs.wait_time + rhs.wait_time, lhs.trip_time + rhs.trip_time};
	}

	bool operator<(const TripSpending& lhs, const TripSpending& rhs)
	{
		return (lhs.wait_time + lhs.trip_time < rhs.wait_time + rhs.trip_time);
	}

	bool operator>(const TripSpending& lhs, const TripSpending& rhs)
	{
		return (lhs.wait_time + lhs.trip_time > rhs.wait_time + rhs.trip_time);
	}

	std::ostream& operator<<(std::ostream& out, const TripItem& item)
	{
		using namespace std;
		out << item.from->name << " -> "s << item.to->name << " by bus "s << item.bus->name << " : "s
			<< item.spending.stop_count << " stops, "s << item.spending.wait_time / 60 << " min wait time "s
			<< item.spending.trip_time / 60 << " min trip time"s;
		return out;
	}

	DistanceFinder::DistanceFinder(Handbook::Data::TransportCatalogue* catalogue, Handbook::Data::BusPtr route)
		: direct_distances_(route->stops.size()), reverse_distances_(route->stops.size())
	{
		int directDistanceSum = 0;
		int reverseDistanceSum = 0;
		direct_distances_[0] = directDistanceSum;
		reverse_distances_[0] = reverseDistanceSum;
		for (size_t i = 1; i < route->stops.size(); ++i)
		{
			directDistanceSum += catalogue->FindStopsDistance(route->stops[i - 1], route->stops[i]);
			direct_distances_[i] = directDistanceSum;
			reverseDistanceSum += catalogue->FindStopsDistance(route->stops[i], route->stops[i - 1]);
			reverse_distances_[i] = reverseDistanceSum;
		}
	}

	int DistanceFinder::DistanceBetween(int from_stop_index, int to_stop_index)
	{
		if (from_stop_index < to_stop_index)
		{
			return direct_distances_[to_stop_index] - direct_distances_[from_stop_index];
		}
		else
		{
			return -(reverse_distances_[to_stop_index] - reverse_distances_[from_stop_index]);
		}
	}

	RouteFinder::RouteFinder(Handbook::Data::TransportCatalogue* catalogue, int bus_wait_time, double bus_velocity)
		: catalogue_(catalogue), bus_wait_time_(bus_wait_time * 60), bus_velocity_(bus_velocity / 3.6)
	{

		// Все остановки будут вершинами графа. Добавим их в словарь для быстрого поиска вершины по названию.
		auto allStops = catalogue_->AllStops();

		// Создаём сам граф с нужным количеством вершин
		graph_ = std::make_unique<NavigationGraph>(allStops.size());

		// Добавляем вершины
		graph::VertexId vertexCount = 0;
		for (auto stop : allStops)
		{
			stop_to_graph_vertex_.insert({stop, vertexCount++});
		}

		// Добавляем грани
		for (auto* route : catalogue->AllBuses())
		{
			DistanceFinder df(catalogue, route);
			const auto& stops = route->stops;
			for (size_t i = 0; i + 1 < stops.size(); ++i) /// UPD делал для того чтобы пользоваться std::abs
			{

				for (size_t j = i + 1; j < stops.size(); ++j)
				{
					AddTripItem(stops[i], stops[j], route,
								{static_cast<int>(j - i), static_cast<double>(bus_wait_time_),

								 df.DistanceBetween(i, j) / bus_velocity_});
					if (!route->is_roundtrip)
					{
						AddTripItem(stops[j], stops[i], route,
									{static_cast<int>(j - i), static_cast<double>(bus_wait_time_),
									 df.DistanceBetween(j, i) / bus_velocity_});
					}
				}
			}
		}
		router_ = std::make_unique<graph::Router<GraphWeight>>(*graph_);
	}

	std::optional<std::vector<const TripItem*>> RouteFinder::findRoute(std::string_view from, std::string_view to) const
	{
		Handbook::Data::StopPtr stopFrom = catalogue_->FindStop(from);
		Handbook::Data::StopPtr stopTo = catalogue_->FindStop(to);
		if (stopFrom == nullptr || stopTo == nullptr)
		{
			return std::nullopt;
		}

		std::vector<const TripItem*> result;
		if (stopFrom == stopTo)
		{
			return result;
		}

		graph::VertexId fromVertexId = stop_to_graph_vertex_.at(stopFrom);
		graph::VertexId toVertexId = stop_to_graph_vertex_.at(stopTo);
		auto route = router_->BuildRoute(fromVertexId, toVertexId);
		if (!route.has_value())
		{
			return std::nullopt;
		}

		for (const auto& edge : route.value().edges)
		{
			result.push_back(&graph_edges_.at(edge));
		}
		return result;
	}

	void RouteFinder::AddTripItem(Handbook::Data::StopPtr from, Handbook::Data::StopPtr to,
								  Handbook::Data::BusPtr route, TripSpending&& spending)
	{
		using namespace std;
		TripItem item{from, to, route, spending};
		int id = graph_->AddEdge(
			graph::Edge<GraphWeight>{stop_to_graph_vertex_[item.from], stop_to_graph_vertex_[item.to], item.spending});
		graph_edges_.push_back(std::move(item));
		if (id != static_cast<int>(graph_edges_.size() - 1))
		{
			throw std::runtime_error("AddEdge error "s + std::to_string(id) + "on graph edges size " +
									 std::to_string(graph_edges_.size() - 1));
		}
	}

} // namespace transport
