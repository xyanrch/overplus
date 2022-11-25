message(STATUS "Enter Server CMakeList ...")

set(SRCS
        ServerSession.cpp
        websocketSession.cpp
        SslServer.cpp
        )
add_library(server ${SRCS})
target_link_libraries(server shared proto)