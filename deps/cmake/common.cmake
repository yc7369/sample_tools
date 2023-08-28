cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0015 NEW)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(BUILD_SHARED_LIBS)
# ....
else()
    message("===> Build Lib with static")
    set(EVENT_LIBRARY_TYPE STATIC)
    set(EVENT__LIBRARY_TYPE STATIC)
endif()

# cxx flags
if(NOT MSVC)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{CXXFLAGS} -Wall  -pthread -Wfatal-errors -std=c++20 -rdynamic -Wno-format-security -Wwrite-strings -fPIC")
    set(CMAKE_CXX_FLAGS_RELEASE " -O2")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{CXXFLAGS} /std:c++17 /utf-8 /MP /bigobj")
    # add_compile_options("/source-charset:utf-8 /Mp")
endif()

#outputs / bins / libs
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_OUTPUT_PATH})
if(MSVC)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_OUTPUT_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_OUTPUT_PATH})
endif()

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

if(MSVC)
    # Use the static C library for all build types
    foreach(var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )
        if(${var} MATCHES "/MT")
            string(REGEX REPLACE "/MT" "/MD" ${var} "${${var}}")
        endif()
    endforeach()

add_definitions(
  /wd4065 # switch statement contains 'default' but no 'case' labels
  /wd4244 # 'conversion' conversion from 'type1' to 'type2', possible loss of data
  /wd4251 # 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
  /wd4267 # 'var' : conversion from 'size_t' to 'type', possible loss of data
  /wd4305 # 'identifier' : truncation from 'type1' to 'type2'
  /wd4307 # 'operator' : integral constant overflow
  /wd4309 # 'conversion' : truncation of constant value
  /wd4334 # 'operator' : result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
  /wd4355 # 'this' : used in base member initializer list
  /wd4506 # no definition for inline function 'function'
  /wd4800 # 'type' : forcing value to bool 'true' or 'false' (performance warning)
  /wd4996 # The compiler encountered a deprecated declaration.
  /wd4090 # 'type' : 'identifier' was previously declared as a different type.
  /wd4661 # 'identifier' : no definition available for use in 'template'
)
  if((MSVC_VERSION GREATER 1929) AND (MSVC_VERSION LESS 1940))

    set(DETECTED_TOOLSET "vc143")

  elseif((MSVC_VERSION GREATER 1919) AND (MSVC_VERSION LESS 1930))

    set(DETECTED_TOOLSET "vc142")

  elseif((MSVC_VERSION GREATER 1909) AND (MSVC_VERSION LESS 1920))

    set(DETECTED_TOOLSET "vc141")

  elseif(MSVC_VERSION EQUAL 1900)

    set(DETECTED_TOOLSET "vc140")

  elseif(MSVC_VERSION EQUAL 1800)

    set(DETECTED_TOOLSET "vc120")

  elseif(MSVC_VERSION EQUAL 1700)

    set(DETECTED_TOOLSET "vc110")
  endif()

  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(DETECTED_TOOLSET "${DETECTED_TOOLSET}-x86")
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(DETECTED_TOOLSET "${DETECTED_TOOLSET}-x64")
  endif()

  set(DETECTED_TOOLSET -${DETECTED_TOOLSET}-mt)

  set(CMAKE_DEBUG_POSTFIX "${DETECTED_TOOLSET}-d")
  set(CMAKE_RELEASE_POSTFIX ${DETECTED_TOOLSET})
  
  message(STATUS "using toolset  ${DETECTED_TOOLSET}!")
endif()

# message debug
message(STATUS "------------------------------")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "------------------------------")

set(EVENT__DISABLE_SAMPLES ON)
set(EVENT__DISABLE_REGRESS ON)
set(EVENT__DISABLE_TESTS ON)
