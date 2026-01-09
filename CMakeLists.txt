cmake_minimum_required(VERSION 3.25)
project(conan_clang_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(imgui REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(nlohmann_json REQUIRED)

set(GLFW_TARGET glfw)
if (TARGET glfw::glfw)
    set(GLFW_TARGET glfw::glfw)
elseif (TARGET glfw3::glfw)
    set(GLFW_TARGET glfw3::glfw)
endif()

add_subdirectory(src/app)
add_subdirectory(plugins/json_formatter)
