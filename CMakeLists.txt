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
    src/core/completion.cpp
    src/core/syntax_highlighter.cpp
    src/core/input_handler.cpp
    src/core/ai_client.cpp
    src/core/ai_command.cpp
    src/platform/platform.cpp
)

<<<<<<< HEAD
# 平台特定源文件
if(WIN32)
    list(APPEND SOURCES 
        src/platform/windows/posix_compat.cpp
        src/core/executor_windows.cpp
    )
else()
    list(APPEND SOURCES src/core/executor.cpp)
endif()
=======
# 头文件
set(HEADERS
    src/core/shell.h
    src/core/parser.h
    src/core/executor.h
    src/core/builtin.h
    src/core/history.h
    src/core/completion.h
    src/core/syntax_highlighter.h
    src/core/input_handler.h
    src/platform/platform.h
)
>>>>>>> 38c84ec98f1a450d11385b24f37f175d985e805b

# 创建可执行文件
add_executable(mysh ${SOURCES})

<<<<<<< HEAD
# 简化版本
add_executable(mysh_simple src/main_windows.cpp)
=======
# 查找readline库
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(READLINE QUIET readline)
endif()

if(NOT READLINE_FOUND)
    # 尝试直接查找
    find_path(READLINE_INCLUDE_DIR readline/readline.h)
    find_library(READLINE_LIBRARY NAMES readline)
    
    if(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
        set(READLINE_FOUND TRUE)
        set(READLINE_INCLUDE_DIRS ${READLINE_INCLUDE_DIR})
        set(READLINE_LIBRARIES ${READLINE_LIBRARY})
    endif()
endif()

if(READLINE_FOUND)
    message(STATUS "Found GNU Readline")
    target_compile_definitions(mysh PRIVATE HAVE_READLINE)
    target_include_directories(mysh PRIVATE ${READLINE_INCLUDE_DIRS})
    target_link_libraries(mysh ${READLINE_LIBRARIES})
else()
    message(STATUS "GNU Readline not found - using simple input mode")
endif()

# 查找curl库
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(CURL QUIET libcurl)
endif()

if(NOT CURL_FOUND)
    # 尝试直接查找
    find_path(CURL_INCLUDE_DIR curl/curl.h)
    find_library(CURL_LIBRARY NAMES curl)
    
    if(CURL_INCLUDE_DIR AND CURL_LIBRARY)
        set(CURL_FOUND TRUE)
        set(CURL_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
        set(CURL_LIBRARIES ${CURL_LIBRARY})
    endif()
endif()

if(CURL_FOUND)
    message(STATUS "Found libcurl")
    target_compile_definitions(mysh PRIVATE HAVE_CURL)
    target_include_directories(mysh PRIVATE ${CURL_INCLUDE_DIRS})
    target_link_libraries(mysh ${CURL_LIBRARIES})
else()
    message(WARNING "libcurl not found - AI features will be disabled")
endif()

# 查找jsoncpp库
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(JSONCPP QUIET jsoncpp)
endif()

if(NOT JSONCPP_FOUND)
    # 尝试直接查找
    find_path(JSONCPP_INCLUDE_DIR json/json.h)
    find_library(JSONCPP_LIBRARY NAMES jsoncpp)
    
    if(JSONCPP_INCLUDE_DIR AND JSONCPP_LIBRARY)
        set(JSONCPP_FOUND TRUE)
        set(JSONCPP_INCLUDE_DIRS ${JSONCPP_INCLUDE_DIR})
        set(JSONCPP_LIBRARIES ${JSONCPP_LIBRARY})
    endif()
endif()

if(JSONCPP_FOUND)
    message(STATUS "Found jsoncpp")
    target_compile_definitions(mysh PRIVATE HAVE_JSONCPP)
    target_include_directories(mysh PRIVATE ${JSONCPP_INCLUDE_DIRS})
    target_link_libraries(mysh ${JSONCPP_LIBRARIES})
else()
    message(WARNING "jsoncpp not found - AI features will be disabled")
endif()

# 平台特定的链接库
if(WIN32)
    target_link_libraries(mysh shlwapi)
elseif(UNIX AND NOT APPLE)
    # Linux特定库
    find_package(Threads REQUIRED)
    target_link_libraries(mysh Threads::Threads)
elseif(APPLE)
    # macOS特定库
    find_package(Threads REQUIRED)
    target_link_libraries(mysh Threads::Threads)
endif()
>>>>>>> 38c84ec98f1a450d11385b24f37f175d985e805b

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