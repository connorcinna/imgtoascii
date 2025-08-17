cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
project(imgtoascii)
find_package(PNG REQUIRED)

add_executable(imgtoascii
src/imgtoascii.cpp)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architecture")
target_include_directories(imgtoascii
	PUBLIC /opt/homebrew/include)
target_link_libraries(imgtoascii
	PRIVATE PNG::PNG)
