#add_library(spdlogs INTERFACE)
project(spdlog)

add_library(spdlog INTERFACE)


target_include_directories(spdlog INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/  ${CMAKE_CURRENT_SOURCE_DIR}/spdlog)

