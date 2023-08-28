project(linker)

# aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_FILES)
# aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/include INC_FILES)

add_library(linker linker_impl_server.cpp)
target_link_libraries(linker PUBLIC  tcp mybase iomn)

target_include_directories(linker PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/linker/  ${CMAKE_CURRENT_SOURCE_DIR}/../mybase/)
target_include_directories(linker INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/) 


add_executable(s_test ev_server.cpp)
target_link_libraries(s_test linker)