#include "builtin.h"
#include "shell.h"
#include <iostream>
#include <string>
#include <vector>

int BuiltinCommands::cmdAi(std::shared_ptr<Command> command) {
    // 检查是否提供了问题
    if (command->arguments.empty()) {
        std::cerr << "ai: missing question" << std::endl;
        std::cerr << "Usage: ai <question>" << std::endl;
        return 1;
    }
    
    // 将所有参数组合成一个问题字符串
    std::string question;
    for (size_t i = 0; i < command->arguments.size(); ++i) {
        if (i > 0) question += " ";
        question += command->arguments[i];
    }
    
    // 使用AI客户端获取回答
    if (aiClient_) {
        std::string answer = aiClient_->ask(question);
        std::cout << "AI Assistant: " << answer << std::endl;
    } else {
        std::cerr << "ai: AI client not available" << std::endl;
        return 1;
    }
    
    return 0;
}