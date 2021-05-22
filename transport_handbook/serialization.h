#pragma once
#include "domain.h"
#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"

namespace Handbook
{
	namespace Control
	{
		class Serializer
		{
		  public:
			Serializer(std::istream& out, Data::TransportCatalogue* tCPtr);

		  private:
			std::istream& out_;
			Handbook::Data::TransportCatalogue* t_c_ptr_; /// приватное поле, должно быть с подчеркиванием
			json::Document doc_;
			std::string ouput_path_;
			void FillDataBase_();
			void Serialize_();
		};

		class Deserializer
		{
		  public:
			Deserializer(std::istream& out, Data::TransportCatalogue* tCPtr);
			void PrintReport();
		  private:
			std::istream& out_;
			Handbook::Data::TransportCatalogue* t_c_ptr_; /// приватное поле, должно быть с подчеркиванием
			json::Document doc_;
			std::string input_path;

		};
	} // namespace Control

} // namespace Handbook
