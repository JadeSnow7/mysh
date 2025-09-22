# MyShell 开发指南

## C++ Linux Shell 开发思路详解

### 1. 核心概念理解

#### 1.1 Shell是什么？
Shell是用户与操作系统内核之间的接口，主要功能包括：
- 解析用户输入的命令
- 启动和管理进程
- 处理输入输出重定向
- 提供编程环境（变量、流控制等）

#### 1.2 关键系统调用
- `fork()`: 创建子进程
- `exec()`: 在进程中执行新程序
- `wait()`: 等待子进程结束
- `pipe()`: 创建管道
- `dup2()`: 复制文件描述符
- `chdir()`: 改变当前目录

### 2. 架构设计思路

#### 2.1 模块化设计
```cpp
// 核心模块分离
class Shell;        // 主控制器
class Parser;       // 命令解析
class Executor;     // 执行引擎
class BuiltinCommands; // 内置命令
class History;      // 历史管理
```

#### 2.2 数据流设计
```
用户输入 → 词法分析 → 语法解析 → 命令执行 → 结果输出
    ↓         ↓        ↓        ↓        ↓
  原始字符  → Token流 → AST结构 → 系统调用 → 用户界面
```

### 3. 实现细节

#### 3.1 命令解析器设计

**词法分析**:
```cpp
std::vector<std::string> tokenize(const std::string& input) {
    // 处理空白符分隔
    // 处理引号（"和'）
    // 处理特殊字符（>、<、|、&）
    // 处理环境变量替换（$VAR）
}
```

**语法解析**:
```cpp
struct Command {
    std::string command;                    // 主命令
    std::vector<std::string> arguments;     // 参数
    std::string inputRedirect;              // < file
    std::string outputRedirect;             // > file
    bool appendOutput;                      // >>
    bool runInBackground;                   // &
};
```

#### 3.2 进程执行模型

```cpp
int executeCommand(const Command& cmd) {
    pid_t pid = fork();
    if (pid == 0) {
        // 子进程：设置重定向，执行程序
        setupRedirection(cmd);
        execv(cmd.executable, cmd.argv);
    } else {
        // 父进程：等待或继续（后台）
        return waitForChild(pid, cmd.background);
    }
}
```

#### 3.3 管道实现

```cpp
int executePipeline(const std::vector<Command>& commands) {
    // 创建管道数组
    std::vector<int> pipes;
    
    // 为每个命令创建进程
    for (size_t i = 0; i < commands.size(); ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            // 设置管道重定向
            if (i > 0) dup2(pipes[(i-1)*2], STDIN_FILENO);
            if (i < commands.size()-1) dup2(pipes[i*2+1], STDOUT_FILENO);
            
            // 关闭所有管道描述符
            closeAllPipes(pipes);
            
            // 执行命令
            execv(commands[i].executable, commands[i].argv);
        }
    }
    
    // 父进程等待所有子进程
    waitForAllChildren();
}
```

#### 3.4 重定向实现

```cpp
bool setupRedirection(const Command& cmd) {
    // 保存原始描述符
    int savedStdin = dup(STDIN_FILENO);
    int savedStdout = dup(STDOUT_FILENO);
    
    // 输入重定向
    if (!cmd.inputFile.empty()) {
        int fd = open(cmd.inputFile.c_str(), O_RDONLY);
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    
    // 输出重定向
    if (!cmd.outputFile.empty()) {
        int flags = O_WRONLY | O_CREAT;
        if (cmd.appendMode) flags |= O_APPEND;
        else flags |= O_TRUNC;
        
        int fd = open(cmd.outputFile.c_str(), flags, 0644);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}
```

### 4. 高级功能实现

#### 4.1 环境变量管理

```cpp
class Environment {
    std::map<std::string, std::string> variables;
    
public:
    void set(const std::string& name, const std::string& value) {
        variables[name] = value;
        setenv(name.c_str(), value.c_str(), 1);
    }
    
    std::string get(const std::string& name) {
        auto it = variables.find(name);
        return (it != variables.end()) ? it->second : "";
    }
};
```

#### 4.2 历史记录管理

```cpp
class History {
    std::vector<std::string> commands;
    std::string historyFile;
    
public:
    void addCommand(const std::string& cmd) {
        if (!cmd.empty() && (commands.empty() || commands.back() != cmd)) {
            commands.push_back(cmd);
            saveToFile();
        }
    }
    
    void loadFromFile() {
        std::ifstream file(historyFile);
        std::string line;
        while (std::getline(file, line)) {
            commands.push_back(line);
        }
    }
};
```

#### 4.3 信号处理

```cpp
void setupSignalHandlers() {
    // 父进程忽略中断信号，让子进程处理
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    
    // 处理子进程结束信号，避免僵尸进程
    signal(SIGCHLD, childHandler);
}

void childHandler(int sig) {
    while (waitpid(-1, nullptr, WNOHANG) > 0) {
        // 清理僵尸进程
    }
}
```

### 5. 开发最佳实践

#### 5.1 错误处理
```cpp
// 系统调用错误检查
if (fork() == -1) {
    perror("fork failed");
    return -1;
}

// 资源管理
class FileDescriptor {
    int fd;
public:
    FileDescriptor(int f) : fd(f) {}
    ~FileDescriptor() { if (fd >= 0) close(fd); }
    operator int() const { return fd; }
};
```

#### 5.2 内存管理
```cpp
// 使用智能指针
std::unique_ptr<Parser> parser;
std::shared_ptr<Command> command;

// RAII原则
class RedirectionGuard {
    int savedStdin, savedStdout;
public:
    RedirectionGuard() : savedStdin(dup(0)), savedStdout(dup(1)) {}
    ~RedirectionGuard() {
        dup2(savedStdin, 0);
        dup2(savedStdout, 1);
        close(savedStdin);
        close(savedStdout);
    }
};
```

#### 5.3 可测试性设计
```cpp
// 依赖注入
class Shell {
    std::unique_ptr<Parser> parser;
    std::unique_ptr<Executor> executor;
    
public:
    Shell(std::unique_ptr<Parser> p, std::unique_ptr<Executor> e)
        : parser(std::move(p)), executor(std::move(e)) {}
};

// 模拟接口
class IExecutor {
public:
    virtual int execute(const Command& cmd) = 0;
};
```

### 6. 扩展功能建议

#### 6.1 Tab补全
```cpp
class TabCompletion {
public:
    std::vector<std::string> complete(const std::string& partial) {
        // 命令补全：在PATH中搜索
        // 文件补全：在当前目录搜索
        // 变量补全：环境变量匹配
    }
};
```

#### 6.2 作业控制
```cpp
class JobControl {
    struct Job {
        pid_t pid;
        std::string command;
        bool running;
        bool foreground;
    };
    
    std::vector<Job> jobs;
    
public:
    void addJob(pid_t pid, const std::string& cmd, bool fg);
    void bringToForeground(int jobId);
    void sendToBackground(int jobId);
};
```

#### 6.3 脚本执行
```cpp
class ScriptEngine {
public:
    int executeScript(const std::string& filename) {
        std::ifstream file(filename);
        std::string line;
        
        while (std::getline(file, line)) {
            if (!line.empty() && line[0] != '#') {
                shell->executeCommand(line);
            }
        }
    }
};
```

### 7. 性能优化

#### 7.1 缓存优化
```cpp
class CommandCache {
    std::unordered_map<std::string, std::string> pathCache;
    
public:
    std::string findExecutable(const std::string& cmd) {
        auto it = pathCache.find(cmd);
        if (it != pathCache.end()) {
            return it->second;
        }
        
        std::string path = searchInPath(cmd);
        pathCache[cmd] = path;
        return path;
    }
};
```

#### 7.2 异步I/O
```cpp
class AsyncIO {
public:
    void readInputAsync(std::function<void(std::string)> callback) {
        std::thread([callback]() {
            std::string input;
            std::getline(std::cin, input);
            callback(input);
        }).detach();
    }
};
```

### 8. 调试技巧

#### 8.1 日志系统
```cpp
class Logger {
public:
    static void debug(const std::string& msg) {
        #ifdef DEBUG
        std::cerr << "[DEBUG] " << msg << std::endl;
        #endif
    }
};
```

#### 8.2 单元测试
```cpp
// 测试框架示例
TEST(ParserTest, BasicCommand) {
    Parser parser;
    auto cmd = parser.parse("ls -la");
    
    ASSERT_EQ(cmd->command, "ls");
    ASSERT_EQ(cmd->arguments.size(), 1);
    ASSERT_EQ(cmd->arguments[0], "-la");
}
```

这个开发指南提供了完整的shell开发思路和实现细节，可以帮助你深入理解Linux系统编程和shell工作原理。