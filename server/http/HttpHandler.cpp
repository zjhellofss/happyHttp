//
// Created by fushenshen on 2020/1/25.
//
#include <cassert>
#include <event2/listener.h>
#include <event2/thread.h>
#include <dirent.h>
#include <event.h>
#include <boost/filesystem.hpp>
#include <sys/stat.h>
#include <ctime>
#include <fcntl.h>
#include "HttpConnection.h"
#include <glog/logging.h>
#include <event2/event.h>
#include <cstdlib>
#include "HttpHandler.h"
#include "HttpHeader.h"

std::shared_ptr<Server> ServerFactory::server = nullptr;

Server *ServerFactory::getServer(const std::string &path) {
    if (server != nullptr) {
        auto ptr = server.get();
        return ptr;
    } else {
        server = std::make_shared<Server>(path);
        return server.get();
    }
}

void ServerFactory::run(const std::string &path) {
    Server *serv = ServerFactory::getServer(path);
    serv->run();
    socketServerProcess();
}


InitConfig *ServerFactory::getInitConfig() {
    assert(server != nullptr);
    Server *s = server.get();
    assert(s != nullptr);
    return s->getInitConfig();
}


void socketServerProcess() {
    sockaddr_in serv{};
    evthread_use_pthreads();
    event_base *base;
    base = event_base_new();
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(ServerFactory::getInitConfig()->getPort());
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    auto listener = evconnlistener_new_bind(base, listenerInit, base,
                                            LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_EXEC |
                                            LEV_OPT_DEFERRED_ACCEPT,
                                            36, (struct sockaddr *) &serv, sizeof(serv));
    if (listener != nullptr) {
        LOG(INFO) << "Listener started successfully\n";
    } else {
        //todo 开启备用的端口号
        serv.sin_port = htons(ServerFactory::getInitConfig()->getAlternatePort());
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

void listenerInit(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *addr, int len, void *ptr) {

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


void eventCb(struct bufferevent *bev, short events, void *arg) {
    if (events & BEV_EVENT_ERROR) {
        LOG(ERROR) << "Read ERROR\n";
        auto *connection = (HttpConnection *) arg;
        delete connection;
    } else {
        //不处理
    }
}

void writeCb(struct bufferevent *bev, void *arg) {
    bufferevent_flush(bev, EV_WRITE, BEV_FLUSH);
}

void readCb(struct bufferevent *bev, void *arg) {
    char request[1024] = {0};
    bufferevent_read(bev, request, sizeof(request));
    std::string requestHead(request);
    HttpHeader httpHeader(request);
    if (httpHeader.getMethod() == "POST") {

    } else if (httpHeader.getMethod() == "GET") {
        const std::string &uri = httpHeader.getUri();
        if (boost::filesystem::extension(uri) == ".cgi") {
            int fd[2];
            int r = pipe(fd);
            assert(!r);
            pid_t pid = fork();
            if (!pid) {
                //child
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                const std::string &binFile = "./" + std::string(uri.begin() + 1, uri.end());
                LOG(INFO) << binFile << "\n";
                if (boost::filesystem::exists(binFile)) {
                    auto m = httpHeader.getParams();
                    for (const auto &p:m) {
                        const std::string &n = p.first;
                        const std::string &v = p.second;
                        int res = setenv(n.data(), v.data(), 1);
                        assert(!res);
                    }
                    execlp(binFile.data(), binFile.data(), nullptr);
                } else {
                    send404(bev);
                }
            } else if (pid > 0) {
                //parent
                int status = 0;
                close(fd[1]);
                const int length = 4096;
                char *buf = new char[length];
                int len = 0;
                std::string str;

                while ((len = (int) read(fd[0], buf, length * sizeof(char))) > 0) {
                    *(buf + len) = '\0';
                    str += buf;
                }
                delete[]buf;
                sendResponseHeader(bev, 200, "OK", ".cgi",
                                   str.size(), "");
                bufferevent_write(bev, str.data(), str.size());
                waitpid(pid, nullptr, status);
                close(fd[0]);
            } else {
                //todo
            }
        } else {
            const std::string &staticPage = ServerFactory::getInitConfig()->getStaticPage();
            std::string page;
            if (uri != "/") {
                if (!boost::filesystem::extension(uri).empty() && uri[0] != '/') {
                    page = uri;
                } else {
                    page = "." + uri;
                }
            } else {
                //访问的首页
                page = staticPage + ServerFactory::getInitConfig()->getIndexFile();
            }
            LOG(INFO) << page << "\n";
            bool isExists = boost::filesystem::exists(page);
            auto p = ServerFactory::getInitConfig()->getWorkPath();
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
                assert(page.size() >= 2);
                page.erase(page.begin(), page.begin() + 1);
                page = staticPage + page;
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


}

void acceptErrorCb(struct evconnlistener *listener, void *ctx) {
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    char buf[128];
    sprintf(buf, "Got an error %d (%s) on the listener. "

                 "Shutting down.\n", err, evutil_socket_error_to_string(err));
    LOG(ERROR) << buf;
    //todo 服务器中断，重新启动
}

void sendResponseHeader(struct bufferevent *bev, int code,
                        const std::string &respCode, const std::string &type,
                        long len, std::string host) {

    std::string resp;
    resp.resize(512);
    const std::string &respType = getFileType(type);
    const std::string dateStr = getDateTime();
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

void sendFile(struct bufferevent *bev, const std::string &path) {

    const int length = 4196;
    int fileFd = open(path.data(), O_RDONLY);
    if (fileFd > 0) {
        char *buf = new char[length];
        int len = 0;
        std::string str;
        while ((len = (int) read(fileFd, buf, length * sizeof(char))) > 0) {
            *(buf + len) = '\0';
            str += buf;
        }
        close(fileFd);
        delete[]buf;
        bufferevent_write(bev, str.data(), str.size());
    } else {
        //code 404
        send404(bev);
    }
}


void sendDirectory(struct bufferevent *bev, std::string directoryPath, std::string host) {
    if (directoryPath[directoryPath.size() - 1] != '/') {
        send404(bev);
        return;
    }
    sendResponseHeader(bev, 200, "OK", "text/html; charset=utf-8", -1, "");
    std::string strBuf;
    strBuf.resize(512);
    std::string path;
    path.reserve(512);
    char enStr[512];
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


void encodeStr(char *to, size_t toSize, char *from) {
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

std::string getFileType(const std::string &filetype) {
    if (filetype.empty()) {
        return "text/plain; charset=utf-8";
    } else if (filetype == ".html" || filetype == ".htm") {
        return "text/html; charset=utf-8";
    } else if (filetype == ".jpg" || filetype == ".jpeg") {
        return "image/jpeg";
    } else if (filetype == ".gif") {
        return "image/gif";
    } else if (filetype == ".cgi") {
        return "text/html; charset=utf-8";
    }
    return "text/html; charset=utf-8";
}

void send404(bufferevent *bev) {
    std::string host = ServerFactory::getInitConfig()->getStaticPage() + "/404.html";
    int len = static_cast<int>(boost::filesystem::file_size(host));
    sendResponseHeader(bev, 404, "Not Found", "text/html", len, "");
    sendFile(bev, host);
}

std::string getDateTime() {
    std::string str;
    str.resize(128);
    time_t now = time(nullptr);
    struct tm tm = *gmtime(&now);
    strftime((char *) str.data(), str.size(), "%a, %d %b %Y %H:%M:%S %Z", &tm);
    return str;
}


