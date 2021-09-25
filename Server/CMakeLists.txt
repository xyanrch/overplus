set(SRCS
    SslServer.cpp
    ServerSession.cpp 
  )
add_library(server ${SRCS})
target_link_libraries(server shared proto)