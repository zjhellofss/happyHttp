//
// Created by fushenshen on 2020/1/12.
//

#ifndef HAPPYHTTP_HTTPCONNECTION_H
#define HAPPYHTTP_HTTPCONNECTION_H

#include <event.h>

class HttpConnection {
public:
    ~ HttpConnection ();

    HttpConnection (int fd, bufferevent *bev);

private:
    int fd;
    struct bufferevent *bev;
};

#endif //HAPPYHTTP_HTTPCONNECTION_H
