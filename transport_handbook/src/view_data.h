#pragma once
#include "transport_catalogue.h"
#include <string>

namespace Handbook
{
	namespace Views
	{
		std::string GetData(const std::string& stat, const Handbook::Data::TransportCatalogue& t_q);
	} // namespace Views
} // namespace Handbook
