//
// Created by fushenshen on 2020/1/11.
//

#include "HttpHeader.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <string>
#include <vector>

void HttpHeader::parse () {
    using namespace boost::algorithm;
    std::vector<std::string> headerVec;
    split_regex(headerVec, this->rawHeaders, boost::regex("\r\n"));

    std::string s1(*(headerVec.begin()));
    std::vector<std::string> methodVec;
    split(methodVec, s1, is_any_of(" "));
    assert(methodVec.size() == 3);
    this->method = methodVec[0];
    this->uri = methodVec[1];

    for (auto it = headerVec.begin() + 1; it != headerVec.end(); ++it) {
        std::vector<std::string> val;
        split_regex(val, *it, boost::regex(": "));
        if (val.size() <= 1) {
            continue;
        } else {
            this->headers.insert({to_lower_copy(val[0]), val[1]});
        }
    }
}

const std::string &HttpHeader::getMethod () const {
    return method;
}

const std::string &HttpHeader::getUri () const {
    return uri;
}

