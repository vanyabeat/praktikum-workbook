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

	double RoutePathSize(const std::vector<std::string>& stops) const;

	std::optional<std::tuple<size_t, size_t, double, std::vector<std::string>>> GetRouteInfo(
		const std::string& bus) const;

	std::optional<std::set<std::string>> GetBusInfo(const std::string& stop) const;

  private:
	std::unordered_map<std::string, std::unordered_map<std::string, double>> distance_between_stops_;
	std::unordered_map<std::string, Coordinates> stops_;
	std::unordered_map<std::string, std::vector<std::string>> bus_to_stops_;
	std::unordered_map<std::string, std::set<std::string>> stop_to_bus_;
};