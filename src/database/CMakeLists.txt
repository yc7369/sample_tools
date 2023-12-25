project(database)


include_directories(${TOP_CMAKE_DIR}/deps/mysql/include ${TOP_CMAKE_DIR}/src/include ${TOP_CMAKE_DIR}/src/database)
link_directories(${TOP_CMAKE_DIR}/deps/mysql/lib)

# -DQTPBASE
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
aux_source_directory(src SRC_FILES)
aux_source_directory(${TOP_CMAKE_DIR}/deps/mysql MYSQL_SRC_FILES)

add_library(database SHARED ${SRC_FILES} ${MYSQL_SRC_FILES})
target_link_libraries(database 
mysqlcppconn-static
mysqlclient
${TOP_CMAKE_DIR}/deps/mysql/lib/libssl.so
${TOP_CMAKE_DIR}/deps/mysql/lib/libcrypto.so
resolv
zlog)

# SET_TARGET_PROPERTIES(database_so PROPERTIES OUTPUT_NAME "database")
# MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")

add_executable(mysqltest main.cpp)
target_link_libraries(mysqltest database mysqlclient)

