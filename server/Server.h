//
// Created by fushenshen on 2020/1/11.
//

#ifndef HAPPYHTTP_SERVER_H
#define HAPPYHTTP_SERVER_H

#include <string>
#include "InitConfig.h"

class Server {

public:
    Server(std::string configPath);

    void init();

    void run();

    void logInit();

    void serverInit();

    virtual ~Server();

    InitConfig *getInitConfig() const;

private:

private:
    InitConfig *initConfig = nullptr;
    std::string configPath;

};


#endif //HAPPYHTTP_SERVER_H
