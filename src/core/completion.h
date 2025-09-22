#ifndef COMPLETION_H
#define COMPLETION_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <functional>

class Shell;

// 补全类型枚举
enum class CompletionType {
    COMMAND,        // 命令补全
    FILE_PATH,      // 文件路径补全  
    ENVIRONMENT,    // 环境变量补全
    BUILTIN,        // 内置命令补全
    OPTION          // 选项补全
};

// 补全候选项结构
struct CompletionCandidate {
    std::string text;           // 补全文本
    std::string description;    // 描述信息
    CompletionType type;        // 补全类型
    
    CompletionCandidate(const std::string& t, const std::string& desc = "", 
                       CompletionType tp = CompletionType::COMMAND)
        : text(t), description(desc), type(tp) {}
};

// 补全上下文信息
struct CompletionContext {
    std::string line;           // 当前完整命令行
    std::string word;           // 当前要补全的词
    size_t position;            // 光标位置
    size_t word_start;          // 当前词的开始位置
    size_t word_end;            // 当前词的结束位置
    bool is_first_word;         // 是否是第一个词（命令）
    std::vector<std::string> words; // 已分析的所有词
};

// 补全引擎类
class CompletionEngine {
public:
    explicit CompletionEngine(Shell* shell);
    ~CompletionEngine() = default;
    
    // 执行补全
    std::vector<CompletionCandidate> complete(const CompletionContext& context);
    
    // 获取可能的补全列表
    std::vector<std::string> getCompletions(const std::string& text, int start, int end);
    
    // 注册自定义补全器
    void registerCompleter(CompletionType type, 
                          std::function<std::vector<CompletionCandidate>(const CompletionContext&)> completer);
    
private:
    Shell* shell_;
    std::set<std::string> builtin_commands_;
    std::set<std::string> system_commands_;
    
    // 各种补全器
    std::vector<CompletionCandidate> completeCommand(const CompletionContext& context);
    std::vector<CompletionCandidate> completeFilePath(const CompletionContext& context);
    std::vector<CompletionCandidate> completeEnvironmentVariable(const CompletionContext& context);
    std::vector<CompletionCandidate> completeBuiltinCommand(const CompletionContext& context);
    
    // 工具方法
    std::vector<std::string> getSystemCommands();
    std::vector<std::string> getFilesInDirectory(const std::string& dir, const std::string& prefix = "");
    bool isExecutable(const std::string& path);
    void initializeBuiltinCommands();
    void cacheSystemCommands();
    
    // 自定义补全器映射
    std::map<CompletionType, std::function<std::vector<CompletionCandidate>(const CompletionContext&)>> custom_completers_;
};

#endif // COMPLETION_H