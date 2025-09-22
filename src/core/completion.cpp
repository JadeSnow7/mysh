#include "completion.h"
#include "shell.h"
#include "builtin.h"
#include <algorithm>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <iostream>

CompletionEngine::CompletionEngine(Shell* shell) : shell_(shell) {
    initializeBuiltinCommands();
    cacheSystemCommands();
}

std::vector<CompletionCandidate> CompletionEngine::complete(const CompletionContext& context) {
    std::vector<CompletionCandidate> candidates;
    
    // 根据上下文决定补全类型
    if (context.is_first_word) {
        // 第一个词：补全命令
        auto command_candidates = completeCommand(context);
        candidates.insert(candidates.end(), command_candidates.begin(), command_candidates.end());
    } else {
        // 其他位置：根据内容类型补全
        if (context.word.empty() || context.word[0] == '/') {
            // 绝对路径或空词：文件路径补全
            auto file_candidates = completeFilePath(context);
            candidates.insert(candidates.end(), file_candidates.begin(), file_candidates.end());
        } else if (context.word[0] == '$') {
            // 环境变量补全
            auto env_candidates = completeEnvironmentVariable(context);
            candidates.insert(candidates.end(), env_candidates.begin(), env_candidates.end());
        } else if (context.word[0] == '-') {
            // 选项补全（暂时不实现具体选项）
            // 可以根据第一个命令来提供相应的选项补全
        } else {
            // 默认：文件路径补全
            auto file_candidates = completeFilePath(context);
            candidates.insert(candidates.end(), file_candidates.begin(), file_candidates.end());
        }
    }
    
    // 排序候选项
    std::sort(candidates.begin(), candidates.end(), 
        [](const CompletionCandidate& a, const CompletionCandidate& b) {
            return a.text < b.text;
        });
    
    return candidates;
}

std::vector<std::string> CompletionEngine::getCompletions(const std::string& text, int start, int end) {
    // 解析上下文
    CompletionContext context;
    context.line = text;
    context.position = end;
    context.word_start = start;
    context.word_end = end;
    context.word = text.substr(start, end - start);
    
    // 简单的分词
    std::istringstream iss(text.substr(0, start));
    std::string word;
    while (iss >> word) {
        context.words.push_back(word);
    }
    context.is_first_word = context.words.empty();
    
    // 获取补全候选项
    auto candidates = complete(context);
    
    // 转换为字符串列表
    std::vector<std::string> completions;
    for (const auto& candidate : candidates) {
        if (candidate.text.find(context.word) == 0) {
            completions.push_back(candidate.text);
        }
    }
    
    return completions;
}

std::vector<CompletionCandidate> CompletionEngine::completeCommand(const CompletionContext& context) {
    std::vector<CompletionCandidate> candidates;
    
    // 内置命令补全
    for (const auto& cmd : builtin_commands_) {
        if (cmd.find(context.word) == 0) {
            candidates.emplace_back(cmd, "内置命令", CompletionType::BUILTIN);
        }
    }
    
    // 系统命令补全
    for (const auto& cmd : system_commands_) {
        if (cmd.find(context.word) == 0) {
            candidates.emplace_back(cmd, "系统命令", CompletionType::COMMAND);
        }
    }
    
    return candidates;
}

std::vector<CompletionCandidate> CompletionEngine::completeFilePath(const CompletionContext& context) {
    std::vector<CompletionCandidate> candidates;
    
    std::string word = context.word;
    std::string dir_path;
    std::string file_prefix;
    
    // 解析目录和文件前缀
    size_t last_slash = word.find_last_of('/');
    if (last_slash != std::string::npos) {
        dir_path = word.substr(0, last_slash + 1);
        file_prefix = word.substr(last_slash + 1);
        if (dir_path[0] != '/') {
            // 相对路径，前面加上当前目录
            dir_path = shell_->getCurrentDirectory() + "/" + dir_path;
        }
    } else {
        // 当前目录
        dir_path = shell_->getCurrentDirectory() + "/";
        file_prefix = word;
    }
    
    // 获取目录中的文件
    auto files = getFilesInDirectory(dir_path, file_prefix);
    for (const auto& file : files) {
        std::string full_path = dir_path + file;
        struct stat st;
        if (stat(full_path.c_str(), &st) == 0) {
            std::string desc = S_ISDIR(st.st_mode) ? "目录" : "文件";
            if (S_ISDIR(st.st_mode)) {
                candidates.emplace_back(file + "/", desc, CompletionType::FILE_PATH);
            } else {
                candidates.emplace_back(file, desc, CompletionType::FILE_PATH);
            }
        }
    }
    
    return candidates;
}

std::vector<CompletionCandidate> CompletionEngine::completeEnvironmentVariable(const CompletionContext& context) {
    std::vector<CompletionCandidate> candidates;
    
    std::string var_name = context.word.substr(1); // 去掉 $
    
    // 常见环境变量
    std::vector<std::string> common_vars = {
        "HOME", "PATH", "USER", "SHELL", "PWD", "TERM", "LANG", "TZ"
    };
    
    for (const auto& var : common_vars) {
        if (var.find(var_name) == 0) {
            std::string value = shell_->getEnvironmentVariable(var);
            std::string desc = "环境变量";
            if (!value.empty()) {
                desc += " = " + value;
            }
            candidates.emplace_back("$" + var, desc, CompletionType::ENVIRONMENT);
        }
    }
    
    return candidates;
}

std::vector<CompletionCandidate> CompletionEngine::completeBuiltinCommand(const CompletionContext& context) {
    std::vector<CompletionCandidate> candidates;
    
    for (const auto& cmd : builtin_commands_) {
        if (cmd.find(context.word) == 0) {
            candidates.emplace_back(cmd, "内置命令", CompletionType::BUILTIN);
        }
    }
    
    return candidates;
}

std::vector<std::string> CompletionEngine::getSystemCommands() {
    std::vector<std::string> commands;
    std::set<std::string> unique_commands;
    
    // 获取PATH环境变量
    std::string path = shell_->getEnvironmentVariable("PATH");
    if (path.empty()) {
        return commands;
    }
    
    // 分割PATH
    std::istringstream iss(path);
    std::string dir;
    while (std::getline(iss, dir, ':')) {
        if (dir.empty()) continue;
        
        try {
            // 使用传统的dirent方法而不是filesystem
            DIR* d = opendir(dir.c_str());
            if (d) {
                struct dirent* entry;
                while ((entry = readdir(d)) != nullptr) {
                    if (entry->d_type == DT_REG || entry->d_type == DT_LNK) {
                        std::string filename = entry->d_name;
                        std::string full_path = dir + "/" + filename;
                        if (isExecutable(full_path) && unique_commands.find(filename) == unique_commands.end()) {
                            unique_commands.insert(filename);
                            commands.push_back(filename);
                        }
                    }
                }
                closedir(d);
            }
        } catch (const std::exception&) {
            // 忽略无法访问的目录
        }
    }
    
    return commands;
}

std::vector<std::string> CompletionEngine::getFilesInDirectory(const std::string& dir, const std::string& prefix) {
    std::vector<std::string> files;
    
    try {
        DIR* d = opendir(dir.c_str());
        if (d) {
            struct dirent* entry;
            while ((entry = readdir(d)) != nullptr) {
                std::string filename = entry->d_name;
                if (filename != "." && filename != ".." && filename.find(prefix) == 0) {
                    files.push_back(filename);
                }
            }
            closedir(d);
        }
    } catch (const std::exception&) {
        // 忽略错误
    }
    
    return files;
}

bool CompletionEngine::isExecutable(const std::string& path) {
    return access(path.c_str(), X_OK) == 0;
}

void CompletionEngine::initializeBuiltinCommands() {
    builtin_commands_ = {
        "help", "exit", "pwd", "cd", "echo", "export", 
        "env", "unset", "history", "clear", "which"
    };
}

void CompletionEngine::cacheSystemCommands() {
    // 暂时禁用系统命令缓存以避免初始化问题
    // system_commands_ = std::set<std::string>(getSystemCommands().begin(), getSystemCommands().end());
    // 简单的默认命令集合
    system_commands_ = {"ls", "cat", "grep", "find", "ps", "top", "nano", "vim"};
}

void CompletionEngine::registerCompleter(CompletionType type, 
                                        std::function<std::vector<CompletionCandidate>(const CompletionContext&)> completer) {
    custom_completers_[type] = completer;
}