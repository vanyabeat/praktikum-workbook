#pragma once
#include "geo.h"
#include "input_reader.h"
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

class TransportCatalogue
{
  public:
	void AddRequest(Request* request);

	void AddStop(std::string stop_name, Coordinates coordinates,
				 std::vector<std::pair<std::string, size_t>> vector_distances_to_other_stop = {});

	double RoutePathSizeNaive(const std::vector<std::string>& stops) const;

	size_t RoutePathSize(const std::vector<std::string>& stops) const;

	std::optional<std::tuple<size_t, size_t, size_t, std::vector<std::string>, double>> GetRouteInfo(
		const std::string& bus) const;

	std::optional<std::set<std::string>> GetBusInfo(const std::string& stop) const;

	size_t GetDistanceBetweenStop(const std::string& stop_l, const std::string& stop_r) const;

  private:
	std::unordered_map<std::string, std::unordered_map<std::string, size_t>> distance_between_stops_;
	std::unordered_map<std::string, Coordinates> stops_;
	std::unordered_map<std::string, std::vector<std::string>> bus_to_stops_;
	std::unordered_map<std::string, std::set<std::string>> stop_to_bus_;
};