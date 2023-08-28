
project(iomn)


set(IOMN_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/metric.cpp)

add_library(iomn STATIC  ${IOMN_SOURCE})

if(MSVC)
    target_link_libraries(iomn PUBLIC  infrabase)
else()
    target_link_libraries(iomn PUBLIC pthread infrabase mybase)
endif()

target_include_directories(iomn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/iomn/)
target_include_directories(iomn INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/) 