project(linker)

# aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_FILES)
# aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/include INC_FILES)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# add_library(linker l1/linker_impl_server.cpp)
# target_link_libraries(linker PUBLIC  tcp mybase iomn


# add_executable(l1_server l1/ev_server.cpp)
# target_link_libraries(l1_server linker)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/l1 l1_SRC_FILES)
add_executable(l1_server ${l1_SRC_FILES})
target_link_libraries(l1_server tcp mybase)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/l2 l2_SRC_FILES)
add_executable(l2_server ${l2_SRC_FILES})
target_link_libraries(l2_server tcp mybase zlog)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/c1 c1_SRC_FILES)
add_executable(l1_client ${c1_SRC_FILES})
target_link_libraries(l1_client tcp mybase)