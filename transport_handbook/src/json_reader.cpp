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
										 t_c_)
					.GetRoot()));
		}
		else
		{
			result.push_back(std::move(Handbook::Views::GetData(json::Document(i), t_c_).GetRoot()));
		}
	}
	return json::Document(result);
}
void Handbook::Control::JsonReader::FillDataBase_()
{
	using namespace std;
	for (const auto& i : doc_.GetRoot().AsMap().find("base_requests"s)->second.AsArray())
	{
		Handbook::Control::AddRequestToCatalogue(Handbook::Control::ParseRequestDocument(json::Document(i)).get(),
												 t_c_);
	}
	t_c_.Heat();
}

// void Handbook::Control::JsonReader::FillRenderSettings()
//{
//	using namespace std;
//	if (doc_.GetRoot().AsMap().find("render_settings") != doc_.GetRoot().AsMap().end())
//	{
//		auto data = doc_.GetRoot().AsMap().at("render_settings").AsMap();
//		renderSettings_.width = data.at("width"s).AsDouble();
//		renderSettings_.height = data.at("height"s).AsDouble();
//		renderSettings_.padding = data.at("padding"s).AsDouble();
//		renderSettings_.stop_radius = data.at("stop_radius"s).AsDouble();
//		renderSettings_.line_width = data.at("stop_radius"s).AsDouble();
//		renderSettings_.bus_label_font_size = data.at("label_font_size"s).AsDouble();
//		renderSettings_.bus_label_offset = svg::Point(data["bus_label_offset"].AsArray()[0].AsDouble(),
//													  data["bus_label_offset"].AsArray()[1].AsDouble());
//		renderSettings_.stop_label_font_size = data["stop_label_font_size"].AsInt();
//		renderSettings_.stop_label_offset = svg::Point(data["stop_label_offset"].AsArray()[0].AsDouble(),
//													   data["stop_label_offset"].AsArray()[1].AsDouble());
//		renderSettings_.underlayer_color = ParsingColor(data["underlayer_color"]);
//		renderSettings_.underlayer_width = data["underlayer_width"].AsDouble();
//		for (auto& color : data["color_palette"].AsArray())
//		{
//			renderSettings_.color_palette.push_back(ParsingColor(color));
//		}
//	}
//}
