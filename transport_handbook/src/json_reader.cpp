#include "json_reader.h"

Handbook::Control::JsonReader::JsonReader(std::istream& out, Handbook::Data::TransportCatalogue* t_c)
	: out_(out), t_c_ptr(t_c), doc_({})
{
	doc_ = json::Load(out);
	FillDataBase_();
}
json::Document Handbook::Control::JsonReader::GenerateReport()
{
	using namespace std;
	json::Array result;
	auto needle = doc_.GetRoot().AsMap().find("stat_requests"s)->second.AsArray();
	bool settings = doc_.GetRoot().AsMap().find("render_settings") != doc_.GetRoot().AsMap().end();
	json::Node ren_set;
	for (const auto& i : needle)
	{
		if (settings && i.AsMap().at("type"s).AsString() == "Map"s)
		{
			result.push_back(std::move(
				Handbook::Views::GetData(json::Document(json::Node{json::Dict{
											 {"type"s, "Map"s},
											 {"id"s, i.AsMap().at("id"s).AsInt()},
											 {"render_settings"s, doc_.GetRoot().AsMap().at("render_settings"s)}}}),
										 t_c_ptr)
					.GetRoot()));
		}
		else
		{
			result.push_back(std::move(Handbook::Views::GetData(json::Document(i), t_c_ptr).GetRoot()));
		}
	}
	return json::Document(result);
}
void Handbook::Control::JsonReader::FillDataBase_()
{
	using namespace std;
	std::vector<std::tuple<std::string_view, std::string_view, int>> buffer_stops;
	std::vector<std::shared_ptr<Handbook::Control::Request>> requests_;
	Handbook::Data::TransportCatalogue* ctx = t_c_ptr;
	for (const auto& i : doc_.GetRoot().AsMap().find("base_requests"s)->second.AsArray())
	{
		requests_.push_back(Handbook::Control::ParseRequestDocument(json::Document(i)));
	}

	// сначала добавим все остановки и буфернем их
	std::for_each(requests_.begin(), requests_.end(),
				  [&buffer_stops, ctx](std::shared_ptr<Handbook::Control::Request>& req) {
					  if (req.get()->getRequestType() == Handbook::Control::RequestType::IsStop)
					  {
						  auto stop = static_cast<Handbook::Control::Stop*>(req.get());
						  ctx->AddStop(stop->getName(), stop->coordinates);
						  for (const auto& item : stop->getDistanceToOtherStop())
						  {
							  buffer_stops.emplace_back(stop->getName(), item.first, item.second);
						  }
					  }
				  });
	// обрабатываем сами расстояния
	std::for_each(buffer_stops.begin(), buffer_stops.end(), [&ctx](auto& item) {
		ctx->AddStopsDistance(std::get<0>(item), std::get<1>(item), std::get<2>(item));
	});

	std::for_each(requests_.begin(), requests_.end(), [&ctx](std::shared_ptr<Handbook::Control::Request>& req) {
		if (req.get()->getRequestType() == Handbook::Control::RequestType::IsBus)
		{
			auto bus = static_cast<Handbook::Control::Bus*>(req.get());
			ctx->AddBus(bus->getName(), bus->getStops(), bus->getIsRoundtrip());
		}
	});
}
