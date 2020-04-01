cmake_minimum_required(VERSION 3.11)
project(particles)
add_executable(particles main.cpp shaders.cpp)
set_property(TARGET particles PROPERTY CXX_STANDARD 17)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

target_link_libraries(particles glfw)
target_link_libraries(particles OpenGL::OpenGL)
target_link_libraries(particles GLEW::GLEW)
