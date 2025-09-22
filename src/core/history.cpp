#include "history.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>

#ifdef PLATFORM_WINDOWS
#include "posix_compat.h"
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#endif

History::History() : maxHistorySize(1000) {
    historyFile = getHistoryFilePath();
    loadFromFile();
}

History::~History() {
    saveToFile();
}

void History::addCommand(const std::string& command) {
    // 忽略空命令和重复的连续命令
    if (command.empty() || command.find_first_not_of(" \t\n\r") == std::string::npos) {
        return;
    }
    
    if (!commands.empty() && commands.back() == command) {
        return;
    }
    
    commands.push_back(command);
    trimHistory();
}

std::vector<std::string> History::getHistory() const {
    return commands;
}

std::string History::getCommand(size_t index) const {
    if (index < commands.size()) {
        return commands[index];
    }
    return "";
}

size_t History::size() const {
    return commands.size();
}

void History::clear() {
    commands.clear();
    saveToFile();
}

void History::show() const {
    for (size_t i = 0; i < commands.size(); ++i) {
        std::cout << std::setw(4) << (i + 1) << "  " << commands[i] << std::endl;
    }
}

std::vector<size_t> History::search(const std::string& pattern) const {
    std::vector<size_t> results;
    
    for (size_t i = 0; i < commands.size(); ++i) {
        if (commands[i].find(pattern) != std::string::npos) {
            results.push_back(i);
        }
    }
    
    return results;
}

void History::setMaxSize(size_t maxSize) {
    maxHistorySize = maxSize;
    trimHistory();
}

void History::loadFromFile() {
    std::ifstream file(historyFile);
    if (!file.is_open()) {
        return; // 文件不存在是正常的
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            commands.push_back(line);
        }
    }
    
    file.close();
    trimHistory();
}

void History::saveToFile() {
    std::ofstream file(historyFile);
    if (!file.is_open()) {
        return; // 无法写入文件，静默失败
    }
    
    for (const auto& command : commands) {
        file << command << std::endl;
    }
    
    file.close();
}

std::string History::getHistoryFilePath() {
    char* home = getenv("HOME");
    if (home) {
        return std::string(home) + "/.mysh_history";
    }
    
    // 如果没有HOME环境变量，尝试从passwd获取
    struct passwd* pw = getpwuid(getuid());
    if (pw) {
        return std::string(pw->pw_dir) + "/.mysh_history";
    }
    
    // 最后的备选方案
    return ".mysh_history";
}

void History::trimHistory() {
    if (commands.size() > maxHistorySize) {
        size_t toRemove = commands.size() - maxHistorySize;
        commands.erase(commands.begin(), commands.begin() + toRemove);
    }
}