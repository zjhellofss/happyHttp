//
// Created by fushenshen on 2020/1/11.
//

#ifndef HAPPYHTTP_INITCONFIG_H
#define HAPPYHTTP_INITCONFIG_H

#include <string>

class InitConfig {
public:
    explicit InitConfig () {}

    int getPort () const {
        return port;
    }

    void setPort (int port) {
        InitConfig::port = port;
    }

    int getAlternatePort () const {
        return alternatePort;
    }

    void setAlternatePort (int alternatePort) {
        InitConfig::alternatePort = alternatePort;
    }

    const std::string &getIndexFile () const {
        return indexFile;
    }

    void setIndexFile (const std::string &indexFile) {
        InitConfig::indexFile = indexFile;
    }

    const std::string &getLogPath () const {
        return logPath;
    }

    void setLogPath (const std::string &logPath) {
        InitConfig::logPath = logPath;
    }

    const std::string &getStaticPage () const {
        return staticPage;
    }

    void setStaticPage (const std::string &staticPage) {
        InitConfig::staticPage = staticPage;
    }

    const std::string &getWorkPath () const {
        return workPath;
    }

    void setWorkPath (const std::string &workPath) {
        InitConfig::workPath = workPath;
    }

private:
    int port;
    int alternatePort;//服务器的备用端口
    std::string indexFile;
    std::string logPath;
    std::string staticPage;
    std::string workPath;
};

#endif //HAPPYHTTP_INITCONFIG_H
