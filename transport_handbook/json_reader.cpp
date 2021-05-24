#include "json_reader.h"

Handbook::Control::JsonReader::JsonReader(std::istream &out, Handbook::Data::TransportCatalogue *t_c)
        : out_(out), t_c_ptr_(t_c), doc_({}) {
    doc_ = json::Load(out_);
    FillDataBase_();
}

json::Document Handbook::Control::JsonReader::GenerateReport() {
    using namespace std;
    json::Array result;
    auto needle = doc_.GetRoot().AsDict().find("stat_requests"s)->second.AsArray();
    bool settings = doc_.GetRoot().AsDict().find("render_settings") != doc_.GetRoot().AsDict().end();
    bool routing_settings = doc_.GetRoot().AsDict().find("routing_settings") != doc_.GetRoot().AsDict().end();
    int busWaitTime = 0;
    double busVelocity = 0.0;
    if (routing_settings) {
        busWaitTime = doc_.GetRoot().AsDict().at("routing_settings").AsDict().at("bus_wait_time").AsInt();
        busVelocity = doc_.GetRoot().AsDict().at("routing_settings").AsDict().at("bus_velocity").AsDouble();
    }
    std::unique_ptr<transport::RouteFinder> r_f =
            std::make_unique<transport::RouteFinder>(t_c_ptr_, busWaitTime, busVelocity);
    json::Node ren_set;
    for (const auto &item : needle) {
        if (settings && item.AsDict().at("type"s).AsString() == "Map"s) {
            result.push_back(std::move(
                    Handbook::Views::GetData(json::Document(json::Node{json::Dict{
                                                     {"type"s,            "Map"s},
                                                     {"id"s,              item.AsDict().at("id"s).AsInt()},
                                                     {"render_settings"s, doc_.GetRoot().AsDict().at("render_settings"s)}}}),
                                             t_c_ptr_, nullptr)
                            .GetRoot()));
        } else if (routing_settings && item.AsDict().at("type").AsString() == "Route") {
            result.push_back(std::move(
                    Handbook::Views::GetData(json::Document(json::Node{json::Dict{
                                                     {"type"s, "Route"s},
                                                     {"id"s,   item.AsDict().at("id"s).AsInt()},
                                                     {"from"s, item.AsDict().at("from"s).AsString()},
                                                     {"to"s,   item.AsDict().at("to"s).AsString()}
                                             }}),
                                             t_c_ptr_, r_f.get())
                            .GetRoot()));
        } else {
            result.push_back(std::move(Handbook::Views::GetData(json::Document(item), t_c_ptr_, nullptr).GetRoot()));
        }
    }
    return json::Document(result);
}

void Handbook::Control::JsonReader::FillDataBase_() {
    using namespace std;
    std::vector<std::tuple<std::string_view, std::string_view, int>> buffer_stops;
    std::vector<std::shared_ptr<Handbook::Control::Request>> requests;

    for (const auto &i : doc_.GetRoot().AsDict().find("base_requests"s)->second.AsArray()) {
        requests.push_back(Handbook::Control::ParseRequestDocument(json::Document(i)));
    }

    // сначала добавим все остановки и буфернем их
    std::for_each(requests.begin(), requests.end(),
                  [&buffer_stops, this](std::shared_ptr<Handbook::Control::Request> &req) {
                      if (req.get()->getRequestType() == Handbook::Control::RequestType::IsStop) {
                          auto stop = static_cast<Handbook::Control::Stop *>(req.get());
                          t_c_ptr_->AddStop(stop->getName(), stop->coordinates);
                          for (const auto &item : stop->getDistanceToOtherStop()) {
                              buffer_stops.emplace_back(stop->getName(), item.first, item.second);
                          }
                      }
                  });
    // обрабатываем сами расстояния
    std::for_each(buffer_stops.begin(), buffer_stops.end(), [this](auto item) {
        t_c_ptr_->AddStopsDistance(std::get<0>(item), std::get<1>(item), std::get<2>(item));
    });

    std::for_each(requests.begin(), requests.end(), [this](std::shared_ptr<Handbook::Control::Request> &req) {
        if (req.get()->getRequestType() == Handbook::Control::RequestType::IsBus) {
            auto bus = static_cast<Handbook::Control::Bus *>(req.get());
            t_c_ptr_->AddBus(bus->getName(), bus->getStops(), bus->getIsRoundtrip());
        }
    });
}
