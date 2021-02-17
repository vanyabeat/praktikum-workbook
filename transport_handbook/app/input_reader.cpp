#include "input_reader.h"


Coordinates ParseCoordsSubstring(const std::string &r_str) {
    //latitude, longitude
    using namespace std;
    size_t comma = r_str.find(","s);
    Coordinates result{std::stod(std::string(r_str.begin(), r_str.begin() + comma)),
                       std::stod(std::string(r_str.begin() + comma + 2, r_str.end()))};
    return result;
}

std::vector<std::string> FullPath(const std::string &r_str) {
    using namespace std;
    size_t comma = r_str.find(">"s);
}

Request *ParseRequestString(const std::string &r_str) {
    using namespace std;
    // я пока не осилил смарт поинтеры и наверное паттерн абстрактная фабрика ?
    Request *result;
    // если это не Stop, то это Bus )
    size_t found_stop = r_str.find("Stop "s);
    size_t find_semicolon;
    std::string name;
    if (found_stop != r_str.npos) {
        result = new Stop();
        result->setRequestType(RequestType::IsStop);
        find_semicolon = r_str.find(":"s);
        name = std::string(r_str.begin() + found_stop + "Stop "s.size(), r_str.begin() + find_semicolon);
        std::string coords_str(r_str.begin() + find_semicolon + 2, r_str.end());
        auto coords = ParseCoordsSubstring(coords_str);
        static_cast<Stop *>(result)->coordinates = coords;
    } else {
        result = new Bus();
        find_semicolon = r_str.find(":"s);
        name = std::string(r_str.begin() + "Bus "s.size(), r_str.begin() + find_semicolon);
        result->setRequestType(RequestType::IsBus);
        std::string stops = std::string(r_str.begin() + find_semicolon + 2, r_str.end());
        int a = 6;
    }
    result->setName(std::move(name));
    return result;
}

RequestType Bus::getRequestType() const {
    return type_;
}

void Bus::setRequestType(RequestType requestType) {
    type_ = requestType;
}

const std::string &Bus::getName() const {
    return name_;
}

void Bus::setName(const std::string &name) {
    name_ = name;
}

RequestType Stop::getRequestType() const {
    return type_;
}

void Stop::setRequestType(RequestType requestType) {
    type_ = requestType;
}

const std::string &Stop::getName() const {
    return name_;
}

void Stop::setName(const std::string &name) {
    name_ = name;
}
