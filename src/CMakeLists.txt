# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.5)

# 项目信息
project (agumi)

# 查找目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)

set(CMAKE_CXX_STANDARD 17)
add_definitions(-DSION_DISABLE_SSL)
set(CMAKE_BUILD_TYPE "Debug")
add_executable(agumi ${DIR_SRCS})
