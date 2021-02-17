#include "input_reader.h"

std::string &ltrim(std::string &str) {
    auto it2 = std::find_if(str.begin(), str.end(),
                            [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
    str.erase(str.begin(), it2);
    return str;
}

std::string &rtrim(std::string &str) {
    auto it1 = std::find_if(str.rbegin(), str.rend(),
                            [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
    str.erase(it1.base(), str.end());
    return str;
}

std::string &trim(std::string &str) {
    return ltrim(rtrim(str));
}

std::vector<std::string> SplitIntoWords(const std::string &text) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == '>' || c == '-') {
            words.push_back(trim(word));
            word = "";
        } else {
            word += c;
        }
    }
    words.push_back(trim(word));

    return words;
}

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
    bool full_cycled = (r_str.find(">"s) != r_str.npos);
    std::vector<std::string> result;
    if (full_cycled) {
        result = SplitIntoWords(r_str);
    } else {
        auto tmp = SplitIntoWords(r_str);
        result.insert(result.end(), tmp.begin(), tmp.end());
        tmp.pop_back();
        std::reverse(tmp.begin(), tmp.end());
        result.insert(result.end(), tmp.begin(), tmp.end());
    }
    return result;
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
        static_cast<Bus *>(result)->setStops(FullPath(std::string(r_str.begin() + find_semicolon + 2, r_str.end())));
    }
    result->setName(std::move(name));
    return result;
}

std::string ReadLine() {
    std::string s;
    getline(std::cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    std::cin >> result;
    ReadLine();
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

const std::vector<std::string> &Bus::getStops() const {
    return stops_;
}

void Bus::setStops(const std::vector<std::string> &stops) {
    Bus::stops_ = stops;
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

Requests::~Requests() {
    for (auto r : requests) {
        delete r;
    }
}
