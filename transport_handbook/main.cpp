#include <fstream>
#include <iostream>
#include <string_view>
#include "domain.h"
#include "serialization.h"
#include "request_handler.h"
#include "sstream"
#include "transport_catalogue.h"

using namespace std::literals;

void PrintUsage(std::ostream &stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        std::unique_ptr<Handbook::Data::TransportCatalogue> transport_catalogue =
                std::make_unique<Handbook::Data::TransportCatalogue>();
        Handbook::Control::Serializer serializer(std::cin, transport_catalogue.get());
    } else if (mode == "process_requests"sv) {
        std::unique_ptr<Handbook::Data::TransportCatalogue> transport_catalogue =
                std::make_unique<Handbook::Data::TransportCatalogue>();
        Handbook::Control::Deserializer deserializer(std::cin, transport_catalogue.get());
        deserializer.PrintReport();
    } else {
        PrintUsage();
        return 1;
    }
}