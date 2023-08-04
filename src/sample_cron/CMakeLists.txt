project(sample_cron)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} TEST_SOURCE)

add_executable(
    ${PROJECT_NAME} 
    ${TEST_SOURCE} 
)