#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <memory>

// 命令结构体
struct Command {
    std::string command;                    // 主命令
    std::vector<std::string> arguments;     // 参数列表
    std::string inputRedirect;              // 输入重定向文件
    std::string outputRedirect;             // 输出重定向文件
    bool appendOutput;                      // 是否追加输出 (>>)
    bool runInBackground;                   // 是否后台运行 (&)
    
    Command() : appendOutput(false), runInBackground(false) {}
};

// 管道命令结构体
struct PipelineCommand {
    std::vector<std::shared_ptr<Command>> commands;
    
    PipelineCommand() = default;
};

class Parser {
public:
    Parser();
    ~Parser();
    
    // 解析命令行字符串
    std::shared_ptr<Command> parse(const std::string& input);
    
    // 解析管道命令
    std::shared_ptr<PipelineCommand> parsePipeline(const std::string& input);
    
private:
    // 词法分析 - 将输入分解为tokens
    std::vector<std::string> tokenize(const std::string& input);
    
    // 解析单个命令
    std::shared_ptr<Command> parseCommand(const std::vector<std::string>& tokens);
    
    // 处理引号
    std::string handleQuotes(const std::string& token);
    
    // 处理环境变量替换
    std::string expandVariables(const std::string& input);
    
    // 检查是否是特殊字符
    bool isSpecialChar(char c);
    
    // 去除字符串两端空白
    std::string trim(const std::string& str);
};

#endif // PARSER_H