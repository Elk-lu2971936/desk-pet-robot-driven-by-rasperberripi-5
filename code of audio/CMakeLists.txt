# CMake Minimum version requirements
cmake_minimum_required(VERSION 3.10)

# Project name
project(whisper)

# Setting C++ standards
set(CMAKE_CXX_STANDARD 11)

# Find all the.cpp files and add them to the source file list
file(GLOB SOURCES "audio-initialize/*.cpp")

# Set the search path for header files
include_directories(/usr/include/x86_64-linux-gnu/)

# Set the search path for library files
link_directories(/usr/lib/x86_64-linux-gnu/)

# Create an executable file
add_executable(${PROJECT_NAME} ${SOURCES})

# Link to the.so file
target_link_libraries(${PROJECT_NAME} curl)

