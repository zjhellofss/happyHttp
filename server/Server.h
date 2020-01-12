//
// Created by fushenshen on 2020/1/11.
//

#ifndef HAPPYHTTP_SERVER_H
#define HAPPYHTTP_SERVER_H

#include <string>
#include <evutil.h>
#include "InitConfig.h"

#define DIR_PATH "<tr><td><a href=\"%s/\">%s/</a></td><td>%ld</td></tr>"
#define DIR_NAME "<html><head><title>目录名: %s</title></head>"
#define REG_PATH "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>"
#define DIR_CUR_NAME "<body><h1>当前目录: %s</h1><table>"
#define END_TABLE "</table></body></html>"


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
                                    long len, std::string host);

    static void sendFile (struct bufferevent *bev, const std::string &path);

    static void sendDirectory (struct bufferevent *bev, std::string path, std::string host);


private:
    InitConfig *initConfig = nullptr;
    std::string configPath;

    static void encodeStr (char *, size_t toSize, char *from);

    static std::string getFileType (const std::string &filetype);

    static void send404 (bufferevent *bev);
};


#endif //HAPPYHTTP_SERVER_H
