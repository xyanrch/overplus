message(STATUS "Enter Protocal CMakeList ...")
aux_source_directory(. proto_SRCS)

add_library(proto ${proto_SRCS})
