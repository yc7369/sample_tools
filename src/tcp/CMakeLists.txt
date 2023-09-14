project(tcp)

# libevent
include_directories(${TOP_CMAKE_DIR}/deps/libevent/include)
include_directories(${CMAKE_BINARY_DIR}/deps/libevent/include)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_FILES)

add_library(${PROJECT_NAME} SHARED  ${SRC_FILES})

target_compile_definitions(${PROJECT_NAME} PRIVATE -DFEATURE_IOMN)
target_link_libraries(${PROJECT_NAME} PUBLIC zlog pthread infrabase event event_core event_openssl event_pthreads iomn)


target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tcp/)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)