cmake_minimum_required(VERSION 3.15)

# set(INFRABASE_SRC_DIR ${CMAKE_SOURCE_DIR}/../infrabase)

if(IS_DIRECTORY ${INFRABASE_SRC_DIR})
    set(INFRABASE_SOURCE_MODE TRUE)
    message("\nUSING INFRBASE_SOURCE code\n")
else()
    # add_subdirectory(${CMAKE_SOURCE_DIR}/deps/infrabase)
endif()
