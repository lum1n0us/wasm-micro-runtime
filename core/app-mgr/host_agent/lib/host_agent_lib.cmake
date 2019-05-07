
set (LIB_HOST_AGENT_DIR ${CMAKE_CURRENT_LIST_DIR})

include_directories(${LIB_HOST_AGENT_DIR})
include_directories(${LIB_HOST_AGENT_DIR}/linux)


file (GLOB_RECURSE source_all ${LIB_HOST_AGENT_DIR}/*.c)

set (LIB_HOST_AGENT_SOURCE ${source_all})

