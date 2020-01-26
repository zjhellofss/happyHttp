// Created by fushenshen on 2020/1/11.
//

#include "HttpHeader.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <string>
#include <vector>
#include <iostream>

void HttpHeader::parse() {
    using namespace boost::algorithm;
    std::vector<std::string> headerVec;
    split_regex(headerVec, this->rawHeaders, boost::regex("\r\n"));

    std::string s1(*(headerVec.begin()));
    std::vector<std::string> methodVec;
    std::string::iterator iter = s1.begin();
    std::string::iterator preIter = iter;
    bool hasParam = false;

    for (; iter != s1.end(); ++iter) {
        char c = *iter;
        if (c == ' ') {
            methodVec.push_back(std::move(std::string(preIter, iter)));
            preIter = iter + 1;
        } else if (c == '&' || c == '?') {
            methodVec.push_back(std::move(std::string(preIter, iter)));
            ++iter;
            preIter = iter;
            std::string paramsName;
            std::string paramsValue;
            while (*iter != ' ') {
                while (*iter != ' ') {
                    //parse name
                    if (*iter == '=') {
                        paramsName = std::move(std::string(preIter, iter));
                        ++iter;
                        preIter = iter;
                        break;
                    }
                    ++iter;
                }
                while (*iter != ' ') {
                    //parse value
                    if (*iter == '&') {
                        paramsValue = std::move(std::string(preIter, iter));
                        ++iter;
                        preIter = iter;
                        break;
                    }
                    ++iter;
                    if (*iter == ' ') {
                        break;
                    }
                }
                if (*iter == ' ') {
                    paramsValue = std::move(std::string(preIter, iter));
                }
                this->params.insert({paramsName, paramsValue});
            }

        }
    }
    assert(methodVec.size() == 2);
    this->method = methodVec[0];
    this->uri = methodVec[1];


    for (auto it = headerVec.begin() + 1; it != headerVec.end(); ++it) {
        //解析键值对
        const std::string &kv = *it;
        std::string name;
        std::string value;
        auto i = kv.begin();
        //parse name
        for (; i != kv.end(); ++i) {
            char c = *i;
            if (c == ':') {
                name = std::string(kv.begin(), i);
                ++i;
                assert(*i == ' ');
                break;
            }
        }
        //pase value
        if (i + 1 < kv.end()) {
            value = std::string(i + 1, kv.end());
            this->headers.insert({to_lower_copy(name), to_lower_copy(value)});
        }
    }
}

const std::map<std::string, std::string> &HttpHeader::getParams() const {
    return params;
}

