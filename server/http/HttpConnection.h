//
// Created by fushenshen on 2020/1/12.
//

#ifndef HAPPYHTTP_HTTPCONNECTION_H
#define HAPPYHTTP_HTTPCONNECTION_H

#include <event.h>

class HttpConnection {
public:
    ~ HttpConnection();

    HttpConnection(int fd, bufferevent *bev);

    int getFd() const;

    bool isTimeOut() ;

    time_t getUseTime() const;

    void setUseTime(time_t useTime);

    bool operator<(const HttpConnection &rhs) const;

    bool operator<(const HttpConnection *rhs) const;

private:
    int fd;
    struct bufferevent *bev;
    time_t useTime;
};

#endif //HAPPYHTTP_HTTPCONNECTION_H
