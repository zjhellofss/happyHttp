# 基于libevent的HttpServer



## 简介
项目主要采用Libevent对client的请求进行处理、boost记录日志和切分请求头。
## 功能

2. 支持静态文件的访问以及自定义的www文件夹
3. 可以作为本地文件浏览器使用并对本地文件进行预览
4. 对于服务器内部错误和请求异常提供了5种不同的日志记录等级以及内置错误页面


</table>

## 编译以及使用

### 编译流程
```shell
    mkdir build
    cmake ..
    make 
    sudo make  install
```

### 样例配置文件
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

### 启动服务器
终端使用命令`nohup httpServer &`启动服务器

## 致谢
感谢我的好朋友[胡昊](https://github.com/1120023921)为这个项目的开发出谋划策，提供各种各样的帮助。