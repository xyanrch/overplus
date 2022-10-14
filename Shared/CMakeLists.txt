message(STATUS "Enter Shared CMakeList ...")
aux_source_directory(. shared_SRCS)
add_library(shared ${shared_SRCS})
