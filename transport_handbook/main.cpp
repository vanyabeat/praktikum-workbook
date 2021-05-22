////#include "control_reader.h"
////#include "transport_catalogue.h"
////#include "view_data.h"
//// int main()
////{
////	int requests_count = Handbook::Control::ReadLineWithNumber();
////
////	Handbook::Data::TransportCatalogue transport_catalogue;
////	while (requests_count)
////	{
////		std::string str = Handbook::Control::ReadLine();
////		auto req = Handbook::Control::ParseRequestDocument(str);
////		AddRequestToCatalogue(req.get(), transport_catalogue);
////		--requests_count;
////	}
////	int stat_count = Handbook::Control::ReadLineWithNumber();
////	std::vector<std::string> out;
////	while (stat_count)
////	{
////		std::string str = Handbook::Control::ReadLine();
////		out.push_back(Handbook::Views::GetData(str, transport_catalogue));
////		--stat_count;
////	}
////	for (const auto& s : out)
////	{
////		std::cout << s << std::endl;
////	}
////}
//
//#include "domain.h"
//#include "json_reader.h"
//#include "request_handler.h"
//#include "sstream"
//#include "transport_catalogue.h"
// int main()
//{
//	std::unique_ptr<Handbook::Data::TransportCatalogue> transport_catalogue =
// std::make_unique<Handbook::Data::TransportCatalogue>(); 	Handbook::Control::JsonReader json_reader(std::cin,
// transport_catalogue.get()); 	auto res = json_reader.GenerateReport(); 	json::Print(res, std::cout);
//}

#include "domain.h"
#include "request_handler.h"
#include "serialization.h"
#include "sstream"
#include "transport_catalogue.h"
#include <fstream>
#include <iostream>
#include <string_view>
using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr)
{
	stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		PrintUsage();
		return 1;
	}

	const std::string_view mode(argv[1]);
	std::unique_ptr<Handbook::Data::TransportCatalogue> transport_catalogue =
		std::make_unique<Handbook::Data::TransportCatalogue>();
	if (mode == "make_base"sv)
	{
		Handbook::Control::Serializer serializer(std::cin, transport_catalogue.get());
	}
	else if (mode == "process_requests"sv)
	{
		Handbook::Control::Deserializer deserializer(std::cin, transport_catalogue.get());
		deserializer.PrintReport();
	}
	else
	{
		PrintUsage();
		return 1;
	}
}