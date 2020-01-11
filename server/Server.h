//
// Created by fushenshen on 2020/1/11.
//

#ifndef HAPPYHTTP_SERVER_H
#define HAPPYHTTP_SERVER_H

#include <string>
#include <evutil.h>
#include "InitConfig.h"


class Server {

public:
    Server (const std::string &configPath);

    void init ();

    void run ();

    void logInit ();

    void serverInit ();

    virtual ~Server ();

private:
    void socketServerProcess ();

    static void listenerInit (
            struct evconnlistener *listener,
            evutil_socket_t fd,
            struct sockaddr *addr,
            int len, void *ptr);


    static void eventCb (struct bufferevent *bev, short events, void *arg);

    static void writeCb (struct bufferevent *bev, void *arg);

    static void readCb (struct bufferevent *bev, void *arg);

    static void acceptErrorCb (struct evconnlistener *listener, void *ctx);

    static void sendResponseHeader (struct bufferevent *bev, int code,
                                    const std::string &respCode, const std::string &type,
                                    long len);

    static void sendFile (struct bufferevent *bev, const std::string &path);

private:
    InitConfig *initConfig = nullptr;
    std::string configPath;

};


#endif //HAPPYHTTP_SERVER_H
