#pragma once
#include "input_reader.h"
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

class TransportCatalogue
{
  public:
	void AddRequest(Request* request)
	{
		switch (request->getRequestType())
		{
		case RequestType::IsBus: {
			Bus* bus = static_cast<Bus*>(request);
			bus_to_stops_[bus->getName()] = bus->getStops();
		}
		case RequestType::IsStop: {
		}
			Stop* stop = static_cast<Stop*>(request);
			stops_.insert({stop->getName(), stop->coordinates});
		}
	}

	std::vector<std::string> GetRoute(const std::string& bus)
	{
		return std::vector<std::string>();
	}

  private:
	std::unordered_set<std::pair<std::string, Coordinates>> stops_;
	std::unordered_map<std::string, std::vector<std::string>> bus_to_stops_;
};