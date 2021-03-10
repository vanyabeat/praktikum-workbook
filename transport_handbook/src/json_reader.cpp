#include "json_reader.h"

Handbook::Control::JsonReader::JsonReader(std::istream& out, Handbook::Data::TransportCatalogue& t_c)
	: out_(out), t_c_(t_c), doc_({})
{
	doc_ = json::Load(out);
	FillDataBase_();
}
json::Document Handbook::Control::JsonReader::GenerateReport()
{
	using namespace std;
	json::Array result;
	auto needle = doc_.GetRoot().AsMap().find("stat_requests"s)->second.AsArray();
	for (const auto& i : needle)
	{
		result.push_back(std::move(Handbook::Views::GetData(json::Document(i), t_c_).GetRoot()));
	}
	return json::Document(result);
}
void Handbook::Control::JsonReader::FillDataBase_()
{
	using namespace std;
	for (const auto& i : doc_.GetRoot().AsMap().find("base_requests"s)->second.AsArray())
	{
		Handbook::Control::AddRequestToCatalogue(
			Handbook::Control::ParseRequestDocument(json::Document(i)).get(), t_c_);
	}
}
