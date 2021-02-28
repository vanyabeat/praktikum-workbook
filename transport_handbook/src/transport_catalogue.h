#pragma once
#include "geo.h"
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Handbook
{
	namespace Data
	{
		class TransportCatalogue
		{
		  public:
			void AddStop(std::string stop_name, Utilities::Coordinates coordinates,
						 std::vector<std::pair<std::string, size_t>> vector_distances_to_other_stop = {});

			void AddBus(std::string bus_name, std::vector<std::string> stops);

			double RoutePathSizeNaive(const std::vector<std::string>& stops) const;

			size_t RoutePathSize(const std::vector<std::string>& stops) const;

			std::optional<std::tuple<size_t, size_t, size_t, std::vector<std::string>, double>> GetRouteInfo(
				const std::string& bus) const;

			std::optional<std::set<std::string>> GetBusInfo(const std::string& stop) const;

			size_t GetDistanceBetweenStop(const std::string& stop_l, const std::string& stop_r) const;

		  private:
			std::unordered_map<std::string, std::unordered_map<std::string, size_t>> distance_between_stops_;
			std::unordered_map<std::string, Utilities::Coordinates> stops_;
			std::unordered_map<std::string, std::vector<std::string>> bus_to_stops_;
			std::unordered_map<std::string, std::set<std::string>> stop_to_bus_;
		};
	} // namespace Data
} // namespace Handbook