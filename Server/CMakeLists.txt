message(STATUS "Enter Server CMakeList ...")
aux_source_directory(. SRCS)
add_library(server ${SRCS})
target_link_libraries(server shared proto)