#ifndef BUILTIN_H
#define BUILTIN_H

#include "parser.h"
#include <memory>
#include <map>
#include <functional>

class Shell;

class BuiltinCommands {
public:
    explicit BuiltinCommands(Shell* shell);
    ~BuiltinCommands();
    
    // 检查是否是内置命令
    bool isBuiltinCommand(const std::string& command);
    
    // 执行内置命令
    int execute(std::shared_ptr<Command> command);
    
private:
    Shell* shell;
    std::map<std::string, std::function<int(std::shared_ptr<Command>)>> builtinMap;
    
    // 内置命令实现
    int cmdExit(std::shared_ptr<Command> command);
    int cmdHelp(std::shared_ptr<Command> command);
    int cmdPwd(std::shared_ptr<Command> command);
    int cmdCd(std::shared_ptr<Command> command);
    int cmdEcho(std::shared_ptr<Command> command);
    int cmdExport(std::shared_ptr<Command> command);
    int cmdEnv(std::shared_ptr<Command> command);
    int cmdUnset(std::shared_ptr<Command> command);
    int cmdHistory(std::shared_ptr<Command> command);
    int cmdClear(std::shared_ptr<Command> command);
    int cmdWhich(std::shared_ptr<Command> command);
    int cmdSet(std::shared_ptr<Command> command);  // 新增：配置设置命令
    
    // 初始化内置命令映射
    void initializeBuiltins();
    
    // 辅助函数
    void printHelp();
    bool changeDirectory(const std::string& path);
};

#endif // BUILTIN_H