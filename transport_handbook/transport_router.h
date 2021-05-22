#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace transport
{

	struct TripSpending
	{
		TripSpending() = default;
		TripSpending(int stop_count, double wait_time, double trip_time)
			: stop_count(stop_count), wait_time(wait_time), trip_time(trip_time)
		{
		}
		int stop_count = 0;
		double wait_time = 0;
		double trip_time = 0;
	};

	TripSpending operator+(const TripSpending& lhs, const TripSpending& rhs);
	bool operator<(const TripSpending& lhs, const TripSpending& rhs);
	bool operator>(const TripSpending& lhs, const TripSpending& rhs);

	struct TripItem
	{
		const Handbook::Data::StopPtr from = nullptr;
		const Handbook::Data::StopPtr to = nullptr;
		const Handbook::Data::BusPtr bus = nullptr;
		TripSpending spending = {};
	};

	std::ostream& operator<<(std::ostream& out, const TripItem& item);

	// Вспомогательный класс для вычисления расстояния между всеми остановками
	// за линейное время
	class DistanceFinder
	{
	  public:
		DistanceFinder(Handbook::Data::TransportCatalogue* catalogue, Handbook::Data::BusPtr route);
		int DistanceBetween(int from_stop_index, int to_stop_index);

	  private:
		std::vector<int> direct_distances_;
		std::vector<int> reverse_distances_;
	};

	class RouteFinder
	{
		using GraphWeight = TripSpending;
		using NavigationGraph = graph::DirectedWeightedGraph<GraphWeight>;

	  public:
		RouteFinder(Handbook::Data::TransportCatalogue* catalogue, int bus_wait_time, double bus_velocity);
		std::optional<std::vector<const TripItem*>> findRoute(std::string_view from, std::string_view to) const;

	  private:
		void AddTripItem(Handbook::Data::StopPtr from, Handbook::Data::StopPtr to, Handbook::Data::BusPtr route,
						 TripSpending&& spending);

	  private:
		Handbook::Data::TransportCatalogue* catalogue_;
		std::unique_ptr<graph::Router<GraphWeight>> router_;
		std::unique_ptr<NavigationGraph> graph_;
		std::unordered_map<Handbook::Data::StopPtr, graph::VertexId> stop_to_graph_vertex_;
		std::vector<TripItem> graph_edges_;
		int bus_wait_time_ = 0;
		double bus_velocity_ = 0;
	};

} // namespace transport
