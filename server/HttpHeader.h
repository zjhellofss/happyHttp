#include <utility>

//
// Created by fushenshen on 2020/1/11.
//

#ifndef HAPPYHTTP_HTTPHEADER_H
#define HAPPYHTTP_HTTPHEADER_H

#include <string>
#include <map>

class HttpHeader {

public:
    explicit HttpHeader (std::string rawHeaders) : rawHeaders(std::move(rawHeaders)) {
        this->parse();
    }

    std::string getValue (const std::string head) const {
        return this->headers.at(head);
    }

    const std::string &getMethod () const;

    const std::string &getUri () const;

private:
    void parse ();

private:
    std::map<std::string, std::string> headers;
    std::string method;
    std::string uri;
    std::string rawHeaders;
};

#endif //HAPPYHTTP_HTTPHEADER_H
