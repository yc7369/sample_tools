# all other
project(transfer_mysql)

link_directories(${PROJECT_SOURCE_DIR}/lib  LIBRARY_OUTPUT_PATH)
include_directories(include)

add_executable(transfer_mysql main.cpp slog.cpp)

find_library(SSL_LIB ssl PATHS ./lib)
find_library(CRYPTO_LIB crypto PATHS ./lib)

# protobuf
link_directories(${CMAKE_SOURCE_DIR}/deps/protobuf/lib
        ${CMAKE_SOURCE_DIR}/deps/protobuf/lib
)

target_link_libraries(transfer_mysql PRIVATE
        zlog
        dl
        pthread
        mysqlcppconn-static
        mysqlclient
        ${SSL_LIB}
        ${CRYPTO_LIB}
        resolv
)