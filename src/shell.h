#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>
#include <map>
#include <memory>

class Parser;
class Executor;
class BuiltinCommands;
class History;

class Shell {
public:
    Shell();
    ~Shell();
    
    // 主运行循环
    int run();
    
    // 执行单个命令
    int executeCommand(const std::string& command);
    
    // 获取和设置环境变量
    std::string getEnvironmentVariable(const std::string& name);
    void setEnvironmentVariable(const std::string& name, const std::string& value);
    
    // 获取当前工作目录
    std::string getCurrentDirectory();
    
    // 获取历史记录对象
    History* getHistory() { return history.get(); }
    
    // 设置退出标志
    void setExitFlag(bool flag) { shouldExit = flag; }
    
    // 获取shell提示符
    std::string getPrompt();

private:
    std::unique_ptr<Parser> parser;
    std::unique_ptr<Executor> executor;
    std::unique_ptr<BuiltinCommands> builtinCommands;
    std::unique_ptr<History> history;
    
    std::map<std::string, std::string> environmentVariables;
    std::string currentDirectory;
    bool shouldExit;
    
    // 初始化shell
    void initialize();
    
    // 读取用户输入
    std::string readInput();
    
    // 显示欢迎信息
    void showWelcome();
};

#endif // SHELL_H