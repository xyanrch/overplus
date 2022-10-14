message(STATUS "Enter Protocal CMakeList ...")
set(proto_SRCS
        socks5/socks5.cpp
        Coding.cpp
        TrojanReq.cpp
        VProtocal/VRequest.cpp
        )

add_library(proto ${proto_SRCS})
target_link_libraries(proto shared)
