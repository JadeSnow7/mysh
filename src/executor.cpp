#include "executor.h"
#include "shell.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <signal.h>

Executor::Executor(Shell* shell) : shell(shell) {
    setupSignalHandlers();
}

Executor::~Executor() = default;

int Executor::execute(std::shared_ptr<Command> command) {
    if (!command) {
        return 1;
    }
    
    return executeExternal(command);
}

int Executor::executePipeline(std::shared_ptr<PipelineCommand> pipeline) {
    if (!pipeline || pipeline->commands.empty()) {
        return 1;
    }
    
    // 如果只有一个命令，直接执行
    if (pipeline->commands.size() == 1) {
        return execute(pipeline->commands[0]);
    }
    
    // 创建管道
    std::vector<int> pipes;
    int numCommands = pipeline->commands.size();
    
    // 创建 (n-1) 个管道
    for (int i = 0; i < numCommands - 1; ++i) {
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            return 1;
        }
        pipes.push_back(pipefd[0]); // read end
        pipes.push_back(pipefd[1]); // write end
    }
    
    std::vector<pid_t> pids;
    
    for (int i = 0; i < numCommands; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return 1;
        }
        
        if (pid == 0) {
            // 子进程
            
            // 设置输入重定向
            if (i > 0) {
                // 不是第一个命令，从前一个管道读取
                dup2(pipes[(i-1)*2], STDIN_FILENO);
            }
            
            // 设置输出重定向
            if (i < numCommands - 1) {
                // 不是最后一个命令，写入到下一个管道
                dup2(pipes[i*2+1], STDOUT_FILENO);
            }
            
            // 关闭所有管道描述符
            for (int fd : pipes) {
                close(fd);
            }
            
            // 执行命令
            auto argv = createArgv(pipeline->commands[i]);
            std::string executable = findExecutable(pipeline->commands[i]->command);
            if (executable.empty()) {
                std::cerr << "Command not found: " << pipeline->commands[i]->command << std::endl;
                exit(127);
            }
            
            execv(executable.c_str(), argv.data());
            perror("execv");
            exit(127);
        } else {
            // 父进程
            pids.push_back(pid);
        }
    }
    
    // 父进程关闭所有管道描述符
    for (int fd : pipes) {
        close(fd);
    }
    
    // 等待所有子进程
    int status = 0;
    for (pid_t pid : pids) {
        waitpid(pid, &status, 0);
    }
    
    return WEXITSTATUS(status);
}

int Executor::executeExternal(std::shared_ptr<Command> command) {
    // 查找可执行文件
    std::string executable = findExecutable(command->command);
    if (executable.empty()) {
        std::cerr << "Command not found: " << command->command << std::endl;
        return 127;
    }
    
    // 设置重定向
    int savedStdin = -1, savedStdout = -1;
    if (!setupRedirection(command, savedStdin, savedStdout)) {
        return 1;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        restoreRedirection(savedStdin, savedStdout);
        return 1;
    }
    
    if (pid == 0) {
        // 子进程
        auto argv = createArgv(command);
        execv(executable.c_str(), argv.data());
        perror("execv");
        exit(127);
    } else {
        // 父进程
        int status = waitForChild(pid, command->runInBackground);
        restoreRedirection(savedStdin, savedStdout);
        return status;
    }
}

bool Executor::setupRedirection(std::shared_ptr<Command> command, 
                               int& savedStdin, int& savedStdout) {
    // 输入重定向
    if (!command->inputRedirect.empty()) {
        savedStdin = dup(STDIN_FILENO);
        int fd = open(command->inputRedirect.c_str(), O_RDONLY);
        if (fd == -1) {
            perror("open input file");
            return false;
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    
    // 输出重定向
    if (!command->outputRedirect.empty()) {
        savedStdout = dup(STDOUT_FILENO);
        int flags = O_WRONLY | O_CREAT;
        if (command->appendOutput) {
            flags |= O_APPEND;
        } else {
            flags |= O_TRUNC;
        }
        
        int fd = open(command->outputRedirect.c_str(), flags, 0644);
        if (fd == -1) {
            perror("open output file");
            if (savedStdin != -1) {
                dup2(savedStdin, STDIN_FILENO);
                close(savedStdin);
            }
            return false;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    
    return true;
}

void Executor::restoreRedirection(int savedStdin, int savedStdout) {
    if (savedStdin != -1) {
        dup2(savedStdin, STDIN_FILENO);
        close(savedStdin);
    }
    
    if (savedStdout != -1) {
        dup2(savedStdout, STDOUT_FILENO);
        close(savedStdout);
    }
}

std::string Executor::findExecutable(const std::string& command) {
    // 如果命令包含路径分隔符，直接检查该路径
    if (command.find('/') != std::string::npos) {
        struct stat st;
        if (stat(command.c_str(), &st) == 0 && (st.st_mode & S_IXUSR)) {
            return command;
        }
        return "";
    }
    
    // 在PATH中搜索
    std::string path = shell->getEnvironmentVariable("PATH");
    if (path.empty()) {
        return "";
    }
    
    auto paths = splitPath(path);
    for (const auto& dir : paths) {
        std::string fullPath = dir + "/" + command;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0 && (st.st_mode & S_IXUSR)) {
            return fullPath;
        }
    }
    
    return "";
}

std::vector<char*> Executor::createArgv(std::shared_ptr<Command> command) {
    std::vector<char*> argv;
    
    // 添加命令名
    argv.push_back(const_cast<char*>(command->command.c_str()));
    
    // 添加参数
    for (auto& arg : command->arguments) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    
    // 添加NULL终止符
    argv.push_back(nullptr);
    
    return argv;
}

int Executor::waitForChild(pid_t pid, bool background) {
    if (background) {
        std::cout << "[Background] Process " << pid << " started" << std::endl;
        return 0;
    }
    
    int status;
    waitpid(pid, &status, 0);
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }
    
    return 1;
}

void Executor::setupSignalHandlers() {
    // 忽略SIGINT在父进程中，让子进程处理
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
}

std::vector<std::string> Executor::splitPath(const std::string& path) {
    std::vector<std::string> result;
    std::string current;
    
    for (char c : path) {
        if (c == ':') {
            if (!current.empty()) {
                result.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    
    if (!current.empty()) {
        result.push_back(current);
    }
    
    return result;
}