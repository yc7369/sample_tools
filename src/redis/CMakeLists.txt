
project(redis)

include_directories(..)
set(REDIS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/redis_mng.cpp)

add_library(redis_mng STATIC  ${REDIS_SOURCE})
target_include_directories(redis_mng PRIVATE redis++ hiredis)