cmake_minimum_required(VERSION 3.10)

project(cpuInfo)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加头文件目录
include_directories(inc)

# 生成可执行文件
add_executable(${PROJECT_NAME} src/test.cpp src/cpu-info.cpp)

