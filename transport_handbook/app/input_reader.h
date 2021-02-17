#pragma once

#include "geo.h"
#include <string>
#include <vector>
#include <iostream>

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

private:
    RequestType type_;
    std::string name_;
    std::vector<std::string> stops;
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

