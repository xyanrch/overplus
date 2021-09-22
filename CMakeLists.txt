cmake_minimum_required(VERSION 3.5)
project(overplus)
set(CMAKE_CXX_STANDARD 17)
#add_definitions(-Wall)
#
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#set(Boost_USE_STATIC_LIBS on)
add_library(srclib "")
target_sources(srclib
    PRIVATE
    Session.cpp
    ServerSession.cpp
    Server.cpp
    SslServer.cpp
    IoContextPool.cpp
    Log.cpp
    Protocal/TrojanReq.cpp
    ConfigManage.cpp
    )
add_executable(overplus main.cpp)
find_package(Boost 1.66  COMPONENTS system  REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(overplus pthread)
target_include_directories(overplus PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(overplus ${Boost_LIBRARIES})
find_package(OpenSSL REQUIRED)
target_link_libraries(overplus 
srclib 
OpenSSL::SSL)
#target_link_libraries(YAN compiler)