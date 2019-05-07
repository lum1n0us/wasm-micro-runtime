
set (APP_MGR_SHARED_DIR ${CMAKE_CURRENT_LIST_DIR})

include_directories(${APP_MGR_SHARED_DIR})


file (GLOB_RECURSE source_all ${APP_MGR_SHARED_DIR}/*.c)

set (APP_MGR_SHARED_SOURCE ${source_all})

