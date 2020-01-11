#include <cassert>
#include <fstream>
#include <glog/logging.h>
#include "server.h"
#include "../include/rapidjson/document.h"
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/thread.h>
#include <event.h>

//
// Created by fushenshen on 2020/1/11.
//
void Server::run () {
    if (!initConfig) {
        //获取服务器的配置文件
        this->init();
    }
    this->socketServerProcess();

}

Server::~Server () {
    if (initConfig != nullptr) {
        delete initConfig;
        initConfig = nullptr;
    }
    //关闭日志系统
    LOG(ERROR) << "系统关闭\n";
    google::ShutdownGoogleLogging();
}

void Server::init () {
    serverInit();
    logInit();

}

Server::Server (const std::string &configPath) : configPath(configPath) {

}

void Server::serverInit () {
    if (this->initConfig != nullptr) {
        return;
    } else {
        using namespace rapidjson;
        this->initConfig = new InitConfig();
        std::ifstream t(configPath);
        if (!t.is_open()) {
            throw std::runtime_error("The server configuration file cannot be opened properly");
        }
        std::string jsonStr((std::istreambuf_iterator<char>(t)),
                            std::istreambuf_iterator<char>());
        Document d;
        d.Parse(jsonStr.data());
        //配置服务器绑定的端口
        if (!d.HasMember("port")) {
            throw std::runtime_error("Missing server port in configuration file");
        } else {
            initConfig->setPort(d["port"].GetInt());
        }
        //配置服务器的主页
        if (!d.HasMember("index")) {
            throw std::runtime_error("Missing server index page in configuration file");
        } else {
            initConfig->setIndexFile(d["index"].GetString());
        }
        //配置服务器的日志文件
        if (!d.HasMember("log_path")) {
            throw std::runtime_error("Missing server log file path in configuration file");
        } else {
            initConfig->setLogPath(d["log_path"].GetString());
        }
        //配置服务器的静态目录
        if (!d.HasMember("static_page")) {
            throw std::runtime_error("Missing server static page path in configuration file");
        } else {
            initConfig->setStaticPage(d["static_page"].GetString());
        }

        //配置服务器的备用端口
        if (!d.HasMember("alternate_port")) {
            throw std::runtime_error("Missing server alternate_port in configuration file");
        } else {
            initConfig->setAlternatePort(d["alternate_port"].GetInt());
        }

    }
}

void Server::logInit () {
    google::InitGoogleLogging("happyHttp");//日志前缀
    std::string basePath = this->initConfig->getLogPath();
    FLAGS_log_dir = basePath;
    google::SetLogDestination(google::INFO, (basePath + "INFO_").data());
    google::SetLogDestination(google::WARNING, (basePath + "WARNING_").data());
    google::SetLogDestination(google::ERROR, (basePath + "ERROR_").data());
    google::SetLogDestination(google::FATAL, (basePath + "FATAL_").data());

    FLAGS_logbufsecs = 5;//延迟写
    FLAGS_max_log_size = 100;//日志文件最大为100MB
    FLAGS_stop_logging_if_full_disk = true;//磁盘满时停止记录
    google::SetStderrLogging(google::INFO);
    FLAGS_colorlogtostderr = true;

}


void Server::socketServerProcess () {
    sockaddr_in serv{};
    evthread_use_pthreads();
    event_base *base;
    base = event_base_new();
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(this->initConfig->getPort());
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    auto listener = evconnlistener_new_bind(base, listenerInit, base,
                                            LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                                            36, (struct sockaddr *) &serv, sizeof(serv));
    if (listener != nullptr) {
        LOG(INFO) << "Listener started successfully\n";
    } else {
        //todo 开启备用的端口号
    }
    evconnlistener_set_error_cb(listener, acceptErrorCb);
    event_base_dispatch(base);
    evconnlistener_free(listener);
    event_base_free(base);

}

void
Server::listenerInit (struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *addr, int len, void *ptr) {

    auto *base = (struct event_base *) ptr;
    bufferevent *bev;
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (bev != nullptr) {
        LOG(INFO) << "Accept client successfully ,socket fd = " << fd << "\n";
    } else {
        LOG(ERROR) << "Failed to create client\n";
        exit(1);
    }
    //将listener修改为非阻塞的模式
    evutil_make_socket_nonblocking(fd);
    //为服务器添加读和写函数
    bufferevent_setcb(bev, readCb, writeCb, eventCb, nullptr);
    //以水平模式读取输入
    bufferevent_enable(bev, EV_READ);
}


void Server::eventCb (struct bufferevent *bev, short events, void *arg) {
    if (events & BEV_EVENT_ERROR) {
        LOG(ERROR) << "Read ERROR\n";
    }
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        LOG(WARNING) << "Read EOF\n";
        bufferevent_free(bev);
    }
}

void Server::writeCb (struct bufferevent *bev, void *arg) {

}

void Server::readCb (struct bufferevent *bev, void *arg) {

}

void Server::acceptErrorCb (struct evconnlistener *listener, void *ctx) {
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    char buf[128];
    sprintf(buf, "Got an error %d (%s) on the listener. "

                 "Shutting down.\n", err, evutil_socket_error_to_string(err));
    LOG(ERROR) << buf;
    //服务器中断，重新启动
    event_base_loopexit(base, nullptr);
    evconnlistener_free(listener);
}



