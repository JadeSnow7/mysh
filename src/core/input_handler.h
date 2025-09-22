#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

class Shell;
class CompletionEngine;
class SyntaxHighlighter;

// 输入处理器类
class InputHandler {
public:
    explicit InputHandler(Shell* shell);
    ~InputHandler();
    
    // 初始化readline
    bool initialize();
    
    // 读取一行输入（支持补全和高亮）
    std::string readLine(const std::string& prompt);
    
    // 启用/禁用自动补全
    void setCompletionEnabled(bool enabled) { completion_enabled_ = enabled; }
    bool isCompletionEnabled() const { return completion_enabled_; }
    
    // 启用/禁用语法高亮
    void setSyntaxHighlightEnabled(bool enabled);
    bool isSyntaxHighlightEnabled() const;
    
    // 获取语法高亮器（用于外部访问）
    SyntaxHighlighter* getSyntaxHighlighter() { return syntax_highlighter_.get(); }
    
    // 添加历史记录
    void addHistory(const std::string& line);
    
    // 清理资源
    void cleanup();
    
private:
    Shell* shell_;
    std::unique_ptr<CompletionEngine> completion_engine_;
    std::unique_ptr<SyntaxHighlighter> syntax_highlighter_;
    
    bool initialized_;
    bool completion_enabled_;
    bool use_readline_;
    
    // readline相关的静态函数
    static char** completion_function(const char* text, int start, int end);
    static char* command_generator(const char* text, int state);
    static void initialize_readline();
    
    // 静态实例指针（readline需要）
    static InputHandler* instance_;
    
    // 非readline输入处理
    std::string readLineSimple(const std::string& prompt);
    
    // 检查readline可用性
    bool checkReadlineAvailability();
};

#endif // INPUT_HANDLER_H