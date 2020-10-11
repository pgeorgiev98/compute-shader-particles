cmake_minimum_required(VERSION 3.11)

project(particles)

set(SOURCES main.cpp shaders.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

option(BUILD_GLFW "Manually build GLFW3" OFF)

if (BUILD_GLFW)
    set(GLFW_PATH "" CACHE PATH "Path to GLFW for manual building")
    if ("${GLFW_PATH}" STREQUAL "")
        message(FATAL_ERROR "GLFW_PATH must be set for GLFW manual building")
    endif()
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(${GLFW_PATH} glfw)
else()
    find_package(glfw3 3.3 REQUIRED)
endif()

find_package(OpenGL REQUIRED)

cmake_policy(SET CMP0074 NEW)
set(GLEW_USE_STATIC_LIBS 1)
find_package(GLEW REQUIRED)

target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} OpenGL::GL)
target_link_libraries(${PROJECT_NAME} GLEW::GLEW)
# For some reason this doesn't get included(on Windows at least) automatically
target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)
