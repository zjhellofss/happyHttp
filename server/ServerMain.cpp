//
// Created by fushenshen on 2020/1/11.
//

#include "Server.h"
#include "http/HttpHandler.h"

int main(int argc, char *argv[]) {

    ServerFactory::run("/Users/fss/CLionProjects/httpserver/happyHttp/www/http/properties/properties.json");
    return 0;
}