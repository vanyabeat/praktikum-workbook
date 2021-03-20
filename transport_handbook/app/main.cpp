//#include "control_reader.h"
//#include "transport_catalogue.h"
//#include "view_data.h"
// int main()
//{
//	int requests_count = Handbook::Control::ReadLineWithNumber();
//
//	Handbook::Data::TransportCatalogue transport_catalogue;
//	while (requests_count)
//	{
//		std::string str = Handbook::Control::ReadLine();
//		auto req = Handbook::Control::ParseRequestDocument(str);
//		AddRequestToCatalogue(req.get(), transport_catalogue);
//		--requests_count;
//	}
//	int stat_count = Handbook::Control::ReadLineWithNumber();
//	std::vector<std::string> out;
//	while (stat_count)
//	{
//		std::string str = Handbook::Control::ReadLine();
//		out.push_back(Handbook::Views::GetData(str, transport_catalogue));
//		--stat_count;
//	}
//	for (const auto& s : out)
//	{
//		std::cout << s << std::endl;
//	}
//}

#include "control.h"
#include "json_reader.h"
#include "request_handler.h"
#include "sstream"
#include "transport_catalogue.h"
int main()
{
	std::unique_ptr<Handbook::Data::TransportCatalogue> transport_catalogue = std::make_unique<Handbook::Data::TransportCatalogue>();
	Handbook::Control::JsonReader json_reader(std::cin, transport_catalogue.get());
	auto res = json_reader.GenerateReport();
	json::Print(res, std::cout);
}
