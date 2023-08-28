cmake_minimum_required(VERSION 3.8)
cmake_policy(SET CMP0015 NEW)
cmake_policy(SET CMP0048 NEW)
project(redis)

# include_directories()

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTHREADED -static-libstdc++ -static-libgcc")

# find_path(HIREDIS_HEADER hiredis)
# target_include_directories(target PUBLIC ${HIREDIS_HEADER})

# find_library(HIREDIS_LIB hiredis)
# target_link_libraries(target ${HIREDIS_LIB})
set(TOP_DIR ${PROJECT_SOURCE_DIR})
message("----> top_dir:${TOP_DIR}")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../lib)

add_subdirectory(openssl)
add_subdirectory(libevent)
# add_subdirectory(hiredis)
# add_subdirectory(redispp)
add_subdirectory(spdlog)


# file copy
# file(<COPY|INSTALL> <files>... DESTINATION )
