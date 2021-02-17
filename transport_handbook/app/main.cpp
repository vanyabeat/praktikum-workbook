#include "input_reader.h"

int main() {
    int requests_count = ReadLineWithNumber();
    auto requests = Requests(requests_count);
    while (requests_count) {
        std::string str = ReadLine();
        requests.requests.push_back(ParseRequestString(str));
        --requests_count;

    }

}

