cmake_minimum_required(VERSION 3.31)
project(Aurora VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(GLAD_DIR ${CMAKE_SOURCE_DIR}/glad)
set(AURORA_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/test)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${SRC_DIR}/*.cpp

    ${TEST_DIR}/*.cpp
)
if (PROJECT_IS_TOP_LEVEL)
    add_executable(${PROJECT_NAME} ${SOURCES})
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCES})
endif()

if (DEFINED AURORA_USE_NAMESPACE)
    message(STATUS "AURORA_USE_NAMESPACE set. Adding to PCH.")
    target_precompile_headers(${PROJECT_NAME}
        PUBLIC
            ${PROJECT_SOURCE_DIR}/pch/namespace.hpp
    )
else()
    message(STATUS "AURORA_USE_NAMESPACE not set. Not using the namespace.")
endif()
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${AURORA_INCLUDE_DIR}
)