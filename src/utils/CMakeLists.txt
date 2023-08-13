project(myutils)


aux_source_directory(. SOURCE_FILES)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}  redis++ hiredis pthread)

# add_library(myutils_so SHARED ${SOURCE_FILES})
# SET_TARGET_PROPERTIES(myutils_so PROPERTIES OUTPUT_NAME "myutil")