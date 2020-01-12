//
// Created by fushenshen on 2020/1/12.
//

#include <event.h>
#include "HttpConnection.h"

HttpConnection::~HttpConnection () {
    bufferevent_free(this->bev);
    evutil_closesocket(this->fd);
}

HttpConnection::HttpConnection (int fd, bufferevent *bev) : fd(fd), bev(bev) {}

