cmake_minimum_required(VERSION 3.12)
project(happyHttp)

set(CMAKE_CXX_STANDARD 14)

LINK_LIBRARIES(event event_pthreads)
link_directories(/usr/local/lib)
LINK_LIBRARIES(glog)


find_package(Boost 1.67.0 COMPONENTS regex filesystem)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(happyHttp server/Server.cpp server/Server.h server/InitConfig.h server/ServerMain.cpp server/http/HttpHeader.h server/http/HttpHeader.cpp server/http/HttpConnection.h server/http/HttpConnection.cpp server/http/HttpHandler.cpp server/http/HttpHandler.h)
    target_link_libraries(happyHttp ${Boost_LIBRARIES})
endif ()