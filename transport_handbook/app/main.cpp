#include "control_reader.h"
#include "transport_catalogue.h"
#include "view_data.h"
int main()
{
	int requests_count = Handbook::Control::ReadLineWithNumber();

	Handbook::Data::TransportCatalogue transport_catalogue;
	while (requests_count)
	{
		std::string str = Handbook::Control::ReadLine();
		auto req = Handbook::Control::ParseRequestString(str);
		AddRequestToCatalogue(req.get(), transport_catalogue);
		--requests_count;
	}
	int stat_count = Handbook::Control::ReadLineWithNumber();
	std::vector<std::string> out;
	while (stat_count)
	{
		std::string str = Handbook::Control::ReadLine();
		out.push_back(Handbook::Views::GetData(str, transport_catalogue));
		--stat_count;
	}
	for (const auto& s : out)
	{
		std::cout << s << std::endl;
	}
}
