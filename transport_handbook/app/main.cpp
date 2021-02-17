#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"
int main()
{
	int requests_count = ReadLineWithNumber();
	auto requests = Requests(requests_count);
	TransportCatalogue transport_catalogue;
	while (requests_count)
	{
		std::string str = ReadLine();
		Request* req = ParseRequestString(str);
		requests.requests.push_back(req);
		transport_catalogue.AddRequest(req);
		--requests_count;
	}

	int stat_count = ReadLineWithNumber();
	std::vector<std::string> out;
	while (stat_count)
	{
		std::string str = ReadLine();
		out.push_back(ReadStat(str, transport_catalogue));
		--stat_count;
	}
	for (const auto& s : out)
	{
		std::cout << s << std::endl;
	}
}
