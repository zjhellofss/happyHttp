//
// Created by fushenshen on 2020/1/11.
//

#include "Server.h"
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <iostream>

int main (int argc, char *argv[]) {


    Server s("/Users/fss/CLionProjects/happyHttp/www/http/properties/properties.json");
    s.run();
//    using namespace std;
//    string str("GET / HTTP/1.1\n"
//               "Host: 127.0.0.1:9999\n"
//               "Connection: keep-alive\n"
//               "Upgrade-Insecure-Requests: 1\n"
//               "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.117 Safari/537.36\n"
//               "Sec-Fetch-User: ?1\n"
//               "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\n"
//               "Sec-Fetch-Site: none\n"
//               "Sec-Fetch-Mode: navigate\n"
//               "Accept-Encoding: gzip, deflate, br\n"
//               "Accept-Language: zh-CN,zh;q=0.9");
//    vector<string> strVec;
//    using namespace boost::algorithm;
//    split(strVec, str, is_any_of("\n"));
//    auto it = strVec.begin();
//    for (; it != strVec.end(); it++) {
//        vector<string> val;
//        split(val, *it, is_any_of(":"));
//        for (const auto &e:val) {
//            std::cout << e << "-";
//        }
//        cout << "\n";
//    }
    return 0;
}