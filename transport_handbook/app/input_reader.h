#pragma once

#include "geo.h"
#include <string>
#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>

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
    Bus() {}

    ~Bus() {}

    RequestType getRequestType() const override;

    void setRequestType(RequestType requestType) override;

    const std::string &getName() const override;

    void setName(const std::string &name) override;

    const std::vector<std::string> &getStops() const;

    void setStops(const std::vector<std::string> &stops);

private:
    RequestType type_;
    std::string name_;
    std::vector<std::string> stops_;
};

class Stop : public Request {
public:
    Stop() {}

    ~Stop() {}

    RequestType getRequestType() const override;

    void setRequestType(RequestType requestType) override;

    const std::string &getName() const override;

    void setName(const std::string &name) override;

    Coordinates coordinates;
private:

    RequestType type_;
    std::string name_;
};

Request *ParseRequestString(const std::string &r_str);

class Requests {
public:
    explicit Requests(size_t size) : requests(size) {

    }

    ~Requests();

    std::vector<Request *> requests;
};