# 基于libevent的HttpServer



## 简介
项目主要采用Libevent对client的请求进行处理、boost记录日志和切分请求头。
## 功能

1. 支持对静态页面和多种格式文件的访问
2. 文件浏览器
3. 自定义的错误页面
4. 支持CGI模式
5. 支持以定时器的方式对空置的连接进行回收


## 编译以及使用

## 编译流程
```shell
    mkdir build
    cmake ..
    make 
    sudo make  install
```

## 样例配置文件
在安装完成后在`properties.json`中修改自己需要的配置
```JSON
{
  "port": 9999,
  "index": "hello.html",
  "log_path": "/Users/fss/serverlog/",
  "alternate_port": 9998,
  "static_page": "/www/http/pages/static/",
  "work_path": "/Users/fss/CLionProjects/happyHttp"
}



```




## 启动服务器
终端使用命令`nohup httpServer &`启动服务器

## 致谢
感谢我的好朋友[胡昊](https://github.com/1120023921)为这个项目的开发出谋划策，提供各种各样的帮助。