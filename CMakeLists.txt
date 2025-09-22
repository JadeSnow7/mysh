cmake_minimum_required(VERSION 3.15)
project(mysh VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
if(MSVC)
    add_compile_options(/W4)
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

# 源文件
set(SOURCES
    src/main.cpp
    src/core/shell.cpp
    src/core/parser.cpp
    src/core/executor.cpp
    src/core/builtin.cpp
    src/core/history.cpp
    src/core/completion.cpp
    src/core/syntax_highlighter.cpp
    src/core/input_handler.cpp
    src/platform/platform.cpp
)

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

# 创建可执行文件
add_executable(mysh ${SOURCES} ${HEADERS})

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

# 设置输出目录
if(WIN32)
    # Windows下Visual Studio生成器会自动创建配置目录
    set_target_properties(mysh PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
else()
    # Linux和macOS使用单一输出目录
    set_target_properties(mysh PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

# 安装配置
install(TARGETS mysh
    RUNTIME DESTINATION bin
)

# 调试配置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(mysh PRIVATE DEBUG)
    if(NOT MSVC)
        target_compile_options(mysh PRIVATE -g)
    endif()
endif()

# 测试配置
enable_testing()

# 添加基本测试
add_test(NAME basic_test
    COMMAND mysh --help
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# CPack配置（打包）
include(CPack)
set(CPACK_PACKAGE_NAME "MyShell")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Cross-platform shell implementation")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
elseif(APPLE)
    set(CPACK_GENERATOR "ZIP;DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
endif()