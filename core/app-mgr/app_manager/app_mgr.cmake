
set (__APP_MGR_DIR ${CMAKE_CURRENT_LIST_DIR})

include_directories(${__APP_MGR_DIR})


file (GLOB source_all ${__APP_MGR_DIR}/*.c  ${__APP_MGR_DIR}/platform/${TARGET_PLATFORM}/*.c)

set (APP_MGR_SOURCE ${source_all})

