#include "../platform.h"

#ifdef PLATFORM_LINUX

#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <algorithm>

class LinuxPlatform : public PlatformInterface {
public:
    ProcessId createProcess(const std::string& executable, 
                           const std::vector<std::string>& args) override {
        pid_t pid = fork();
        if (pid == 0) {
            // 子进程
            std::vector<char*> argv;
            argv.push_back(const_cast<char*>(executable.c_str()));
            for (const auto& arg : args) {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }
            argv.push_back(nullptr);
            
            execv(executable.c_str(), argv.data());
            perror("execv");
            exit(127);
        }
        return pid;
    }
    
    int waitForProcess(ProcessHandle handle, bool background) override {
        if (background) {
            std::cout << "[Background] Process " << handle << " started" << std::endl;
            return 0;
        }
        
        int status;
        waitpid(handle, &status, 0);
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            return 128 + WTERMSIG(status);
        }
        
        return 1;
    }
    
    bool killProcess(ProcessHandle handle) override {
        return kill(handle, SIGTERM) == 0;
    }
    
    bool changeDirectory(const std::string& path) override {
        return chdir(path.c_str()) == 0;
    }
    
    std::string getCurrentDirectory() override {
        char* cwd = getcwd(nullptr, 0);
        if (cwd) {
            std::string result(cwd);
            free(cwd);
            return result;
        }
        return "";
    }
    
    bool fileExists(const std::string& path) override {
        struct stat st;
        return stat(path.c_str(), &st) == 0;
    }
    
    bool isExecutable(const std::string& path) override {
        struct stat st;
        return stat(path.c_str(), &st) == 0 && (st.st_mode & S_IXUSR);
    }
    
    bool setupRedirection(const std::string& inputFile, 
                         const std::string& outputFile, 
                         bool appendOutput) override {
        // 实现重定向逻辑（简化版）
        if (!inputFile.empty()) {
            int fd = open(inputFile.c_str(), O_RDONLY);
            if (fd == -1) return false;
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        
        if (!outputFile.empty()) {
            int flags = O_WRONLY | O_CREAT;
            if (appendOutput) flags |= O_APPEND;
            else flags |= O_TRUNC;
            
            int fd = open(outputFile.c_str(), flags, 0644);
            if (fd == -1) return false;
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        
        return true;
    }
    
    std::vector<FileDescriptor> createPipe() override {
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            return {};
        }
        return {pipefd[0], pipefd[1]};
    }
    
    std::string getEnvironmentVariable(const std::string& name) override {
        char* value = getenv(name.c_str());
        return value ? std::string(value) : "";
    }
    
    void setEnvironmentVariable(const std::string& name, 
                               const std::string& value) override {
        setenv(name.c_str(), value.c_str(), 1);
    }
    
    void unsetEnvironmentVariable(const std::string& name) override {
        unsetenv(name.c_str());
    }
    
    std::string getUserName() override {
        char* user = getenv("USER");
        if (user) return std::string(user);
        
        struct passwd* pw = getpwuid(getuid());
        return pw ? std::string(pw->pw_name) : "unknown";
    }
    
    std::string getHomeDirectory() override {
        char* home = getenv("HOME");
        if (home) return std::string(home);
        
        struct passwd* pw = getpwuid(getuid());
        return pw ? std::string(pw->pw_dir) : "";
    }
    
    std::vector<std::string> splitPath(const std::string& path) override {
        std::vector<std::string> result;
        std::istringstream iss(path);
        std::string dir;
        
        while (std::getline(iss, dir, PATH_DELIMITER)) {
            if (!dir.empty()) {
                result.push_back(dir);
            }
        }
        
        return result;
    }
    
    std::string findExecutable(const std::string& command) override {
        // 如果包含路径分隔符，直接检查
        if (command.find(PATH_SEPARATOR) != std::string::npos) {
            return isExecutable(command) ? command : "";
        }
        
        // 在PATH中搜索
        std::string path = getEnvironmentVariable("PATH");
        auto paths = splitPath(path);
        
        for (const auto& dir : paths) {
            std::string fullPath = dir + PATH_SEPARATOR + command;
            if (isExecutable(fullPath)) {
                return fullPath;
            }
        }
        
        return "";
    }
    
    void setupSignalHandlers() override {
        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
    }
};

#endif // PLATFORM_LINUX