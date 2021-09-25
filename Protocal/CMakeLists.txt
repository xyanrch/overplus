message(STATUS "Enter Protocal CMakeList ...")
set(proto_SRCS
TrojanReq.cpp
VplusProtocal/Coding.cpp 
VplusProtocal/VRequest.cpp 
)

add_library(proto ${proto_SRCS})
