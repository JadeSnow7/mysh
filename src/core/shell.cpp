#include "shell.h"
#include "parser.h"
#include "executor.h"
#include "builtin.h"
#include "history.h"
#include "input_handler.h"
#include "syntax_highlighter.h"
#include "completion.h"
#include <iostream>
#include <cstdlib>

#ifdef PLATFORM_WINDOWS
#include "posix_compat.h"
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#endif

Shell::Shell() : shouldExit(false) {
    initialize();
}

Shell::~Shell() = default;

void Shell::initialize() {
    // 初始化各个组件
    parser = std::make_unique<Parser>();
    executor = std::make_unique<Executor>(this);
    builtinCommands = std::make_unique<BuiltinCommands>(this);
    history = std::make_unique<History>();
    
    // 重新启用InputHandler
    inputHandler = std::make_unique<InputHandler>(this);
    inputHandler->initialize();
    
    // 获取当前工作目录
    char* cwd = getcwd(nullptr, 0);
    if (cwd) {
        currentDirectory = std::string(cwd);
        free(cwd);
    }
    
    // 初始化一些基本环境变量
    char* home = getenv("HOME");
    if (home) {
        environmentVariables["HOME"] = std::string(home);
    }
    
    char* path = getenv("PATH");
    if (path) {
        environmentVariables["PATH"] = std::string(path);
    }
    
    char* user = getenv("USER");
    if (user) {
        environmentVariables["USER"] = std::string(user);
    } else {
        // 如果没有USER环境变量，尝试从passwd获取
        struct passwd* pw = getpwuid(getuid());
        if (pw) {
            environmentVariables["USER"] = std::string(pw->pw_name);
        }
    }
}

int Shell::run() {
    showWelcome();
    
    while (!shouldExit) {
        try {
            // 显示提示符并读取输入
            std::string input = readInput();
            
            // 跳过空输入
            if (input.empty()) {
                continue;
            }
            
            // 显示语法高亮版本（如果启用）
            showInputPrompt(input);
            
            // 添加到历史记录
            history->addCommand(input);
            if (inputHandler) {
                inputHandler->addHistory(input);
            }
            
            // 执行命令
            executeCommand(input);
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    return 0;
}

int Shell::executeCommand(const std::string& command) {
    // 解析命令
    auto parsedCommand = parser->parse(command);
    if (!parsedCommand) {
        return 1;
    }
    
    // 检查是否是内置命令
    if (builtinCommands->isBuiltinCommand(parsedCommand->command)) {
        return builtinCommands->execute(parsedCommand);
    }
    
    // 执行外部命令
    return executor->execute(parsedCommand);
}

std::string Shell::getEnvironmentVariable(const std::string& name) {
    auto it = environmentVariables.find(name);
    if (it != environmentVariables.end()) {
        return it->second;
    }
    
    // 如果在内部环境变量中找不到，尝试系统环境变量
    char* value = getenv(name.c_str());
    if (value) {
        return std::string(value);
    }
    
    return "";
}

void Shell::setEnvironmentVariable(const std::string& name, const std::string& value) {
    environmentVariables[name] = value;
    setenv(name.c_str(), value.c_str(), 1);
}

std::string Shell::getCurrentDirectory() {
    char* cwd = getcwd(nullptr, 0);
    if (cwd) {
        std::string result(cwd);
        free(cwd);
        return result;
    }
    return currentDirectory;
}

std::string Shell::getPrompt() {
    std::string user = getEnvironmentVariable("USER");
    std::string cwd = getCurrentDirectory();
    
    // 简化路径显示（如果在HOME目录下，显示~）
    std::string home = getEnvironmentVariable("HOME");
    if (!home.empty() && cwd.find(home) == 0) {
        if (cwd == home) {
            cwd = "~";
        } else {
            cwd = "~" + cwd.substr(home.length());
        }
    }
    
    return user + "@mysh:" + cwd + "$ ";
}

std::string Shell::readInput() {
    if (inputHandler) {
        return inputHandler->readLine(getPrompt());
    }
    
    // 备用方案：简单输入
    std::cout << getPrompt();
    std::string input;
    std::getline(std::cin, input);
    
    // 检查EOF（Ctrl+D）
    if (std::cin.eof()) {
        shouldExit = true;
        std::cout << std::endl;
        return "";
    }
    
    return input;
}

void Shell::showWelcome() {
    std::cout << "==================================" << std::endl;
    std::cout << "    欢迎使用 MyShell v1.0" << std::endl;
    std::cout << "    输入 'help' 查看可用命令" << std::endl;
    std::cout << "    输入 'exit' 退出shell" << std::endl;
    std::cout << "==================================" << std::endl;
}

void Shell::showInputPrompt(const std::string& command) {
    // 如果启用了语法高亮，显示高亮版本
    if (inputHandler && inputHandler->isSyntaxHighlightEnabled() && !command.empty()) {
        // 获取高亮版本的命令
        auto highlighter = inputHandler->getSyntaxHighlighter();
        if (highlighter) {
            std::string highlighted = highlighter->highlight(command);
            // 在简单模式下显示高亮版本作为反馈
            // 注意：我们需要确保这不会干扰实际的命令执行
            std::cout << "\r" << getPrompt() << highlighted << std::endl;
        }
    }
}

void Shell::setCompletionEnabled(bool enabled) {
    if (inputHandler) {
        inputHandler->setCompletionEnabled(enabled);
    }
}

void Shell::setSyntaxHighlightEnabled(bool enabled) {
    if (inputHandler) {
        inputHandler->setSyntaxHighlightEnabled(enabled);
    }
}

bool Shell::isCompletionEnabled() const {
    return inputHandler ? inputHandler->isCompletionEnabled() : false;
}

bool Shell::isSyntaxHighlightEnabled() const {
    return inputHandler ? inputHandler->isSyntaxHighlightEnabled() : false;
}