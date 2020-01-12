#include <cassert>
#include <string>
#include <fstream>
#include <streambuf>
#include <glog/logging.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/thread.h>
#include <dirent.h>
#include <event.h>
#include <boost/filesystem.hpp>
#include <sys/stat.h>
#include <ctime>
#include "Server.h"
#include "HttpHeader.h"
#include "../include/rapidjson/document.h"
#include "../postMethod/Post.h"
#include "HttpConnection.h"

//
// Created by fushenshen on 2020/1/11.
//
InitConfig *config;
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
    LOG(WARNING) << "系统关闭\n";
    //关闭日志系统
    google::ShutdownGoogleLogging();
}

void Server::init () {
    serverInit();
    logInit();
    LOG(INFO) << "切换到工作目录\n";
    chdir(this->initConfig->getWorkPath().data());
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
        t.close();
        LOG(INFO) << "日志系统初始化成功\n";
    }
}

void Server::logInit () {
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
                                            LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_EXEC |
                                            LEV_OPT_DEFERRED_ACCEPT,
                                            36, (struct sockaddr *) &serv, sizeof(serv));
    if (listener != nullptr) {
        LOG(INFO) << "Listener started successfully\n";
    } else {
        //todo 开启备用的端口号
        serv.sin_port = htons(this->initConfig->getAlternatePort());
        listener = evconnlistener_new_bind(base, listenerInit, base,
                                           LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                                           36, (struct sockaddr *) &serv, sizeof(serv));
        if (listener != nullptr) {
            LOG(INFO) << "Listener started successfully in alternative port\n";
        } else {
            LOG(FATAL) << "Listener started fail\n";
        }
    }
    evconnlistener_set_error_cb(listener, acceptErrorCb);
    event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);
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
        //将listener修改为非阻塞的模式
        evutil_make_socket_nonblocking(fd);
        //为client添加读和写函数
        bufferevent_setcb(bev, readCb, writeCb, eventCb, new HttpConnection(fd, bev));
        //以水平模式读取输入
        bufferevent_enable(bev, EV_READ);
    } else {
        LOG(WARNING) << "Failed to create client\n";
    }
}


void Server::eventCb (struct bufferevent *bev, short events, void *arg) {
    if (events & BEV_EVENT_ERROR) {
        LOG(ERROR) << "Read ERROR\n";
        auto *connection = (HttpConnection *) arg;
        delete connection;
    } else {
        //不处理
    }
}

void Server::writeCb (struct bufferevent *bev, void *arg) {
    bufferevent_flush(bev, EV_WRITE, BEV_FLUSH);
}

void Server::readCb (struct bufferevent *bev, void *arg) {
    char request[1024] = {0};
    bufferevent_read(bev, request, sizeof(request));
    std::string requestHead(request);
    HttpHeader httpHeader(request);
    if (httpHeader.getMethod() == "POST") {

    } else if (httpHeader.getMethod() == "GET") {
        const std::string &uri = httpHeader.getUri();
        const std::string &staticPage = config->getStaticPage();
        std::string page;
        if (uri != "/") {
            if (!boost::filesystem::extension(uri).empty() && uri[0] != '/') {
                page = uri;
            } else {
                page = "." + uri;
            }
        } else {
            //访问的首页
            page = "." + staticPage + config->getIndexFile();
        }
        bool isExists = boost::filesystem::exists(page);
        auto p = boost::filesystem::current_path().string();
        //在文件存在的情况下
        if (isExists) {
            LOG(INFO) << "Visit uri successfully:" << httpHeader.getUri() << "\n";
            if (boost::filesystem::is_directory(page)) {
                //文件是一个目录
                sendDirectory(bev, page, httpHeader.getValue("host"));
            } else {
                //存在非目录文件
                sendResponseHeader(bev, 200, "ok", boost::filesystem::extension(page),
                                   boost::filesystem::file_size(page),
                                   "");
                sendFile(bev, page);
            }
        } else {
            page = std::string(page.begin() + 1, page.end());
            page = "." + staticPage + page;
            if (boost::filesystem::exists(page)) {
                //文件是位于static 目录中的一个文件
                LOG(INFO) << "Visit uri successfully:" << httpHeader.getUri() << "\n";
                if (boost::filesystem::is_directory(page)) {
                    sendDirectory(bev, page, httpHeader.getValue("host"));
                } else {
                    sendResponseHeader(bev, 200, "OK", getFileType(boost::filesystem::extension(page)),
                                       boost::filesystem::file_size(page), "");
                    sendFile(bev, page);
                }
            } else {
                //404
                LOG(INFO) << "404 Not found:" << httpHeader.getUri() << "\n";
                send404(bev);
            }

        }
    }


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

void Server::sendResponseHeader (struct bufferevent *bev, int code,
                                 const std::string &respCode, const std::string &type,
                                 long len, std::string host) {

    std::string resp;
    resp.resize(512);
    std::string respType = getFileType(type);
    std::string dateStr = DatePostMethod().getDateTime();
    sprintf((char *) resp.data(), "HTTP/1.1 %d %s\r\nContent-Type:%s\r\nContent-Length:%ld\r\nServer:%s\r\nDate:%s\r\n",
            code,
            respCode.data(), type.data(), len, "happyHttp", dateStr.data());
    if (code == 301) {
        sprintf((char *) resp.data() + strlen(resp.data()), "Location:http://%s\r\n", host.data());
    }
    sprintf((char *) resp.data() + strlen(resp.data()), "\r\n");
    size_t dataLen = strlen(resp.data());
    bufferevent_write(bev, resp.data(), dataLen);
}

void Server::sendFile (struct bufferevent *bev, const std::string &path) {

    std::ifstream file(path);
    std::string str((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());
    if (file.is_open()) {
        bufferevent_write(bev, str.data(), strlen(str.data()));
    } else {
        //code 500
    }
    file.close();
}


void Server::sendDirectory (struct bufferevent *bev, std::string directoryPath, std::string host) {
    if (directoryPath[directoryPath.size() - 1] != '/') {
        send404(bev);
        return;
    }
    sendResponseHeader(bev, 200, "OK", "text/html; charset=utf-8", -1, "");
    std::string strBuf;
    strBuf.resize(128);
    std::string path;
    path.reserve(128);
    char enStr[128];
    const char *dirname = directoryPath.c_str();
    char *bufStr = (char *) strBuf.c_str();
    sprintf(bufStr, HEAD_TABLE, dirname,
            dirname);
    struct dirent **ptr;
    int num = scandir(dirname, &ptr, nullptr, alphasort);
    if (num <= 0) {
        //log
    } else {
        for (int i = 0; i < num; ++i) {
            char *pathStr = (char *) path.c_str();
            char *name = ptr[i]->d_name;
            sprintf(pathStr, "%s/%s", dirname, name);
            struct stat st{};
            stat(pathStr, &st);
            encodeStr(enStr, sizeof(enStr), name);
            if (S_ISREG(st.st_mode)) {
                sprintf(bufStr + strlen(bufStr),
                        REG_PATH,
                        enStr, name, (long) st.st_size);
            } else if (S_ISDIR(st.st_mode)) {
                sprintf(bufStr + strlen(bufStr),
                        DIR_PATH,
                        enStr, name, (long) st.st_size);
            }
            bufferevent_write(bev, bufStr, strlen(bufStr));
            strBuf.clear();
        }
        sprintf(bufStr + strlen(bufStr), END_TABLE);
        bufferevent_write(bev, bufStr, strlen(bufStr));
    }
    free(ptr);
}


void Server::encodeStr (char *to, size_t toSize, char *from) {
    int toLen;

    for (toLen = 0; *from != '\0' && toLen + 4 < toSize; ++from) {
        if (isalnum(*from) || strchr("/_.-~", *from) != (char *) 0) {
            *to = *from;
            ++to;
            ++toLen;
        } else {
            sprintf(to, "%%%02x", (int) *from & 0xff);
            to += 3;
            toLen += 3;
        }
    }
    *to = '\0';
}

std::string Server::getFileType (const std::string &filetype) {
    if (filetype.empty()) {
        return "text/plain; charset=utf-8";
    }
    if (filetype == ".html" || filetype == ".htm") {
        return "text/html; charset=utf-8";
    }
    if (filetype == ".jpg" || filetype == ".jpeg") {
        return "image/jpeg";
    }
    if (filetype == ".gif") {
        return "image/gif";
    }
    return "text/plain; charset=utf-8";
}

void Server::send404 (bufferevent *bev) {
    std::string host = "." + config->getStaticPage() + "404.html";
    int len = static_cast<int>(boost::filesystem::file_size(host));
    sendResponseHeader(bev, 404, "Not Found", "text/html", len, "");
    sendFile(bev, host);
}









