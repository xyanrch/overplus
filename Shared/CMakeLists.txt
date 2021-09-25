set(shared_SRCS
    ConfigManage.cpp
    Log.cpp 
    IoContextPool.cpp 
  )
  set(HEADERS
  ConfigManage.h
  Log.h 
  IoContextPool.h 
    )
add_library(shared ${shared_SRCS})
install(FILES ${HEADERS} DESTINATION include/overplus/Shared)
#target_link_libraries(shared)