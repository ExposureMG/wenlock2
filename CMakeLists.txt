cmake_minimum_required(VERSION 3.22)
project(wenlock VERSION 2.0 DESCRIPTION "A discord bot")
 
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
 

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/handler.cpp
)
 

find_package(DPP REQUIRED)
 

target_link_libraries(${PROJECT_NAME} 
    ${DPP_LIBRARIES}
)
 

target_include_directories(${PROJECT_NAME} PRIVATE
    ${DPP_INCLUDE_DIR}
    include/
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
