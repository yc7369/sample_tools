project(filetest)


add_executable(zero_copy zero_copy.cpp)
target_link_libraries(zero_copy PUBLIC zlog )
