# all other
project(use_mysql)


# link_directories(${PROJECT_SOURCE_DIR}/lib  LIBRARY_OUTPUT_PATH)
include_directories(include ${TOP_CMAKE_DIR}/deps/mysql/include)

add_executable(use_mysql main.cpp slog.cpp)

# protobuf
link_directories(${CMAKE_SOURCE_DIR}/deps/protobuf/lib
        ${CMAKE_SOURCE_DIR}/deps/protobuf/lib
        ${TOP_CMAKE_DIR}/deps/mysql/lib
)

target_link_libraries(use_mysql PRIVATE
        zlog
        dl
        pthread
        mybase
        mysqldb
)