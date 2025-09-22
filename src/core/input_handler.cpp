#include "input_handler.h"
#include "shell.h"
#include "completion.h"
#include "syntax_highlighter.h"
#include <iostream>
#include <algorithm>

// 检查readline是否可用
#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#define USE_READLINE 1
#else
#define USE_READLINE 0
#endif

// 静态实例指针
InputHandler* InputHandler::instance_ = nullptr;

InputHandler::InputHandler(Shell* shell) 
    : shell_(shell), initialized_(false), completion_enabled_(true), use_readline_(false) {
    
    // 重新启用CompletionEngine
    completion_engine_ = std::make_unique<CompletionEngine>(shell);
    syntax_highlighter_ = std::make_unique<SyntaxHighlighter>();
    
    // 设置内置命令给语法高亮器
    std::set<std::string> builtin_commands = {
        "help", "exit", "pwd", "cd", "echo", "export", 
        "env", "unset", "history", "clear", "which", "set"
    };
    syntax_highlighter_->setBuiltinCommands(builtin_commands);
    
    instance_ = this;
}

InputHandler::~InputHandler() {
    cleanup();
    instance_ = nullptr;
}

bool InputHandler::initialize() {
    if (initialized_) {
        return true;
    }
    
    use_readline_ = checkReadlineAvailability();
    
    if (use_readline_) {
#if USE_READLINE
        initialize_readline();
        std::cout << "Using GNU Readline for enhanced input features" << std::endl;
#endif
    } else {
        std::cout << "Using simple input mode (install libreadline-dev for enhanced features)" << std::endl;
    }
    
    initialized_ = true;
    return true;
}

std::string InputHandler::readLine(const std::string& prompt) {
    if (use_readline_) {
#if USE_READLINE
        char* line = readline(prompt.c_str());
        if (line == nullptr) {
            return ""; // EOF
        }
        
        std::string result(line);
        free(line);
        
        // 如果启用语法高亮，在这里可以处理实时高亮
        // 注意：readline本身不支持实时语法高亮，这需要更复杂的实现
        
        return result;
#endif
    }
    
    return readLineSimple(prompt);
}

void InputHandler::setSyntaxHighlightEnabled(bool enabled) {
    if (syntax_highlighter_) {
        syntax_highlighter_->setEnabled(enabled);
    }
}

bool InputHandler::isSyntaxHighlightEnabled() const {
    return syntax_highlighter_ && syntax_highlighter_->isEnabled();
}

void InputHandler::addHistory(const std::string& line) {
    if (line.empty()) {
        return;
    }
    
    if (use_readline_) {
#if USE_READLINE
        add_history(line.c_str());
#endif
    }
}

void InputHandler::cleanup() {
    if (use_readline_) {
#if USE_READLINE
        rl_cleanup_after_signal();
#endif
    }
}

std::string InputHandler::readLineSimple(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    
    // 检查EOF（Ctrl+D）
    if (std::cin.eof()) {
        return "";
    }
    
    // 应用语法高亮（仅用于显示，不改变实际输入）
    if (syntax_highlighter_ && syntax_highlighter_->isEnabled() && !input.empty()) {
        // 在简单模式下，我们可以在命令执行后显示高亮版本
        // 这里只是记录，实际高亮会在其他地方处理
    }
    
    return input;
}

bool InputHandler::checkReadlineAvailability() {
#if USE_READLINE
    return true;
#else
    return false;
#endif
}

#if USE_READLINE
// readline回调函数
char** InputHandler::completion_function(const char* text, int start, int end) {
    if (!instance_ || !instance_->completion_enabled_) {
        return nullptr;
    }
    
    // 获取补全候选项
    std::string current_line = rl_line_buffer;
    auto completions = instance_->completion_engine_->getCompletions(current_line, start, end);
    
    if (completions.empty()) {
        return nullptr;
    }
    
    // 转换为readline格式
    char** matches = (char**)malloc(sizeof(char*) * (completions.size() + 2));
    if (!matches) {
        return nullptr;
    }
    
    matches[0] = strdup(text); // 默认匹配
    
    for (size_t i = 0; i < completions.size(); ++i) {
        matches[i + 1] = strdup(completions[i].c_str());
    }
    matches[completions.size() + 1] = nullptr;
    
    return matches;
}

char* InputHandler::command_generator(const char* text, int state) {
    // 这个函数会被readline反复调用来生成补全候选项
    static std::vector<std::string> matches;
    static size_t match_index;
    
    if (state == 0) {
        // 第一次调用，生成匹配列表
        matches.clear();
        match_index = 0;
        
        if (instance_ && instance_->completion_enabled_) {
            std::string current_line = rl_line_buffer;
            int start = rl_point - strlen(text);
            int end = rl_point;
            matches = instance_->completion_engine_->getCompletions(current_line, start, end);
        }
    }
    
    // 返回下一个匹配项
    if (match_index < matches.size()) {
        return strdup(matches[match_index++].c_str());
    }
    
    return nullptr;
}

void InputHandler::initialize_readline() {
    // 设置补全函数
    rl_attempted_completion_function = completion_function;
    
    // 设置其他readline选项
    rl_basic_word_break_characters = " \t\n\"\\'`@$><=;|&{(";
    
    // 启用历史记录
    using_history();
    
    // 设置历史文件（可选）
    // read_history(".mysh_history");
    
    // 其他readline配置
    rl_completion_append_character = ' ';
    rl_completion_suppress_append = 0;
}
#endif