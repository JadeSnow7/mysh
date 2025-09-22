#ifndef PLATFORM_H
#define PLATFORM_H

#include <string>
#include <vector>
#include <memory>

// 平台检测宏
#ifdef _WIN32
    #define PLATFORM_WINDOWS
#elif defined(__APPLE__)
    #define PLATFORM_MACOS
#elif defined(__linux__)
    #define PLATFORM_LINUX
#else
    #define PLATFORM_UNKNOWN
#endif

// 平台特定包含
#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    #include <process.h>
    #include <io.h>
    #include <direct.h>
    #define PATH_SEPARATOR '\\'
    #define PATH_DELIMITER ';'
#else
    #include <unistd.h>
    #include <sys/wait.h>
    #include <sys/stat.h>
    #include <pwd.h>
    #include <signal.h>
    #define PATH_SEPARATOR '/'
    #define PATH_DELIMITER ':'
#endif

// 跨平台类型定义
#ifdef PLATFORM_WINDOWS
    typedef DWORD ProcessId;
    typedef HANDLE FileDescriptor;
    typedef HANDLE ProcessHandle;
#else
    typedef pid_t ProcessId;
    typedef int FileDescriptor;
    typedef pid_t ProcessHandle;
#endif

// 跨平台接口类
class PlatformInterface {
public:
    virtual ~PlatformInterface() = default;
    
    // 进程管理
    virtual ProcessId createProcess(const std::string& executable, 
                                   const std::vector<std::string>& args) = 0;
    virtual int waitForProcess(ProcessHandle handle, bool background = false) = 0;
    virtual bool killProcess(ProcessHandle handle) = 0;
    
    // 文件系统
    virtual bool changeDirectory(const std::string& path) = 0;
    virtual std::string getCurrentDirectory() = 0;
    virtual bool fileExists(const std::string& path) = 0;
    virtual bool isExecutable(const std::string& path) = 0;
    
    // 重定向和管道
    virtual bool setupRedirection(const std::string& inputFile, 
                                 const std::string& outputFile, 
                                 bool appendOutput) = 0;
    virtual std::vector<FileDescriptor> createPipe() = 0;
    
    // 环境变量
    virtual std::string getEnvironmentVariable(const std::string& name) = 0;
    virtual void setEnvironmentVariable(const std::string& name, 
                                       const std::string& value) = 0;
    virtual void unsetEnvironmentVariable(const std::string& name) = 0;
    
    // 用户信息
    virtual std::string getUserName() = 0;
    virtual std::string getHomeDirectory() = 0;
    
    // 路径处理
    virtual std::vector<std::string> splitPath(const std::string& path) = 0;
    virtual std::string findExecutable(const std::string& command) = 0;
    
    // 信号处理
    virtual void setupSignalHandlers() = 0;
};

// 工厂函数
std::unique_ptr<PlatformInterface> createPlatformInterface();

#endif // PLATFORM_H