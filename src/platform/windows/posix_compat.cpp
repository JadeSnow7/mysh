#ifdef _WIN32

#include "posix_compat.h"
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cstring>
#include <cstdlib>

// 全局变量存储子进程信息
static std::map<pid_t, HANDLE> g_processes;
static pid_t g_next_pid = 1000;

// fork实现 - 在Windows上创建新进程
pid_t fork(void) {
    // Windows不支持真正的fork，这里返回错误
    // 实际应用中需要重构代码使用CreateProcess
    return -1;
}

// waitpid实现
int waitpid(pid_t pid, int* status, int options) {
    auto it = g_processes.find(pid);
    if (it == g_processes.end()) {
        return -1;
    }
    
    HANDLE hProcess = it->second;
    DWORD result = WaitForSingleObject(hProcess, INFINITE);
    
    if (result == WAIT_OBJECT_0) {
        DWORD exitCode;
        GetExitCodeProcess(hProcess, &exitCode);
        if (status) {
            *status = static_cast<int>(exitCode);
        }
        CloseHandle(hProcess);
        g_processes.erase(it);
        return pid;
    }
    
    return -1;
}

// execvp实现
int execvp(const char* file, char* const argv[]) {
    std::string cmdline = file;
    
    // 构建命令行
    for (int i = 1; argv[i] != nullptr; ++i) {
        cmdline += " ";
        cmdline += argv[i];
    }
    
    // 使用system调用执行（简化实现）
    int result = system(cmdline.c_str());
    exit(result); // execvp不应该返回
    return -1;
}

// pipe实现
int pipe(int pipefd[2]) {
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa;
    
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        return -1;
    }
    
    pipefd[0] = _open_osfhandle(reinterpret_cast<intptr_t>(hRead), _O_RDONLY);
    pipefd[1] = _open_osfhandle(reinterpret_cast<intptr_t>(hWrite), _O_WRONLY);
    
    if (pipefd[0] == -1 || pipefd[1] == -1) {
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return -1;
    }
    
    return 0;
}

// dup2实现
int dup2(int oldfd, int newfd) {
    return _dup2(oldfd, newfd);
}

// signal实现
sighandler_t signal(int signum, sighandler_t handler) {
    // Windows信号处理简化实现
    switch (signum) {
        case SIGINT:
            // 设置Ctrl+C处理
            if (handler == SIG_IGN) {
                SetConsoleCtrlHandler(NULL, TRUE);
            } else {
                SetConsoleCtrlHandler(NULL, FALSE);
            }
            break;
        default:
            break;
    }
    return handler;
}

// 用户信息相关
static struct passwd g_passwd = {0};

struct passwd* getpwuid(int uid) {
    (void)uid; // 忽略参数
    
    static char username[256];
    static char homedir[MAX_PATH];
    
    DWORD size = sizeof(username);
    if (!GetUserNameA(username, &size)) {
        strcpy_s(username, sizeof(username), "user");
    }
    
    if (!GetEnvironmentVariableA("USERPROFILE", homedir, sizeof(homedir))) {
        strcpy_s(homedir, sizeof(homedir), "C:\\Users\\Default");
    }
    
    g_passwd.pw_name = username;
    g_passwd.pw_dir = homedir;
    
    return &g_passwd;
}

int getuid(void) {
    return 1000; // 返回固定值
}

// 环境变量操作
int setenv(const char* name, const char* value, int overwrite) {
    if (!overwrite && getenv(name) != nullptr) {
        return 0;
    }
    
    std::string env_str = std::string(name) + "=" + std::string(value);
    return _putenv(env_str.c_str());
}

int unsetenv(const char* name) {
    std::string env_str = std::string(name) + "=";
    return _putenv(env_str.c_str());
}

// 目录操作
char* getcwd(char* buf, size_t size) {
    return _getcwd(buf, static_cast<int>(size));
}

int chdir(const char* path) {
    return _chdir(path);
}

// Windows特定的进程创建函数
pid_t create_process_windows(const std::string& executable, 
                           const std::vector<std::string>& args) {
    std::string cmdline = executable;
    for (const auto& arg : args) {
        cmdline += " \"" + arg + "\"";
    }
    
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    
    // 创建进程
    if (!CreateProcessA(
        NULL,                           // 应用程序名
        const_cast<char*>(cmdline.c_str()), // 命令行
        NULL,                           // 进程安全属性
        NULL,                           // 线程安全属性
        TRUE,                           // 继承句柄
        0,                              // 创建标志
        NULL,                           // 环境
        NULL,                           // 当前目录
        &si,                            // 启动信息
        &pi                             // 进程信息
    )) {
        return -1;
    }
    
    CloseHandle(pi.hThread);
    
    pid_t pid = g_next_pid++;
    g_processes[pid] = pi.hProcess;
    
    return pid;
}

#endif // _WIN32