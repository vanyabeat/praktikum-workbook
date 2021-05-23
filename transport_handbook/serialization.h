#pragma once
#include "domain.h"
#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"
#include "svg.pb.h"
#include "map_renderer.pb.h"
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
			std::string render_settings_;
			void FillDataBase_();
			void Serialize_();
			// вообще надо к такой фигне zlib )))
			std::string JsonDocToString_(json::Document&& doc);
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
			json::Dict render_settings_;
			std::string input_path;
			json::Dict DictFromString(const std::string &str);
		};
	} // namespace Control

} // namespace Handbook
