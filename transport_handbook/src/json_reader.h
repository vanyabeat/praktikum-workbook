#pragma once
#include "control.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "view.h"

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
