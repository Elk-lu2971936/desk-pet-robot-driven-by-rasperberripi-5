# CMake最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(whisper)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)

# 查找所有的 .cpp 文件并将它们添加到源文件列表中
file(GLOB SOURCES "src/*.cpp")

# 设置头文件搜索路径
include_directories(/usr/include/x86_64-linux-gnu/)

# 设置库文件搜索路径
link_directories(/usr/lib/x86_64-linux-gnu/)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接 .so 文件
target_link_libraries(${PROJECT_NAME} curl)

