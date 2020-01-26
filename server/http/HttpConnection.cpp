//
// Created by fushenshen on 2020/1/12.
//

#include <event.h>
#include "HttpConnection.h"

HttpConnection::~HttpConnection() {
    bufferevent_free(this->bev);
    evutil_closesocket(this->fd);
}

HttpConnection::HttpConnection(int fd, bufferevent *bev) : fd(fd), bev(bev),
                                                           useTime(time(nullptr)) {}

int HttpConnection::getFd() const {
    return fd;
}

time_t HttpConnection::getUseTime() const {
    return useTime;
}

void HttpConnection::setUseTime(time_t useTime) {
    HttpConnection::useTime = useTime;
}

bool HttpConnection::operator<(const HttpConnection &rhs) const {
    return useTime < rhs.useTime;
}



