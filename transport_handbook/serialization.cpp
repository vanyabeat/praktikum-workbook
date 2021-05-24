#include "serialization.h"
#include <fstream>

Handbook::Control::Serializer::Serializer(std::istream &out, Handbook::Data::TransportCatalogue *tCPtr)
        : out_(out), t_c_ptr_(tCPtr), doc_({}) {
    doc_ = json::Load(out_);
    ouput_path_ = doc_.GetRoot().AsDict().at("serialization_settings").AsDict().at("file").AsString();
    render_settings_ = JsonDocToString_(json::Document(doc_.GetRoot().AsDict().at("render_settings").AsDict()));
    FillDataBase_();
    Serialize_();
}

void Handbook::Control::Serializer::FillDataBase_() {
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

void Handbook::Control::Serializer::Serialize_() {
    protodata::TransportCatalogue tc_proto;

    for (auto stop : t_c_ptr_->AllStops()) {
        auto stop_name = stop->name;
        protodata::Stop *tmp = tc_proto.add_stops();
        tmp->set_name(stop->name);
        tmp->set_lat(stop->coordinates.lat);
        tmp->set_lng(stop->coordinates.lng);
    }
    auto bayaned_stops = t_c_ptr_->AllBayanedStops();
    tc_proto.set_separator(bayaned_stops.second);
    for (const auto&[bayaned, dist] : bayaned_stops.first) {
        (*tc_proto.mutable_distances_between_stops())[bayaned] = dist;
    }
    for (auto bus : t_c_ptr_->AllBuses()) {
        protodata::Bus *tmp = tc_proto.add_buses();
        tmp->set_name(bus->name);
        tmp->set_is_roundtrip(bus->is_roundtrip);
        for (const auto stop : bus->stops) {
            tmp->add_stops(stop->name);
        }
    }

    protodata::RenderSettings *render_settings_tc_ = new protodata::RenderSettings;
    /*1*/
    render_settings_tc_->set_width(doc_.GetRoot().AsDict().at("render_settings").AsDict().at("width").AsDouble());
    /*2*/
    render_settings_tc_->set_height(doc_.GetRoot().AsDict().at("render_settings").AsDict().at("height").AsDouble());
    /*3*/
    render_settings_tc_->set_padding(
            doc_.GetRoot().AsDict().at("render_settings").AsDict().at("padding").AsDouble());
    /*4*/
    render_settings_tc_->set_stop_radius(
            doc_.GetRoot().AsDict().at("render_settings").AsDict().at("stop_radius").AsDouble());
    /*5*/
    render_settings_tc_->set_line_width(
            doc_.GetRoot().AsDict().at("render_settings").AsDict().at("line_width").AsDouble());
    /*6*/
    render_settings_tc_->set_stop_label_font_size(
            doc_.GetRoot().AsDict().at("render_settings").AsDict().at("stop_label_font_size").AsInt());
    { /*7*/
        for (const auto &item : doc_.GetRoot().AsDict().at("render_settings").AsDict().at(
                "stop_label_offset").AsArray()) {
            render_settings_tc_->add_stop_label_offset(item.AsDouble());
        }
    }
    /*8*/
    render_settings_tc_->set_underlayer_color(JsonDocToString_(
            json::Document(doc_.GetRoot().AsDict().at("render_settings").AsDict().at("underlayer_color"))));
    /*9*/
    render_settings_tc_->set_underlayer_width(
            doc_.GetRoot().AsDict().at("render_settings").AsDict().at("underlayer_width").AsDouble());

    /*10*/
    render_settings_tc_->set_color_palette(
            JsonDocToString_(json::Document(doc_.GetRoot().AsDict().at("render_settings").AsDict().at(
                    "color_palette"))));

    render_settings_tc_->set_bus_label_font_size(
            doc_.GetRoot().AsDict().at("render_settings").AsDict().at("bus_label_font_size").AsInt());
    /*12*/
    {
        for (const auto &item : doc_.GetRoot().AsDict().at("render_settings").AsDict().at(
                "bus_label_offset").AsArray()) {
            render_settings_tc_->add_bus_label_offset(item.AsDouble());
        }
    }


    /*9*/

    /*11*/


    { /*9*/
        for (const auto &item : doc_.GetRoot().AsDict().at("render_settings").AsDict().at(
                "bus_label_offset").AsArray()) {
            render_settings_tc_->add_bus_label_offset(item.AsDouble());
        }
    }
    tc_proto.set_allocated_render(render_settings_tc_);
    protodata::RoutingSettings *r_s = new protodata::RoutingSettings;
    r_s->set_wait_time(doc_.GetRoot().AsDict().at("routing_settings").AsDict().at("bus_wait_time").AsInt());
    r_s->set_velocity(doc_.GetRoot().AsDict().at("routing_settings").AsDict().at("bus_velocity").AsDouble());
    tc_proto.set_allocated_routing_settings(r_s);
    std::ofstream ofs(ouput_path_, std::ios_base::out | std::ios_base::binary);
    tc_proto.SerializeToOstream(&ofs);
}

std::string Handbook::Control::Serializer::JsonDocToString_(json::Document &&doc) {
    std::stringstream ss;
    json::Print(doc, ss);
    return ss.str();
}

Handbook::Control::Deserializer::Deserializer(std::istream &out, Handbook::Data::TransportCatalogue *tCPtr)
        : out_(out), t_c_ptr_(tCPtr), doc_({}) {
    doc_ = json::Load(out_);
    input_path = doc_.GetRoot().AsDict().at("serialization_settings").AsDict().at("file").AsString();
    std::ifstream ifs(input_path, std::ios_base::in | std::ios_base::binary);
    protodata::TransportCatalogue tc_proto;
    tc_proto.ParseFromIstream(&ifs);
    json::Array blo;
    for (double item : tc_proto.render().bus_label_offset()) {
        blo.push_back(item);
    }
    json::Array slo;
    for (double item : tc_proto.render().stop_label_offset()) {
        slo.push_back(item);
    }
    render_settings_ = json::Dict{
            {"width",                tc_proto.render().width()},
            {"height",               tc_proto.render().height()},
            {"padding",              tc_proto.render().padding()},
            {"stop_radius",          tc_proto.render().stop_radius()},
            {"line_width",           tc_proto.render().line_width()},
            {"stop_label_font_size", static_cast<int>(tc_proto.render().stop_label_font_size())},
            {"stop_label_offset",    slo},
            {"underlayer_color",     NodeFromString(tc_proto.render().underlayer_color())},
            {"underlayer_width",     tc_proto.render().underlayer_width()},
            {"color_palette",        NodeFromString(tc_proto.render().color_palette())},
            {"bus_label_font_size",  static_cast<int>(tc_proto.render().bus_label_font_size())},
            {"bus_label_offset",     blo}
    };
    routing_settings_.push_back(tc_proto.routing_settings().wait_time());
    routing_settings_.push_back(tc_proto.routing_settings().velocity());
    for (const auto &item : tc_proto.stops()) {
        t_c_ptr_->AddStop(item.name(), {.lat = item.lat(), .lng = item.lng()});
    }
    std::string separator = tc_proto.separator();
    for (const auto&[key, value] : tc_proto.distances_between_stops()) {
        // в баянах твоя сила и мудрость
        // да и списывать не повадно будет из моего гита)
        t_c_ptr_->AddStopsDistance(key.substr(0, key.find(separator)),
                                   key.substr(key.find(separator) + separator.size(), key.size()), value);
    }
    for (const auto &item : tc_proto.buses()) {
        std::vector<std::string> stops(item.stops().begin(), item.stops().end());
        t_c_ptr_->AddBus(item.name(), stops, item.is_roundtrip());
    }
}

void Handbook::Control::Deserializer::PrintReport() {
    using namespace std;
    json::Array result;
    auto needle = doc_.GetRoot().AsDict().find("stat_requests"s)->second.AsArray();
    //	bool settings = doc_.GetRoot().AsDict().find("render_settings") != doc_.GetRoot().AsDict().end();
    bool settings = !render_settings_.empty();
    bool routing_settings = !routing_settings_.empty();
    int busWaitTime = std::get<int>(routing_settings_[0]);
    double busVelocity = std::get<double>(routing_settings_[1]);;
//    if (routing_settings) {
//        busWaitTime = doc_.GetRoot().AsDict().at("routing_settings").AsDict().at("bus_wait_time").AsInt();
//        busVelocity = doc_.GetRoot().AsDict().at("routing_settings").AsDict().at("bus_velocity").AsDouble();
//    }
    std::unique_ptr<transport::RouteFinder> r_f =
            std::make_unique<transport::RouteFinder>(t_c_ptr_, busWaitTime, busVelocity);
    json::Node ren_set;
    for (const auto &item : needle) {
        if (settings && item.AsDict().at("type"s).AsString() == "Map"s) {

            result.push_back(std::move(
                    Handbook::Views::GetData(json::Document(json::Node{json::Dict{
                                                     {"type"s,            "Map"s},
                                                     {"id"s,              item.AsDict().at("id"s).AsInt()},
                                                     {"render_settings"s, json::Node(render_settings_)}}}),
                                             t_c_ptr_, nullptr)
                            .GetRoot()));
        } else if (routing_settings && item.AsDict().at("type").AsString() == "Route") {
            result.push_back(
                    std::move(Handbook::Views::GetData(
                            json::Document(json::Node{json::Dict{{"type"s, "Route"s},
                                                                 {"id"s,   item.AsDict().at("id"s).AsInt()},
                                                                 {"from"s, item.AsDict().at("from"s).AsString()},
                                                                 {"to"s,   item.AsDict().at("to"s).AsString()}}}),
                            t_c_ptr_, r_f.get())
                                      .GetRoot()));
        } else {
            result.push_back(std::move(Handbook::Views::GetData(json::Document(item), t_c_ptr_, nullptr).GetRoot()));
        }
    }
    json::Print(json::Document(result), std::cout);
}

json::Dict Handbook::Control::Deserializer::DictFromString(const std::string &str) {
    std::stringstream ss(str);
    auto doc = json::Load(ss);
    return doc.GetRoot().AsDict();
}

json::Node Handbook::Control::Deserializer::NodeFromString(const std::string &str) {
    std::stringstream ss(str);
    auto doc = json::Load(ss);
    return doc.GetRoot();
}
