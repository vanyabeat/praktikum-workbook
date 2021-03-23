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
		class JsonReader
		{
		  public:
			JsonReader(std::istream& out, Handbook::Data::TransportCatalogue* t_c_ptr);

			json::Document GenerateReport();

		  private:
			void FillDataBase_();

			std::istream& out_;
			Handbook::Data::TransportCatalogue* t_c_ptr;
			json::Document doc_;
		};
	} // namespace Control
} // namespace Handbook
