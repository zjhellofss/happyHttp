//
// Created by fushenshen on 2020/1/25.
//

#ifndef HAPPYHTTP_HTTPHANDLER_H
#define HAPPYHTTP_HTTPHANDLER_H

#include <event2/util.h>
#include "../Server.h"

class ServerFactory {
public:
    static Server *getServer(const std::string &);


    static void run(const std::string &path);

    static InitConfig *getInitConfig();


private :
    static std::shared_ptr<Server> server;
};

#define HEAD_TABLE "<html><head><title>目录名: %s</title></head><body><h1>当前目录: %s</h1><table>"
#define DIR_PATH "<tr><td><a href=\"%s/\">%s/</a></td><td>%ld</td></tr>"
#define REG_PATH "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>"
#define DIR_CUR_NAME "<body><h1>当前目录: %s</h1><table>"
#define END_TABLE "</table></body></html>"


void socketServerProcess();

static void listenerInit(
        struct evconnlistener *listener,
        evutil_socket_t fd,
        struct sockaddr *addr,
        int len, void *ptr);


static void eventCb(struct bufferevent *bev, short events, void *arg);

static void writeCb(struct bufferevent *bev, void *arg);

static void readCb(struct bufferevent *bev, void *arg);

static void acceptErrorCb(struct evconnlistener *listener, void *ctx);

static void sendResponseHeader(struct bufferevent *bev, int code,
                               const std::string &respCode, const std::string &type,
                               long len, std::string host);

static void sendFile(struct bufferevent *bev, const std::string &path);

static void sendDirectory(struct bufferevent *bev, std::string path, std::string host);

static void encodeStr(char *, size_t toSize, char *from);

static std::string getFileType(const std::string &filetype);

static void send404(bufferevent *bev);

static std::string getDateTime();


#endif //HAPPYHTTP_HTTPHANDLER_H
