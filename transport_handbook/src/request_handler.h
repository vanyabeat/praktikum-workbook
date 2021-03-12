#pragma once
#include "transport_catalogue.h"
#include "json.h"
#include <string>

namespace Handbook
{
	namespace Views
	{
		std::string GetData(const std::string& stat, const Handbook::Data::TransportCatalogue& t_q);
		json::Document GetData(const json::Document &stat, const Handbook::Data::TransportCatalogue& t_q);
	} // namespace Views
} // namespace Handbook
