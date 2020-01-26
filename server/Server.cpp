#include <string>
#include <fstream>
#include <iostream>
#include <utility>
#include <glog/logging.h>
#include "Server.h"
#include "http/HttpHeader.h"
#include "../include/rapidjson/document.h"

//
// Created by fushenshen on 2020/1/11.
//
InitConfig *config;

void Server::run() {
    if (!initConfig) {
        //获取服务器的配置文件
        this->init();
    }
}

Server::~Server() {
    if (initConfig != nullptr) {
        delete initConfig;
        initConfig = nullptr;
    }
    LOG(WARNING) << "系统关闭\n";
    //关闭日志系统
    google::ShutdownGoogleLogging();
}

void Server::init() {
    serverInit();
    logInit();
    LOG(INFO) << "切换到工作目录\n";
    chdir(this->initConfig->getWorkPath().data());
}

Server::Server(std::string configPath) : configPath(std::move(configPath)) {

}

void Server::serverInit() {
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
        t.close();
        Document d;
        try {
            d.Parse(jsonStr.data());
            //配置服务器绑定的端口
            initConfig->setPort(d["port"].GetInt());
            //配置服务器的主页
            initConfig->setIndexFile(d["index"].GetString());
            //配置服务器的日志文件
            initConfig->setLogPath(d["log_path"].GetString());
            //配置服务器的静态目录
            initConfig->setStaticPage(d["static_page"].GetString());
            //配置服务器的备用端口
            initConfig->setAlternatePort(d["alternate_port"].GetInt());
            initConfig->setWorkPath(d["work_path"].GetString());
            config = this->initConfig;
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            throw std::runtime_error("The server configuration file cannot be opened properly");
        }
    }
}

void Server::logInit() {
    google::InitGoogleLogging("happyHttp");//日志前缀
    std::string basePath = this->initConfig->getLogPath();

    FLAGS_log_dir = basePath;
    FLAGS_logbufsecs = 5;//延迟写
    FLAGS_max_log_size = 100;//日志文件最大为100MB
    FLAGS_stop_logging_if_full_disk = true;//磁盘满时停止记录
    FLAGS_colorlogtostderr = true;

    google::SetLogDestination(google::INFO, (basePath + "INFO_").data());
    google::SetLogDestination(google::WARNING, (basePath + "WARNING_").data());
    google::SetLogDestination(google::ERROR, (basePath + "ERROR_").data());
    google::SetLogDestination(google::FATAL, (basePath + "FATAL_").data());

    google::SetStderrLogging(google::INFO);
}

InitConfig *Server::getInitConfig() const {
    return initConfig;
}

std::set<HttpConnection*> &Server::getConnections() {
    return connections;
}











