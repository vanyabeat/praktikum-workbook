#pragma once

#include "geo.h"
#include "json.h"
#include "transport_catalogue.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <regex>
#include <string>
#include <vector>

namespace Handbook {
    namespace Control {
        enum RequestType {
            IsStop,
            IsBus
        };

        class Request {
        public:
            virtual RequestType getRequestType() const = 0;

            virtual void setRequestType(RequestType requestType) = 0;

            virtual const std::string &getName() const = 0;

            virtual void setName(const std::string &name) = 0;

            virtual ~Request() = default;
        };

        class Bus : public Request {
        public:
            Bus() {
            }

            ~Bus() {
            }

            RequestType getRequestType() const override;

            void setRequestType(RequestType requestType) override;

            const std::string &getName() const override;

            void setName(const std::string &name) override;

            const std::vector<std::string> &getStops() const;

            void setStops(const std::vector<std::string> &stops);

            bool getIsRoundtrip() const;

            void setIsRoundtrip(bool isRoundtrip);

        private:
            RequestType type_;
            std::string name_;
            std::vector<std::string> stops_;
            bool is_roundtrip_;
        };

        class Stop : public Request {
        public:
            Stop() {
            }

            ~Stop() {
            }

            RequestType getRequestType() const override;

            void setRequestType(RequestType requestType) override;

            const std::string &getName() const override;

            void setName(const std::string &name) override;

            Utilities::Coordinates coordinates;

            const std::vector<std::pair<std::string, size_t>> &getDistanceToOtherStop() const;

            void setDistanceToOtherStop(const std::vector<std::pair<std::string, size_t>> &distanceToOtherStop);

        private:
            std::vector<std::pair<std::string, size_t>> distance_to_other_stop;
            RequestType type_;
            std::string name_;
        };

        std::shared_ptr<Request> ParseRequestString(const std::string &r_str);

        std::shared_ptr<Request> ParseRequestDocument(const json::Document &doc);

        std::string ReadLine(std::istream &istream = std::cin);

        int ReadLineWithNumber(std::istream &istream = std::cin);

        void AddRequestToCatalogue(Request *request, Handbook::Data::TransportCatalogue &transport_catalogue);
    } // namespace Control
} // namespace Handbook