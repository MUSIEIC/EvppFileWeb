# CMake 最低版本要求
 cmake_minimum_required(VERSION 3.10)

# 项目名称
project(EvppWeb)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# 添加可执行文件
add_executable(EvppWeb src/main.cpp)
target_link_directories(EvppWeb PRIVATE /usr/local/lib)
target_link_libraries(EvppWeb PRIVATE evpp glog)
