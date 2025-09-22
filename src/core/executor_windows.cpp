#ifdef PLATFORM_WINDOWS

#include "executor.h"
#include "shell.h"
#include "posix_compat.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sstream>

// Windows特定的executor实现
int Executor::executePipeline(const std::vector<std::shared_ptr<Command>>& commands) {
    if (commands.empty()) return 0;
    
    // 在Windows上，我们使用简化的管道实现
    // 对于单个命令，直接执行
    if (commands.size() == 1) {
        return executeCommand(commands[0]);
    }
    
    // 对于多个命令的管道，我们使用临时文件
    std::string tempFile = "temp_pipe_" + std::to_string(GetCurrentProcessId()) + ".tmp";
    
    for (size_t i = 0; i < commands.size(); ++i) {
        auto command = commands[i];
        
        // 构建命令行
        std::string cmdline = command->command;
        for (const auto& arg : command->arguments) {
            cmdline += " " + arg;
        }
        
        // 设置重定向
        if (i > 0) {
            // 不是第一个命令，从临时文件读取
            cmdline += " < " + tempFile;
        }
        
        if (i < commands.size() - 1) {
            // 不是最后一个命令，输出到临时文件
            cmdline += " > " + tempFile + "_next";
        }
        
        // 执行命令
        int result = system(cmdline.c_str());
        
        // 更新临时文件
        if (i < commands.size() - 1) {
            remove(tempFile.c_str());
            rename((tempFile + "_next").c_str(), tempFile.c_str());
        }
        
        if (result != 0 && i < commands.size() - 1) {
            // 中间命令失败，清理并返回
            remove(tempFile.c_str());
            return result;
        }
    }
    
    // 清理临时文件
    remove(tempFile.c_str());
    return 0;
}

int Executor::executeCommand(std::shared_ptr<Command> command) {
    if (!command) return 1;
    
    // 构建命令行
    std::string cmdline = command->command;
    for (const auto& arg : command->arguments) {
        cmdline += " \"" + arg + "\"";
    }
    
    // 处理重定向
    if (!command->inputRedirect.empty()) {
        cmdline += " < \"" + command->inputRedirect + "\"";
    }
    
    if (!command->outputRedirect.empty()) {
        if (command->appendOutput) {
            cmdline += " >> \"" + command->outputRedirect + "\"";
        } else {
            cmdline += " > \"" + command->outputRedirect + "\"";
        }
    }
    
    // 后台执行
    if (command->runInBackground) {
        cmdline = "start /B " + cmdline;
    }
    
    // 执行命令
    return system(cmdline.c_str());
}

std::string Executor::findExecutable(const std::string& command) {
    // 检查是否是绝对路径
    if (command.find('\\') != std::string::npos || command.find('/') != std::string::npos) {
        return command;
    }
    
    // 添加.exe扩展名（如果没有）
    std::string exe_command = command;
    if (exe_command.length() < 4 || 
        exe_command.substr(exe_command.length() - 4) != ".exe") {
        exe_command += ".exe";
    }
    
    // 在PATH中查找
    char* path_env = getenv("PATH");
    if (!path_env) return command;
    
    std::string path_str(path_env);
    std::istringstream path_stream(path_str);
    std::string path_dir;
    
    while (std::getline(path_stream, path_dir, ';')) {
        if (path_dir.empty()) continue;
        
        std::string full_path = path_dir;
        if (full_path.back() != '\\' && full_path.back() != '/') {
            full_path += '\\';
        }
        full_path += exe_command;
        
        // 检查文件是否存在
        if (_access(full_path.c_str(), 0) == 0) {
            return full_path;
        }
    }
    
    return command;
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

int Executor::waitForChild(ProcessHandle handle, bool background) {
    if (background) {
        std::cout << "[Background] Process " << handle << " started" << std::endl;
        return 0;
    }
    
    // 在Windows上，handle实际上是进程ID
    HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, 
                                 FALSE, handle);
    if (hProcess) {
        WaitForSingleObject(hProcess, INFINITE);
        
        DWORD exitCode;
        GetExitCodeProcess(hProcess, &exitCode);
        CloseHandle(hProcess);
        
        return static_cast<int>(exitCode);
    }
    
    return 1;
}

void Executor::setupSignalHandlers() {
    // Windows信号处理
    signal(SIGINT, SIG_IGN);
    
    // 设置控制台Ctrl+C处理
    SetConsoleCtrlHandler([](DWORD dwCtrlType) -> BOOL {
        switch (dwCtrlType) {
            case CTRL_C_EVENT:
                // 忽略Ctrl+C，让子进程处理
                return TRUE;
            default:
                return FALSE;
        }
    }, TRUE);
}

std::vector<std::string> Executor::splitPath(const std::string& path) {
    std::vector<std::string> paths;
    std::istringstream stream(path);
    std::string item;
    
    while (std::getline(stream, item, ';')) {
        if (!item.empty()) {
            paths.push_back(item);
        }
    }
    
    return paths;
}

#endif // PLATFORM_WINDOWS