#pragma once

#include "geo.h"
#include <cmath>
#include <deque>
#include <numeric>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Handbook
{
	namespace Data
	{
		template <typename T> using PairPtrs = std::pair<const T*, const T*>;

		template <typename T> struct PairPtrHasher
		{
			uint64_t operator()(const PairPtrs<T>& stops_ptr) const
			{
				size_t ptrValue1 = reinterpret_cast<size_t>(stops_ptr.first);
				size_t ptrValue2 = reinterpret_cast<size_t>(stops_ptr.second);
				double res_hash = ptrValue1 + ptrValue2 * 47;
				return static_cast<uint64_t>(std::sqrt(res_hash));
			}
		};

		struct Stop
		{
			std::string name;
			Utilities::Coordinates coordinates;
		};

		using StopPtr = const Stop*;

		struct Bus
		{
			std::string name;
			bool is_roundtrip = false;
			std::vector<StopPtr> stops;

			double GetCoordinateLength() const;
		};

		using BusPtr = const Bus*;

		struct BusStat
		{
			int stops_in_route = 0;
			int unique_stops = 0;
			double route_length = 0.0;
			double curvature = 0.0;
		};

		class TransportCatalogue
		{
		  public:
			void AddStop(std::string_view name, Utilities::Coordinates coordinates);

			StopPtr FindStop(std::string_view name) const;

			const std::unordered_set<BusPtr>* GetBusesOnStop(StopPtr stop) const;

			std::unordered_set<BusPtr> GetBusesWithStops() const;

			void AddStopsDistance(std::string_view from_stop, std::string_view to_stop, int distance);

			int FindStopsDistance(StopPtr from_stop_ptr, StopPtr to_stop_ptr) const;

			void AddBus(std::string_view name, const std::vector<std::string>& bus_stops, bool is_roundtrip);

			BusPtr FindBus(std::string_view name) const; /// желательно поменять тип результата на BusPtr

			BusStat GetBusStat(const Bus* bus) const;

			std::vector<BusPtr> AllBuses();

			std::vector<StopPtr> AllStops();

			std::pair<std::unordered_map<std::string, int>, std::string> AllBayanedStops();

		  private:
			std::deque<Bus> buses_;
			std::deque<Stop> stops_;

			std::unordered_map<std::string_view, BusPtr> buses_by_name_;
			std::unordered_map<std::string_view, StopPtr> stops_by_name_;
			std::unordered_map<StopPtr, std::unordered_set<BusPtr>> buses_by_stop_;
			std::unordered_map<PairPtrs<Stop>, int, PairPtrHasher<Stop>> stop_distances_;
			const std::string bayan = "[:|||:]"; /// оставлю здесь, так удобнее менять сепараторы...
/// не соглашусь с аргументом, класс долже бать споректирован, как другим удобно использовать, а это поле получается бессмысленным
/// если предполагается менять сепараторы, то должно быть для этого api
		};
	} // namespace Data
} // namespace Handbook