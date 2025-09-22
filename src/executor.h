#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"
#include <memory>
#include <vector>
#include <sys/types.h>

class Shell;

class Executor {
public:
    explicit Executor(Shell* shell);
    ~Executor();
    
    // 执行命令
    int execute(std::shared_ptr<Command> command);
    
    // 执行管道命令
    int executePipeline(std::shared_ptr<PipelineCommand> pipeline);
    
private:
    Shell* shell;
    
    // 执行外部程序
    int executeExternal(std::shared_ptr<Command> command);
    
    // 设置重定向
    bool setupRedirection(std::shared_ptr<Command> command, 
                         int& savedStdin, int& savedStdout);
    
    // 恢复重定向
    void restoreRedirection(int savedStdin, int savedStdout);
    
    // 查找可执行文件
    std::string findExecutable(const std::string& command);
    
    // 将命令参数转换为char*数组
    std::vector<char*> createArgv(std::shared_ptr<Command> command);
    
    // 等待子进程
    int waitForChild(pid_t pid, bool background);
    
    // 处理信号
    void setupSignalHandlers();
    
    // 分割PATH环境变量
    std::vector<std::string> splitPath(const std::string& path);
};

#endif // EXECUTOR_H