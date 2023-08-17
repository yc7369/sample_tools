cmake_minimum_required(VERSION 3.8)
cmake_policy(SET CMP0015 NEW)
project(sample_tools)

# cxx flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{CXXFLAGS} -std=c++17 -pthread -Wall -Wwrite-strings")
set(CMAKE_CXX_FLAGS_RELEASE " -O2")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# definitions
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    message(STATUS "using Debug build mode!")
    add_definitions(-D_DEBUG)
else ()
    message(STATUS "using Release build mode!")
    add_definitions(-DNDEBUG)
    if (NOT GIT_VERSION)
        EXECUTE_PROCESS(COMMAND git describe --abbrev=0 --tags
                TIMEOUT 5
                OUTPUT_VARIABLE GIT_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE
                )
    endif ()

    MESSAGE(STATUS "building from git tag ${GIT_VERSION}")
    add_definitions(-DBUILD_VERSION=\"${GIT_VERSION}\")

    string(TIMESTAMP VERSION "%Y-%m-%d %H:%M:%S")
    MESSAGE(STATUS "building VERSION ${VERSION}")
    add_definitions(-DTIME_VERSION=\"${VERSION}\")
endif ()
# add_definitions(-DSPDLOG_HEADER_ONLY)
add_definitions(-DOS_LNX)
add_definitions(-DUSE_BUFFEVENT_DEP)

# message debug
message(STATUS "------------------------------")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "------------------------------")

#outputs / bins / libs
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)
set(TOP_CMAKE_DIR ${CMAKE_SOURCE_DIR})

# deps
include_directories(include)
link_directories(${PROJECT_SOURCE_DIR}/lib)
# add_subdirectory(${PROJECT_SOURCE_DIR}/deps/)

#src
add_subdirectory(src)
# add_subdirectory(test)
