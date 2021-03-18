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
/// правильей объявить полноценные структуры Stop и Bus, иначе что бы орентироваться в полях вам приходиться добавлять комментарии к индекса, т.е. уже видно что использовать не удобно
		using Stop = std::tuple<std::string, Utilities::Coordinates>;
		/*							0				1						2					3*/
/// в Bus сохраняется полноценная копия остановки, желательно хранить только указатель
		using Bus = std::tuple<std::string, std::vector<Stop>, int /*stops_on_route*/, int /*uniq_stops*/,
							   /*   4					5 					6*/
							   int /*length*/, double /*curvature*/, bool /*roundtrip*/>;
		class TransportCatalogue
		{
		  public:
			void AddStop(std::string stop_name, Utilities::Coordinates coordinates,
						 std::vector<std::pair<std::string, size_t>> vector_distances_to_other_stop = {});

			Stop GetStop(std::string stop_name) const
			{
				Utilities::Coordinates coordinates = stops_.at(stop_name);
				return std::make_tuple(std::move(stop_name), std::move(coordinates));
			}

			Bus GetBus(std::string bus_name) const
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

			std::vector<Stop*> GetStops() const
			{
				return re_stops_views_;
			}

			std::vector<Bus*> GetBuses() const
			{
				return re_buses_views_;
			}

			void Heat()
			{
				re_stops_ = std::move(GetStops_());
				re_buses_ = std::move(GetBuses_());
				re_buses_views_.reserve(re_buses_.size());
				re_stops_views_.reserve(re_stops_.size());
				for (int id = 0; id < re_buses_.size(); ++id)
				{
					re_buses_views_.push_back(&re_buses_[id]);
				}
				for (int id = 0; id < re_stops_.size(); ++id)
				{
					re_stops_views_.push_back(&re_stops_[id]);
				}
			}

			std::set<std::string> GetBusOfStop(std::string& stop) const
			{
				return stop_to_bus_.at(stop);
			}

			void AddBus(std::string bus_name, std::vector<std::string> stops, bool is_round_trip = false);

			double RoutePathSizeNaive(const std::vector<std::string>& stops) const;

			size_t RoutePathSize(const std::vector<std::string>& stops) const;

			std::optional<std::tuple<size_t, size_t, size_t, std::vector<std::string>, double>> GetRouteInfo(
				const std::string& bus) const;

			std::optional<std::set<std::string>> GetBusInfo(const std::string& stop) const;

			size_t GetDistanceBetweenStop(const std::string& stop_l, const std::string& stop_r) const;

		  private:
			std::vector<Stop> GetStops_() const
			{
				std::vector<Stop> res;
				for (const auto& i : stops_)
				{
					res.push_back({i.first, i.second});	/// по возможности пользуйтесь методом emplace_back он немножно эффективнее
				}
				return res;
			}

			std::vector<Bus> GetBuses_() const
			{
				std::vector<Bus> buses;
				for (const auto& [p, _] : bus_to_stops_)
				{
					buses.push_back(GetBus(p));
				}
				return buses;
			}
			void AddBus_(std::string bus_name, std::vector<std::string> stops);
			std::vector<Stop> re_stops_;
			std::vector<Bus> re_buses_;
			std::vector<Stop*> re_stops_views_;	/// название контейнера не травиться, обычно для указателей добавляется _ptr, лучше что то такое re_stop_ptrs (смысл re_ не знаю)
			std::vector<Bus*> re_buses_views_;	/// --//--
/// у вас очень много сохраняется копий названий остановок и автобусов
/// постарайтесь хранить названия (std::string) только в одном месте (в вашем случае re_stops_ и re_buses_), а в остальных контейрах либо указатели, либо string_view
			std::unordered_map<std::string, std::unordered_map<std::string, size_t>> distance_between_stops_;
			std::unordered_map<std::string, Utilities::Coordinates> stops_;
			std::unordered_map<std::string, std::vector<std::string>> bus_to_stops_;
			std::unordered_map<std::string, std::set<std::string>> stop_to_bus_;
			std::unordered_map<std::string, bool> bus_to_round_trip_;
		};
	} // namespace Data
} // namespace Handbook