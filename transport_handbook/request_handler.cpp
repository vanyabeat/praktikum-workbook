#include "request_handler.h"
#include "map_renderer.h"

#include <iomanip>
#include <numeric>
#include <sstream>

static svg::Color ParsingColor(const json::Node &color) {
    if (color.IsString()) {
        return color.AsString();
    }
    const auto &color_array = color.AsArray();
    if (color_array.size() == 3) {
        return svg::Rgb(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt());
    }

    return svg::Rgba(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt(), color_array[3].AsDouble());
}

static Handbook::Renderer::RenderSettings ReadRenderSettings(json::Dict data) {
    using namespace std;
    Handbook::Renderer::RenderSettings settings;
    settings.width = data["width"s].AsDouble();
    settings.height = data["height"s].AsDouble();
    settings.padding = data["padding"s].AsDouble();
    settings.line_width = data["line_width"s].AsDouble();
    settings.stop_radius = data["stop_radius"s].AsDouble();
    settings.bus_label_font_size = data["bus_label_font_size"s].AsDouble();
    settings.bus_label_offset = svg::Point(data["bus_label_offset"s].AsArray()[0].AsDouble(),
                                           data["bus_label_offset"s].AsArray()[1].AsDouble());
    settings.stop_label_font_size = data["stop_label_font_size"s].AsInt();
    settings.stop_label_offset = svg::Point(data["stop_label_offset"s].AsArray()[0].AsDouble(),
                                            data["stop_label_offset"s].AsArray()[1].AsDouble());
    settings.underlayer_color = ParsingColor(data["underlayer_color"s]);
    settings.underlayer_width = data["underlayer_width"s].AsDouble();
    for (const auto &color : data["color_palette"s].AsArray()) {
        settings.color_palette.push_back(ParsingColor(color));
    }
    return settings;
}

static json::Node makePathAnswer(int requestId, const std::vector<const transport::TripItem *> &data) {
    using namespace std;
    json::Builder builder;
    builder.StartArray();
    double totalTime = 0.0;
    for (const auto &item : data) {
        totalTime += (item->spending.wait_time + item->spending.trip_time);
        builder.StartDict()
                .Key("type"s)
                .Value("Wait"s)
                .Key("stop_name"s)
                .Value(item->from->name)
                .Key("time"s)
                .Value(item->spending.wait_time / 60)
                .EndDict()

                .StartDict()
                .Key("type"s)
                .Value("Bus"s)
                .Key("bus"s)
                .Value(item->bus->name)
                .Key("span_count"s)
                .Value(item->spending.stop_count)
                .Key("time"s)
                .Value(item->spending.trip_time / 60)
                .EndDict();
    }
    json::Node x = builder.EndArray().Build();

    return json::Builder{}
            .StartDict()
            .Key("request_id"s)
            .Value(requestId)
            .Key("total_time"s)
            .Value(totalTime / 60)
            .Key("items"s)
            .Value(x.AsArray())
            .EndDict()
            .Build();
}

json::Document Handbook::Views::GetData(const json::Document &stat, const Handbook::Data::TransportCatalogue *t_q,
                                        const transport::RouteFinder *r_f) {
    using namespace std;
    json::Node result;
    const auto &dict = stat.GetRoot().AsDict();
    int id = dict.at("id"s).AsInt();
    std::string type = dict.at("type"s).AsString();

    if (type == "Bus"s) {
        std::string name = dict.at("name"s).AsString();
        const Handbook::Data::Bus *bus = t_q->FindBus(name);
        if (bus != nullptr) {
            auto info = t_q->GetBusStat(bus);

            result = json::Builder{}
                    .StartDict()
                    .Key("curvature")
                    .Value(info.curvature)
                    .Key("request_id")
                    .Value(id)
                    .Key("route_length")
                    .Value(info.route_length)
                    .Key("stop_count")
                    .Value(info.stops_in_route)
                    .Key("unique_stop_count")
                    .Value(info.unique_stops)
                    .EndDict()
                    .Build()
                    .AsDict();

            return json::Document(result);
        }
    } else if (type == "Stop"s) {
        std::string name = dict.at("name"s).AsString();
        auto stop = t_q->FindStop(name);
        if (stop) {
            auto info = t_q->GetBusesOnStop(stop);
            if (!info->empty()) {
                std::vector<json::Node> buses;
                for (const auto &item : *info) {
                    buses.push_back(item->name);
                }
                std::sort(buses.begin(), buses.end(),
                          [](json::Node &l, json::Node &r) { return l.AsString() < r.AsString(); });

                result = json::Builder{}
                        .StartDict()
                        .Key("request_id")
                        .Value(id)
                        .Key("buses")
                        .Value(std::move(buses))
                        .EndDict()
                        .Build()
                        .AsDict();
                return json::Document(result);
            } else {
                result = json::Builder{}
                        .StartDict()
                        .Key("request_id")
                        .Value(id)
                        .Key("buses")
                        .StartArray()
                        .EndArray()
                        .EndDict()
                        .Build()
                        .AsDict();
                return json::Document(result);
            }
        }
    } else if (type == "Map"s) {

        Handbook::Renderer::RenderSettings renderSettings = ReadRenderSettings(dict.at("render_settings").AsDict());

        Handbook::Renderer::Map map_renderer(renderSettings);
        Handbook::Renderer::BusesByName buses_by_name;

        for (const auto &bus : t_q->GetBusesWithStops()) {
            buses_by_name.emplace(bus->name, bus);
        }

        stringstream out;

        map_renderer.Render(buses_by_name, out);

        json::Node res = json::Builder{}
                .StartDict()
                .Key("request_id")
                .Value(id)
                .Key("map")
                .Value(out.str())
                .EndDict()
                .Build()
                .AsDict();
        return json::Document(res);
    } else if (type == "Route" && r_f) {
        std::string f = dict.at("from"s).AsString();
        std::string t = dict.at("to"s).AsString();
        auto path_info = r_f->findRoute(f, t);
        if (path_info.has_value()) {
            return json::Document(makePathAnswer(id, path_info.value()));
        }
    }
    result = json::Dict{{"request_id"s,    id},
                        {"error_message"s, "not found"s}};
    return json::Document(result);
}
