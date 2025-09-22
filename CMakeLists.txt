cmake_minimum_required(VERSION 3.15)
project(mysh VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
if(MSVC)
    add_compile_options(/W4)
    # 禁用一些Windows特定的警告
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 平台检测
if(WIN32)
    add_definitions(-DPLATFORM_WINDOWS)
elseif(APPLE)
    add_definitions(-DPLATFORM_MACOS)
elseif(UNIX)
    add_definitions(-DPLATFORM_LINUX)
endif()

# 添加源文件目录到包含路径
include_directories(src)
include_directories(src/core)
include_directories(src/platform)
include_directories(src/platform/windows)

# 源文件
set(SOURCES
    src/main.cpp
    src/core/shell.cpp
    src/core/parser.cpp
    src/core/builtin.cpp
    src/core/history.cpp
    src/platform/platform.cpp
)

# 平台特定源文件
if(WIN32)
    list(APPEND SOURCES 
        src/platform/windows/posix_compat.cpp
        src/core/executor_windows.cpp
    )
else()
    list(APPEND SOURCES src/core/executor.cpp)
endif()

# 创建可执行文件
add_executable(mysh ${SOURCES})

# 简化版本
add_executable(mysh_simple src/main_windows.cpp)

# 设置输出目录
set_target_properties(mysh PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(mysh_simple PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Windows特定的链接库
if(WIN32)
    target_link_libraries(mysh shlwapi)
    target_link_libraries(mysh_simple shlwapi)
endif()