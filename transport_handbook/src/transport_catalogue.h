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
		using Stop = std::tuple<std::string, Utilities::Coordinates>;
		using Bus = std::tuple<std::string, std::vector<Stop>, int /*stops_on_route*/, int /*uniq_stops*/,
							   int /*length*/, double /*curvature*/, bool /*roundtrip*/>;
		class TransportCatalogue
		{
		  public:
			void AddStop(std::string stop_name, Utilities::Coordinates coordinates,
						 std::vector<std::pair<std::string, size_t>> vector_distances_to_other_stop = {});

			Stop GetStop(std::string stop_name)
			{
				Utilities::Coordinates coordinates = stops_.at(stop_name);
				return std::make_tuple(std::move(stop_name), std::move(coordinates));
			}

			Bus GetBus(std::string bus_name)
			{
				bool is_round_trip = bus_to_round_trip_.at(bus_name);
				auto info = GetRouteInfo(bus_name);
				std::vector<Stop> stop_res;

				for (const auto& stop : std::get<3>(info.value()))
				{
					stop_res.push_back(GetStop(stop));
				}

				return std::make_tuple(std::move(bus_name), std::move(stop_res), std::move(std::get<0>(info.value())),
									   std::move(std::get<1>(info.value())),
									   static_cast<int>(std::get<2>(info.value())), std::get<4>(info.value()),
									   is_round_trip);
			}

			void AddBus(std::string bus_name, std::vector<std::string> stops, bool is_round_trip = false)
			{
				AddBus_(bus_name, stops);
				bus_to_round_trip_[bus_name] = is_round_trip;
			}

			double RoutePathSizeNaive(const std::vector<std::string>& stops) const;

			size_t RoutePathSize(const std::vector<std::string>& stops) const;

			std::optional<std::tuple<size_t, size_t, size_t, std::vector<std::string>, double>> GetRouteInfo(
				const std::string& bus) const;

			std::optional<std::set<std::string>> GetBusInfo(const std::string& stop) const;

			size_t GetDistanceBetweenStop(const std::string& stop_l, const std::string& stop_r) const;

		  private:
			void AddBus_(std::string bus_name, std::vector<std::string> stops);
			std::unordered_map<std::string, std::unordered_map<std::string, size_t>> distance_between_stops_;
			std::unordered_map<std::string, Utilities::Coordinates> stops_;
			std::unordered_map<std::string, std::vector<std::string>> bus_to_stops_;
			std::unordered_map<std::string, std::set<std::string>> stop_to_bus_;
			std::unordered_map<std::string, bool> bus_to_round_trip_;
		};
	} // namespace Data
} // namespace Handbook