#ifndef SYNTAX_HIGHLIGHTER_H
#define SYNTAX_HIGHLIGHTER_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <regex>

// ANSI颜色代码
namespace Colors {
    constexpr const char* RESET = "\033[0m";
    constexpr const char* BOLD = "\033[1m";
    constexpr const char* DIM = "\033[2m";
    
    // 前景色
    constexpr const char* BLACK = "\033[30m";
    constexpr const char* RED = "\033[31m";
    constexpr const char* GREEN = "\033[32m";
    constexpr const char* YELLOW = "\033[33m";
    constexpr const char* BLUE = "\033[34m";
    constexpr const char* MAGENTA = "\033[35m";
    constexpr const char* CYAN = "\033[36m";
    constexpr const char* WHITE = "\033[37m";
    
    // 明亮前景色
    constexpr const char* BRIGHT_BLACK = "\033[90m";
    constexpr const char* BRIGHT_RED = "\033[91m";
    constexpr const char* BRIGHT_GREEN = "\033[92m";
    constexpr const char* BRIGHT_YELLOW = "\033[93m";
    constexpr const char* BRIGHT_BLUE = "\033[94m";
    constexpr const char* BRIGHT_MAGENTA = "\033[95m";
    constexpr const char* BRIGHT_CYAN = "\033[96m";
    constexpr const char* BRIGHT_WHITE = "\033[97m";
}

// 语法元素类型
enum class SyntaxType {
    COMMAND,            // 命令
    BUILTIN_COMMAND,    // 内置命令
    OPTION,             // 选项 (-l, --help)
    STRING,             // 字符串 ("text", 'text')
    VARIABLE,           // 变量 ($VAR, ${VAR})
    PIPE,               // 管道 (|)
    REDIRECT,           // 重定向 (>, >>, <)
    BACKGROUND,         // 后台 (&)
    COMMENT,            // 注释 (#)
    NUMBER,             // 数字
    PATH,               // 文件路径
    OPERATOR,           // 操作符
    NORMAL              // 普通文本
};

// 高亮样式配置
struct HighlightStyle {
    std::string color;
    bool bold = false;
    bool dim = false;
    
    HighlightStyle() = default;
    HighlightStyle(const std::string& c, bool b = false, bool d = false) 
        : color(c), bold(b), dim(d) {}
    
    std::string apply(const std::string& text) const {
        std::string style = color;
        if (bold) style += Colors::BOLD;
        if (dim) style += Colors::DIM;
        return style + text + Colors::RESET;
    }
};

// 语法高亮器
class SyntaxHighlighter {
public:
    SyntaxHighlighter();
    ~SyntaxHighlighter() = default;
    
    // 高亮整行命令
    std::string highlight(const std::string& line);
    
    // 设置高亮样式
    void setStyle(SyntaxType type, const HighlightStyle& style);
    
    // 获取高亮样式
    HighlightStyle getStyle(SyntaxType type) const;
    
    // 启用/禁用高亮
    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }
    
    // 注册内置命令列表
    void setBuiltinCommands(const std::set<std::string>& commands);
    
private:
    bool enabled_;
    std::map<SyntaxType, HighlightStyle> styles_;
    std::set<std::string> builtin_commands_;
    
    // 初始化默认样式
    void initializeDefaultStyles();
    
    // 分析命令行语法
    std::vector<std::pair<SyntaxType, std::string>> analyze(const std::string& line);
    
    // 检测各种语法元素
    SyntaxType detectType(const std::string& token, bool is_first_word = false);
    bool isOption(const std::string& token);
    bool isString(const std::string& token);
    bool isVariable(const std::string& token);
    bool isPath(const std::string& token);
    bool isNumber(const std::string& token);
    bool isRedirection(const std::string& token);
    
    // 分词器
    std::vector<std::string> tokenize(const std::string& line);
};

#endif // SYNTAX_HIGHLIGHTER_H