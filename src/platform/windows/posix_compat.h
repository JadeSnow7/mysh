#ifndef POSIX_COMPAT_H
#define POSIX_COMPAT_H

#ifdef _WIN32

#include <windows.h>
#include <process.h>
#include <io.h>
#include <direct.h>
#include <sys/stat.h>
#include <fcntl.h>

// POSIX类型定义
typedef int pid_t;
typedef HANDLE PIPE_HANDLE;

// 文件描述符常量
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// 文件权限常量
#define S_IXUSR _S_IEXEC
#define S_IRUSR _S_IREAD
#define S_IWUSR _S_IWRITE

// 文件打开标志
#ifndef O_RDONLY
#define O_RDONLY _O_RDONLY
#endif
#ifndef O_WRONLY
#define O_WRONLY _O_WRONLY
#endif
#ifndef O_CREAT
#define O_CREAT _O_CREAT
#endif
#ifndef O_TRUNC
#define O_TRUNC _O_TRUNC
#endif
#ifndef O_APPEND
#define O_APPEND _O_APPEND
#endif

// 进程等待状态宏
#define WEXITSTATUS(status) (status)
#define WIFEXITED(status) (1)

// 信号常量
#define SIGINT  2
#define SIGQUIT 3
#define SIG_IGN ((void(*)(int))1)

// 函数声明
#ifdef __cplusplus
extern "C" {
#endif

// 进程管理 - 使用不同的名称避免冲突
pid_t posix_fork(void);
int posix_waitpid(pid_t pid, int* status, int options);
int posix_execvp(const char* file, char* const argv[]);

// 管道
int posix_pipe(int pipefd[2]);

// 信号处理
typedef void (*posix_sighandler_t)(int);
posix_sighandler_t posix_signal(int signum, posix_sighandler_t handler);

// 用户信息
struct posix_passwd {
    char* pw_name;
    char* pw_dir;
    // 简化版本，只包含必要字段
};

struct posix_passwd* posix_getpwuid(int uid);
int posix_getuid(void);

// 环境变量
int posix_setenv(const char* name, const char* value, int overwrite);
int posix_unsetenv(const char* name);

#ifdef __cplusplus
}
#endif

#endif // _WIN32

#endif // POSIX_COMPAT_H