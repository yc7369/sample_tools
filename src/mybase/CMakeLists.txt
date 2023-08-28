
aux_source_directory(./src SRC_FILES)
aux_source_directory(./include INC_FILES)

add_library(mybase STATIC ${SRC_FILES} ${INC_FILES})

if(MSVC)
    target_link_libraries(mybase PUBLIC  infrabase)
else()
    target_link_libraries(mybase PUBLIC dl pthread infrabase)
endif()

target_include_directories(mybase PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/mybase/)
target_include_directories(mybase INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/) 