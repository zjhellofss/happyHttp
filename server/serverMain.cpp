//
// Created by fushenshen on 2020/1/11.
//

#include "server.h"

int main (int argc, char *argv[]) {
    Server s("/Users/fss/CLionProjects/happyHttp/www/http/properties/properties.json");
    s.run();
    return 0;
}