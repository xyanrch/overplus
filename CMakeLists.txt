cmake_minimum_required(VERSION 3.5)
project(overplus)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)#add_definitions(-Wall)
#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)
add_compile_options(-DBOOST_ASIO_CONCURRENCY_HINT_1=BOOST_ASIO_CONCURRENCY_HINT_UNSAFE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#set(Boost_USE_STATIC_LIBS on)
include_directories(.)
find_package(Boost 1.66  COMPONENTS system  REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif()
find_package(Threads REQUIRED)
###generate server binary code
find_package(OpenSSL REQUIRED)


add_subdirectory(Shared)
add_subdirectory(Server)
add_subdirectory(Protocol)
add_subdirectory(Client)

add_executable(overplus Server/main.cpp)
target_link_libraries(overplus Threads::Threads  shared server proto OpenSSL::SSL)
target_link_libraries(overplus ${Boost_LIBRARIES})
##generate client binary code




