
project(zlog)


set(BLOG_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/ztp_log.cc)

add_library(zlog STATIC  ${BLOG_SOURCE})

if(MSVC)
    target_link_libraries(zlog PUBLIC  infrabase spdlog)
else()
    target_link_libraries(zlog PUBLIC pthread infrabase spdlog)
endif()

target_include_directories(zlog PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/zlog/)
target_include_directories(zlog INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/) 