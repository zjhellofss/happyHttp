//
// Created by fushenshen on 2020/1/11.
//

#include "Server.h"

int main (int argc, char *argv[]) {


    Server s("../www/http/properties/properties.json");
    s.run();
    return 0;
}