project(mysqldb)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/mysqldb/)

if(NOT MSVC)
    target_link_libraries(${PROJECT_NAME} INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libmysqlcppconn-static.a
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libmysqlclient.a
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libssl.so
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libcrypto.so
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libresolv.so)
endif()
