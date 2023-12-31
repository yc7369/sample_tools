project(redis++)

if (NOT DEFINED REDIS_PLUS_PLUS_CXX_STANDARD)
    set(REDIS_PLUS_PLUS_CXX_STANDARD 11)
endif()

message(STATUS "The CXX standard is c++${REDIS_PLUS_PLUS_CXX_STANDARD}")

set(CMAKE_CXX_FLAGS "-std=c++${REDIS_PLUS_PLUS_CXX_STANDARD} -Wall -W -Werror -fPIC")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/sw/redis++)

file(GLOB PROJECT_SOURCE_FILES "${PROJECT_SOURCE_DIR}/*.cpp")
file(GLOB PROJECT_SOURCE_FILES2 "${PROJECT_SOURCE_DIR}/recipes/*.cpp")

set(STATIC_LIB static)
set(SHARED_LIB shared)

add_library(${STATIC_LIB} STATIC ${PROJECT_SOURCE_FILES} ${PROJECT_SOURCE_FILES2})
add_library(${SHARED_LIB} SHARED ${PROJECT_SOURCE_FILES} ${PROJECT_SOURCE_FILES2})

# hiredis dependency
# find_path(HIREDIS_HEADER hiredis)
set(HIREDIS_HEADER ../hiredis)
include_directories(..)
target_include_directories(${STATIC_LIB} PUBLIC ${HIREDIS_HEADER})
target_include_directories(${SHARED_LIB} PUBLIC ${HIREDIS_HEADER})

# find_library(HIREDIS_LIB hiredis)
set(HIREDIS_LIB_PATH ${LIBRARY_OUTPUT_PATH})
link_directories(${HIREDIS_LIB_PATH})

set(HIREDIS_LIB hiredis)
target_link_libraries(${SHARED_LIB} ${HIREDIS_LIB})

set_target_properties(${STATIC_LIB} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set_target_properties(${SHARED_LIB} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

set_target_properties(${STATIC_LIB} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(${SHARED_LIB} PROPERTIES CLEAN_DIRECT_OUTPUT 1)

option(REDIS_PLUS_PLUS_BUILD_TEST "Build tests for redis++" OFF)

if(REDIS_PLUS_PLUS_BUILD_TEST)
    add_subdirectory(test)
endif()

# Install static lib.
install(TARGETS ${STATIC_LIB}
        ARCHIVE DESTINATION lib)

# Install shared lib.
install(TARGETS ${SHARED_LIB}
        LIBRARY DESTINATION lib)

#Install headers.
set(HEADER_PATH "src/sw/redis++")
file(GLOB HEADERS "${PROJECT_SOURCE_DIR}/*.h*")
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/${HEADER_PATH})
