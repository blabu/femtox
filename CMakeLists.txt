cmake_minimum_required(VERSION 3.1)
set(PROJ_NAME Femtox)

project(${PROJ_NAME})

file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.c*")

add_library(${PROJ_NAME} SHARED ${SRC})

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
target_include_directories(${PROJ_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads REQUIRED)
target_link_libraries(${PROJ_NAME} PRIVATE Threads::Threads)

set(FEMTOX_LIB_NAME ${PROJ_NAME} PARENT_SCOPE)
set(FEMTOX_HEADERS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)