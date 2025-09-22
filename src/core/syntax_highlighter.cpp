#include "syntax_highlighter.h"
#include <algorithm>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

SyntaxHighlighter::SyntaxHighlighter() : enabled_(true) {
    initializeDefaultStyles();
}

std::string SyntaxHighlighter::highlight(const std::string& line) {
    if (!enabled_ || line.empty()) {
        return line;
    }
    
    auto tokens = analyze(line);
    std::string result;
    
    for (const auto& token : tokens) {
        auto style = getStyle(token.first);
        result += style.apply(token.second);
    }
    
    return result;
}

void SyntaxHighlighter::setStyle(SyntaxType type, const HighlightStyle& style) {
    styles_[type] = style;
}

HighlightStyle SyntaxHighlighter::getStyle(SyntaxType type) const {
    auto it = styles_.find(type);
    if (it != styles_.end()) {
        return it->second;
    }
    return HighlightStyle(Colors::RESET);
}

void SyntaxHighlighter::setBuiltinCommands(const std::set<std::string>& commands) {
    builtin_commands_ = commands;
}

void SyntaxHighlighter::initializeDefaultStyles() {
    // 设置默认颜色方案
    styles_[SyntaxType::COMMAND] = HighlightStyle(Colors::BRIGHT_GREEN, true);
    styles_[SyntaxType::BUILTIN_COMMAND] = HighlightStyle(Colors::BRIGHT_BLUE, true);
    styles_[SyntaxType::OPTION] = HighlightStyle(Colors::YELLOW);
    styles_[SyntaxType::STRING] = HighlightStyle(Colors::GREEN);
    styles_[SyntaxType::VARIABLE] = HighlightStyle(Colors::CYAN);
    styles_[SyntaxType::PIPE] = HighlightStyle(Colors::BRIGHT_MAGENTA, true);
    styles_[SyntaxType::REDIRECT] = HighlightStyle(Colors::BRIGHT_RED, true);
    styles_[SyntaxType::BACKGROUND] = HighlightStyle(Colors::BRIGHT_YELLOW, true);
    styles_[SyntaxType::COMMENT] = HighlightStyle(Colors::BRIGHT_BLACK);
    styles_[SyntaxType::NUMBER] = HighlightStyle(Colors::BRIGHT_CYAN);
    styles_[SyntaxType::PATH] = HighlightStyle(Colors::BLUE);
    styles_[SyntaxType::OPERATOR] = HighlightStyle(Colors::MAGENTA);
    styles_[SyntaxType::NORMAL] = HighlightStyle(Colors::RESET);
}

std::vector<std::pair<SyntaxType, std::string>> SyntaxHighlighter::analyze(const std::string& line) {
    std::vector<std::pair<SyntaxType, std::string>> result;
    
    // 检查注释
    size_t comment_pos = line.find('#');
    std::string main_part = (comment_pos != std::string::npos) ? line.substr(0, comment_pos) : line;
    std::string comment_part = (comment_pos != std::string::npos) ? line.substr(comment_pos) : "";
    
    // 分析主要部分
    auto tokens = tokenize(main_part);
    bool is_first_word = true;
    
    for (const auto& token : tokens) {
        SyntaxType type = detectType(token, is_first_word);
        result.emplace_back(type, token);
        
        if (is_first_word && type != SyntaxType::NORMAL) {
            is_first_word = false;
        }
    }
    
    // 添加注释部分
    if (!comment_part.empty()) {
        result.emplace_back(SyntaxType::COMMENT, comment_part);
    }
    
    return result;
}

SyntaxType SyntaxHighlighter::detectType(const std::string& token, bool is_first_word) {
    if (token.empty()) {
        return SyntaxType::NORMAL;
    }
    
    // 特殊字符
    if (token == "|") return SyntaxType::PIPE;
    if (token == "&") return SyntaxType::BACKGROUND;
    if (isRedirection(token)) return SyntaxType::REDIRECT;
    
    // 选项
    if (isOption(token)) return SyntaxType::OPTION;
    
    // 字符串
    if (isString(token)) return SyntaxType::STRING;
    
    // 变量
    if (isVariable(token)) return SyntaxType::VARIABLE;
    
    // 数字
    if (isNumber(token)) return SyntaxType::NUMBER;
    
    // 第一个词：命令
    if (is_first_word) {
        if (builtin_commands_.find(token) != builtin_commands_.end()) {
            return SyntaxType::BUILTIN_COMMAND;
        }
        return SyntaxType::COMMAND;
    }
    
    // 路径
    if (isPath(token)) return SyntaxType::PATH;
    
    return SyntaxType::NORMAL;
}

bool SyntaxHighlighter::isOption(const std::string& token) {
    return token.length() >= 2 && token[0] == '-' && 
           (std::isalpha(token[1]) || token[1] == '-');
}

bool SyntaxHighlighter::isString(const std::string& token) {
    return (token.length() >= 2) && 
           ((token.front() == '"' && token.back() == '"') ||
            (token.front() == '\'' && token.back() == '\''));
}

bool SyntaxHighlighter::isVariable(const std::string& token) {
    return !token.empty() && token[0] == '$';
}

bool SyntaxHighlighter::isPath(const std::string& token) {
    return token.find('/') != std::string::npos || 
           token == "." || token == ".." ||
           (token.length() > 0 && token[0] == '~');
}

bool SyntaxHighlighter::isNumber(const std::string& token) {
    if (token.empty()) return false;
    
    size_t start = 0;
    if (token[0] == '-' || token[0] == '+') start = 1;
    
    for (size_t i = start; i < token.length(); ++i) {
        if (!std::isdigit(token[i]) && token[i] != '.') {
            return false;
        }
    }
    
    return start < token.length();
}

bool SyntaxHighlighter::isRedirection(const std::string& token) {
    return token == ">" || token == ">>" || token == "<" || 
           token == "2>" || token == "2>>" || token == "&>";
}

std::vector<std::string> SyntaxHighlighter::tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::string current_token;
    
    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        
        // 处理引号
        if (c == '"' || c == '\'') {
            if (!current_token.empty() && current_token[0] != '"' && current_token[0] != '\'') {
                // 当前token不是字符串，需要先保存
                tokens.push_back(current_token);
                current_token.clear();
            }
            
            if (current_token.empty()) {
                // 开始新的字符串token
                current_token += c;
                // 找到匹配的结束引号
                size_t end = i + 1;
                while (end < line.length() && line[end] != c) {
                    current_token += line[end];
                    end++;
                }
                if (end < line.length()) {
                    current_token += line[end]; // 添加结束引号
                    tokens.push_back(current_token);
                    current_token.clear();
                    i = end; // 跳过已处理的部分
                }
            } else if (current_token[0] == c) {
                // 结束当前字符串token
                current_token += c;
                tokens.push_back(current_token);
                current_token.clear();
            }
        }
        // 处理操作符
        else if (c == '|' || c == '&' || c == '>' || c == '<') {
            // 保存之前的token
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
            
            // 处理多字符操作符
            std::string op(1, c);
            if (c == '>' && i + 1 < line.length() && line[i + 1] == '>') {
                op = ">>";
                i++; // 跳过下一个字符
            } else if (c == '<' && i + 1 < line.length() && line[i + 1] == '<') {
                op = "<<";
                i++; // 跳过下一个字符
            } else if (c == '&' && i + 1 < line.length() && line[i + 1] == '>') {
                op = "&>";
                i++; // 跳过下一个字符
            } else if (c == '2') {
                // 检查是否是2>或2>>
                if (i + 1 < line.length() && line[i + 1] == '>') {
                    op = "2>";
                    i++; // 跳过下一个字符
                    if (i + 1 < line.length() && line[i + 1] == '>') {
                        op = "2>>";
                        i++; // 跳过下一个字符
                    }
                } else {
                    current_token += c;
                    continue;
                }
            }
            
            tokens.push_back(op);
        }
        // 处理空格
        else if (std::isspace(c)) {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
        }
        // 其他字符
        else {
            current_token += c;
        }
    }
    
    // 保存最后一个token
    if (!current_token.empty()) {
        tokens.push_back(current_token);
    }
    
    return tokens;
}