cmake_minimum_required(VERSION 3.5)
project(overplus)
set(CMAKE_CXX_STANDARD 17)
#add_definitions(-Wall)
#
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(srclib "")
target_sources(srclib
    PRIVATE
    Session.cpp
    Server.cpp
    IoContextPool.cpp
    )
add_executable(overplus main.cpp)
find_package(Boost  COMPONENTS system thread REQUIRED)
find_package(OpenSSL REQUIRED)
target_link_libraries(overplus 
srclib 
Boost::system Boost::thread 
OpenSSL::SSL)
#target_link_libraries(YAN compiler)