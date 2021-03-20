#pragma once
#include "domain.h"
#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

namespace Handbook
{
	namespace Control
	{
/// лишний комментарий
		/*
		 * {
  "base_requests": [ ... ],
  "stat_requests": [ ... ]
}
		 * */

		class JsonReader
		{
		  public:
			JsonReader(std::istream& out, Handbook::Data::TransportCatalogue& t_c);

			json::Document GenerateReport();

		  private:
			void FillDataBase_();


			std::istream& out_;
			Handbook::Data::TransportCatalogue& t_c_;
			json::Document doc_;
		};
	} // namespace Control
} // namespace Handbook
