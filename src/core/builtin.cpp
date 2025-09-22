#include "builtin.h"
#include "shell.h"
#include "history.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <unistd.h>
#include <cstdlib>
#include <sys/stat.h>

BuiltinCommands::BuiltinCommands(Shell* shell) : shell(shell) {
    initializeBuiltins();
}

BuiltinCommands::~BuiltinCommands() = default;

void BuiltinCommands::initializeBuiltins() {
    builtinMap["exit"] = [this](std::shared_ptr<Command> cmd) { return cmdExit(cmd); };
    builtinMap["help"] = [this](std::shared_ptr<Command> cmd) { return cmdHelp(cmd); };
    builtinMap["pwd"] = [this](std::shared_ptr<Command> cmd) { return cmdPwd(cmd); };
    builtinMap["cd"] = [this](std::shared_ptr<Command> cmd) { return cmdCd(cmd); };
    builtinMap["echo"] = [this](std::shared_ptr<Command> cmd) { return cmdEcho(cmd); };
    builtinMap["export"] = [this](std::shared_ptr<Command> cmd) { return cmdExport(cmd); };
    builtinMap["env"] = [this](std::shared_ptr<Command> cmd) { return cmdEnv(cmd); };
    builtinMap["unset"] = [this](std::shared_ptr<Command> cmd) { return cmdUnset(cmd); };
    builtinMap["history"] = [this](std::shared_ptr<Command> cmd) { return cmdHistory(cmd); };
    builtinMap["clear"] = [this](std::shared_ptr<Command> cmd) { return cmdClear(cmd); };
    builtinMap["which"] = [this](std::shared_ptr<Command> cmd) { return cmdWhich(cmd); };
    builtinMap["set"] = [this](std::shared_ptr<Command> cmd) { return cmdSet(cmd); };
}

bool BuiltinCommands::isBuiltinCommand(const std::string& command) {
    return builtinMap.find(command) != builtinMap.end();
}

int BuiltinCommands::execute(std::shared_ptr<Command> command) {
    auto it = builtinMap.find(command->command);
    if (it != builtinMap.end()) {
        return it->second(command);
    }
    return 1;
}

int BuiltinCommands::cmdExit(std::shared_ptr<Command> command) {
    int exitCode = 0;
    
    if (!command->arguments.empty()) {
        try {
            exitCode = std::stoi(command->arguments[0]);
        } catch (const std::exception&) {
            std::cerr << "exit: invalid exit code" << std::endl;
            return 1;
        }
    }
    
    std::cout << "Goodbye!" << std::endl;
    shell->setExitFlag(true);
    return exitCode;
}

int BuiltinCommands::cmdHelp(std::shared_ptr<Command> command) {
    printHelp();
    return 0;
}

int BuiltinCommands::cmdPwd(std::shared_ptr<Command> command) {
    std::cout << shell->getCurrentDirectory() << std::endl;
    return 0;
}

int BuiltinCommands::cmdCd(std::shared_ptr<Command> command) {
    std::string path;
    
    if (command->arguments.empty()) {
        // 没有参数，切换到HOME目录
        path = shell->getEnvironmentVariable("HOME");
        if (path.empty()) {
            std::cerr << "cd: HOME not set" << std::endl;
            return 1;
        }
    } else {
        path = command->arguments[0];
        
        // 处理 ~ 符号
        if (path == "~" || path.substr(0, 2) == "~/") {
            std::string home = shell->getEnvironmentVariable("HOME");
            if (home.empty()) {
                std::cerr << "cd: HOME not set" << std::endl;
                return 1;
            }
            if (path == "~") {
                path = home;
            } else {
                path = home + path.substr(1);
            }
        }
    }
    
    return changeDirectory(path) ? 0 : 1;
}

int BuiltinCommands::cmdEcho(std::shared_ptr<Command> command) {
    bool newline = true;
    size_t start = 0;
    
    // 检查 -n 选项
    if (!command->arguments.empty() && command->arguments[0] == "-n") {
        newline = false;
        start = 1;
    }
    
    for (size_t i = start; i < command->arguments.size(); ++i) {
        if (i > start) {
            std::cout << " ";
        }
        std::cout << command->arguments[i];
    }
    
    if (newline) {
        std::cout << std::endl;
    }
    
    return 0;
}

int BuiltinCommands::cmdExport(std::shared_ptr<Command> command) {
    if (command->arguments.empty()) {
        // 显示所有导出的变量
        return cmdEnv(command);
    }
    
    for (const auto& arg : command->arguments) {
        size_t pos = arg.find('=');
        if (pos != std::string::npos) {
            std::string name = arg.substr(0, pos);
            std::string value = arg.substr(pos + 1);
            shell->setEnvironmentVariable(name, value);
        } else {
            // 只有变量名，设置为空值
            shell->setEnvironmentVariable(arg, "");
        }
    }
    
    return 0;
}

int BuiltinCommands::cmdEnv(std::shared_ptr<Command> command) {
    extern char **environ;
    
    for (char **env = environ; *env != nullptr; ++env) {
        std::cout << *env << std::endl;
    }
    
    return 0;
}

int BuiltinCommands::cmdUnset(std::shared_ptr<Command> command) {
    if (command->arguments.empty()) {
        std::cerr << "unset: missing variable name" << std::endl;
        return 1;
    }
    
    for (const auto& var : command->arguments) {
        unsetenv(var.c_str());
    }
    
    return 0;
}

int BuiltinCommands::cmdHistory(std::shared_ptr<Command> command) {
    History* hist = shell->getHistory();
    if (!hist) {
        std::cerr << "History not available" << std::endl;
        return 1;
    }
    
    if (command->arguments.empty()) {
        // 显示所有历史记录
        hist->show();
    } else {
        std::string subcommand = command->arguments[0];
        
        if (subcommand == "-c" || subcommand == "clear") {
            // 清空历史记录
            hist->clear();
            std::cout << "History cleared." << std::endl;
        } else if (subcommand.substr(0, 1) == "-" && subcommand.length() > 1) {
            // 显示最后 n 条命令
            try {
                int n = std::stoi(subcommand.substr(1));
                auto history = hist->getHistory();
                size_t start = history.size() > n ? history.size() - n : 0;
                
                for (size_t i = start; i < history.size(); ++i) {
                    std::cout << std::setw(4) << (i + 1) << "  " << history[i] << std::endl;
                }
            } catch (const std::exception&) {
                std::cerr << "history: invalid number" << std::endl;
                return 1;
            }
        } else {
            // 搜索历史记录
            auto results = hist->search(subcommand);
            if (results.empty()) {
                std::cout << "No matching commands found." << std::endl;
            } else {
                auto history = hist->getHistory();
                for (size_t index : results) {
                    std::cout << std::setw(4) << (index + 1) << "  " << history[index] << std::endl;
                }
            }
        }
    }
    
    return 0;
}

int BuiltinCommands::cmdClear(std::shared_ptr<Command> command) {
    // 清屏：发送ANSI转义序列
    std::cout << "\033[2J\033[H" << std::flush;
    return 0;
}

int BuiltinCommands::cmdWhich(std::shared_ptr<Command> command) {
    if (command->arguments.empty()) {
        std::cerr << "which: missing command name" << std::endl;
        return 1;
    }
    
    for (const auto& cmd : command->arguments) {
        if (isBuiltinCommand(cmd)) {
            std::cout << cmd << ": shell builtin" << std::endl;
        } else {
            // 在PATH中查找
            std::string path = shell->getEnvironmentVariable("PATH");
            bool found = false;
            
            if (!path.empty()) {
                std::string current;
                for (char c : path) {
                    if (c == ':') {
                        if (!current.empty()) {
                            std::string fullPath = current + "/" + cmd;
                            struct stat st;
                            if (stat(fullPath.c_str(), &st) == 0 && (st.st_mode & S_IXUSR)) {
                                std::cout << fullPath << std::endl;
                                found = true;
                                break;
                            }
                            current.clear();
                        }
                    } else {
                        current += c;
                    }
                }
                
                if (!found && !current.empty()) {
                    std::string fullPath = current + "/" + cmd;
                    struct stat st;
                    if (stat(fullPath.c_str(), &st) == 0 && (st.st_mode & S_IXUSR)) {
                        std::cout << fullPath << std::endl;
                        found = true;
                    }
                }
            }
            
            if (!found) {
                std::cout << cmd << " not found" << std::endl;
            }
        }
    }
    
    return 0;
}

void BuiltinCommands::printHelp() {
    std::cout << "MyShell v1.0 - 内置命令帮助\n" << std::endl;
    std::cout << "内置命令：" << std::endl;
    std::cout << "  help      - 显示此帮助信息" << std::endl;
    std::cout << "  exit [n]  - 退出shell，可选择退出码" << std::endl;
    std::cout << "  pwd       - 显示当前工作目录" << std::endl;
    std::cout << "  cd [dir]  - 切换目录，无参数时切换到HOME" << std::endl;
    std::cout << "  echo [-n] - 显示文本，-n选项不换行" << std::endl;
    std::cout << "  export    - 设置环境变量" << std::endl;
    std::cout << "  env       - 显示所有环境变量" << std::endl;
    std::cout << "  unset     - 删除环境变量" << std::endl;
    std::cout << "  history   - 显示命令历史" << std::endl;
    std::cout << "  clear     - 清屏" << std::endl;
    std::cout << "  which     - 查找命令位置" << std::endl;
    std::cout << "  set       - 配置自动补全和语法高亮" << std::endl;
    std::cout << std::endl;
    std::cout << "特殊功能：" << std::endl;
    std::cout << "  > file    - 输出重定向" << std::endl;
    std::cout << "  >> file   - 追加输出重定向" << std::endl;
    std::cout << "  < file    - 输入重定向" << std::endl;
    std::cout << "  cmd1 | cmd2 - 管道" << std::endl;
    std::cout << "  cmd &     - 后台运行" << std::endl;
    std::cout << "  $VAR      - 环境变量替换" << std::endl;
    std::cout << "  Tab       - 自动补全（安装readline时）" << std::endl;
}

bool BuiltinCommands::changeDirectory(const std::string& path) {
    if (chdir(path.c_str()) == -1) {
        perror("cd");
        return false;
    }
    
    // 更新PWD环境变量
    char* newPwd = getcwd(nullptr, 0);
    if (newPwd) {
        shell->setEnvironmentVariable("PWD", newPwd);
        free(newPwd);
    }
    
    return true;
}

int BuiltinCommands::cmdSet(std::shared_ptr<Command> command) {
    if (command->arguments.empty()) {
        // 显示当前设置
        std::cout << "MyShell 设置:" << std::endl;
        std::cout << "  completion: " << (shell->isCompletionEnabled() ? "enabled" : "disabled") << std::endl;
        std::cout << "  syntax-highlight: " << (shell->isSyntaxHighlightEnabled() ? "enabled" : "disabled") << std::endl;
        std::cout << std::endl;
        std::cout << "用法:" << std::endl;
        std::cout << "  set completion on|off     - 启用/禁用自动补全" << std::endl;
        std::cout << "  set syntax-highlight on|off - 启用/禁用语法高亮" << std::endl;
        return 0;
    }
    
    if (command->arguments.size() < 2) {
        std::cerr << "set: missing arguments" << std::endl;
        std::cerr << "Usage: set <option> <value>" << std::endl;
        return 1;
    }
    
    std::string option = command->arguments[0];
    std::string value = command->arguments[1];
    
    // 标准化值
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    bool enable = (value == "on" || value == "true" || value == "1" || value == "enabled");
    
    if (option == "completion") {
        shell->setCompletionEnabled(enable);
        std::cout << "Auto-completion " << (enable ? "enabled" : "disabled") << std::endl;
        return 0;
    } else if (option == "syntax-highlight") {
        shell->setSyntaxHighlightEnabled(enable);
        std::cout << "Syntax highlighting " << (enable ? "enabled" : "disabled") << std::endl;
        return 0;
    } else {
        std::cerr << "set: unknown option '" << option << "'" << std::endl;
        std::cerr << "Available options: completion, syntax-highlight" << std::endl;
        return 1;
    }
}