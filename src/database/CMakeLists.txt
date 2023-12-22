project(database)


include_directories(CMAKE_SOURCE_DIR/database ${TOP_CMAKE_DIR}/deps/mysql ${TOP_CMAKE_DIR}/src/include)

# -DQTPBASE
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
aux_source_directory(. SRC_FILES)

add_library(database_so STATIC ${SRC_FILES})
target_link_libraries(database_so mysqlcppconn zlog)


SET_TARGET_PROPERTIES(database_so PROPERTIES OUTPUT_NAME "database")
MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")

