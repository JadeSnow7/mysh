#include "core/syntax_highlighter.h"
#include <iostream>

// 函数用于转义字符串中的特殊字符，以便在终端中可见
std::string escape_string(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '\033':
                result += "\\033";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result += c;
        }
    }
    return result;
}

int main() {
    SyntaxHighlighter highlighter;
    
    // 测试一些命令
    std::string test1 = "echo 'Hello World'";
    std::string test2 = "ls -la | head -5";
    std::string test3 = "cd /tmp";
    
    std::cout << "Testing syntax highlighter:" << std::endl;
    
    std::cout << "Input: " << test1 << std::endl;
    auto tokens1 = highlighter.tokenize(test1);
    std::cout << "Tokens: ";
    for (const auto& token : tokens1) {
        std::cout << "[" << token << "] ";
    }
    std::cout << std::endl;
    std::cout << "Output (escaped): " << escape_string(highlighter.highlight(test1)) << std::endl << std::endl;
    
    std::cout << "Input: " << test2 << std::endl;
    auto tokens2 = highlighter.tokenize(test2);
    std::cout << "Tokens: ";
    for (const auto& token : tokens2) {
        std::cout << "[" << token << "] ";
    }
    std::cout << std::endl;
    std::cout << "Output (escaped): " << escape_string(highlighter.highlight(test2)) << std::endl << std::endl;
    
    std::cout << "Input: " << test3 << std::endl;
    auto tokens3 = highlighter.tokenize(test3);
    std::cout << "Tokens: ";
    for (const auto& token : tokens3) {
        std::cout << "[" << token << "] ";
    }
    std::cout << std::endl;
    std::cout << "Output (escaped): " << escape_string(highlighter.highlight(test3)) << std::endl << std::endl;
    
    return 0;
}