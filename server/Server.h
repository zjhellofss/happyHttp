//
// Created by fushenshen on 2020/1/11.
//

#ifndef HAPPYHTTP_SERVER_H
#define HAPPYHTTP_SERVER_H

#include <string>
#include <set>
#include "InitConfig.h"
#include "http/HttpConnection.h"

class Server {

public:
    Server(std::string configPath);

    void init();

    void run();

    void logInit();

    void serverInit();

    virtual ~Server();

    InitConfig *getInitConfig() const;

    std::set<HttpConnection *> &getConnections();

private:

private:
    InitConfig *initConfig = nullptr;
    std::string configPath;
    std::set<HttpConnection *> connections;
};


#endif //HAPPYHTTP_SERVER_H
