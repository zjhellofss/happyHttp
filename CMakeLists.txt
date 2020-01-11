cmake_minimum_required(VERSION 3.12)
project(happyHttp)

set(CMAKE_CXX_STANDARD 14)

LINK_LIBRARIES(event event_pthreads)
link_directories(/usr/local/lib)
LINK_LIBRARIES(glog)

add_executable(happyHttp server/server.cpp server/server.h server/initConfig.h server/serverMain.cpp)