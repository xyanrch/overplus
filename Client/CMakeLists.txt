message(STATUS "Enter client CMakeList ...")
aux_source_directory(. SRCS)
add_library(client ${SRCS})
target_link_libraries(client shared proto)