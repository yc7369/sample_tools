project(zktest)

add_executable(
    zkfind zkfind.cpp
)
target_link_libraries(zkfind zookeeper_mt)

add_executable(
    zklock zklock.cpp
)
target_link_libraries(zklock zookeeper_mt)