#include "parser.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>

Parser::Parser() = default;
Parser::~Parser() = default;

std::shared_ptr<Command> Parser::parse(const std::string& input) {
    if (input.empty() || input.find_first_not_of(" \t\n\r") == std::string::npos) {
        return nullptr;
    }
    
    // 检查是否包含管道
    if (input.find('|') != std::string::npos) {
        auto pipeline = parsePipeline(input);
        if (pipeline && !pipeline->commands.empty()) {
            return pipeline->commands[0]; // 暂时只返回第一个命令，后续完善管道处理
        }
        return nullptr;
    }
    
    // 词法分析
    auto tokens = tokenize(input);
    if (tokens.empty()) {
        return nullptr;
    }
    
    // 解析单个命令
    return parseCommand(tokens);
}

std::shared_ptr<PipelineCommand> Parser::parsePipeline(const std::string& input) {
    auto pipeline = std::make_shared<PipelineCommand>();
    
    // 按管道符分割
    std::istringstream iss(input);
    std::string commandStr;
    
    while (std::getline(iss, commandStr, '|')) {
        commandStr = trim(commandStr);
        if (!commandStr.empty()) {
            auto tokens = tokenize(commandStr);
            if (!tokens.empty()) {
                auto command = parseCommand(tokens);
                if (command) {
                    pipeline->commands.push_back(command);
                }
            }
        }
    }
    
    return pipeline;
}

std::vector<std::string> Parser::tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    std::string current;
    bool inQuotes = false;
    char quoteChar = '\0';
    
    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];
        
        if (!inQuotes) {
            if (c == '"' || c == '\'') {
                inQuotes = true;
                quoteChar = c;
            } else if (std::isspace(c)) {
                if (!current.empty()) {
                    tokens.push_back(expandVariables(current));
                    current.clear();
                }
            } else if (isSpecialChar(c)) {
                // 处理特殊字符
                if (!current.empty()) {
                    tokens.push_back(expandVariables(current));
                    current.clear();
                }
                
                // 处理重定向符号
                if (c == '>' && i + 1 < input.length() && input[i + 1] == '>') {
                    tokens.push_back(">>");
                    ++i; // 跳过下一个字符
                } else {
                    tokens.push_back(std::string(1, c));
                }
            } else {
                current += c;
            }
        } else {
            if (c == quoteChar) {
                inQuotes = false;
                quoteChar = '\0';
            } else {
                current += c;
            }
        }
    }
    
    if (!current.empty()) {
        tokens.push_back(expandVariables(current));
    }
    
    return tokens;
}

std::shared_ptr<Command> Parser::parseCommand(const std::vector<std::string>& tokens) {
    if (tokens.empty()) {
        return nullptr;
    }
    
    auto command = std::make_shared<Command>();
    command->command = tokens[0];
    
    for (size_t i = 1; i < tokens.size(); ++i) {
        const std::string& token = tokens[i];
        
        if (token == "<") {
            // 输入重定向
            if (i + 1 < tokens.size()) {
                command->inputRedirect = tokens[++i];
            }
        } else if (token == ">") {
            // 输出重定向
            if (i + 1 < tokens.size()) {
                command->outputRedirect = tokens[++i];
                command->appendOutput = false;
            }
        } else if (token == ">>") {
            // 追加输出重定向
            if (i + 1 < tokens.size()) {
                command->outputRedirect = tokens[++i];
                command->appendOutput = true;
            }
        } else if (token == "&") {
            // 后台运行
            command->runInBackground = true;
        } else {
            // 普通参数
            command->arguments.push_back(token);
        }
    }
    
    return command;
}

std::string Parser::handleQuotes(const std::string& token) {
    if (token.length() < 2) {
        return token;
    }
    
    char first = token[0];
    char last = token[token.length() - 1];
    
    if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
        return token.substr(1, token.length() - 2);
    }
    
    return token;
}

std::string Parser::expandVariables(const std::string& input) {
    std::string result = input;
    size_t pos = 0;
    
    while ((pos = result.find('$', pos)) != std::string::npos) {
        if (pos + 1 >= result.length()) {
            break;
        }
        
        size_t start = pos + 1;
        size_t end = start;
        
        // 找到变量名的结束位置
        while (end < result.length() && 
               (std::isalnum(result[end]) || result[end] == '_')) {
            ++end;
        }
        
        if (end > start) {
            std::string varName = result.substr(start, end - start);
            char* value = getenv(varName.c_str());
            std::string replacement = value ? value : "";
            
            result.replace(pos, end - pos, replacement);
            pos += replacement.length();
        } else {
            ++pos;
        }
    }
    
    return result;
}

bool Parser::isSpecialChar(char c) {
    return c == '<' || c == '>' || c == '&' || c == '|';
}

std::string Parser::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return "";
    }
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}