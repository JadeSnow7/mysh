#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <windows.h>
#include <io.h>
#include <direct.h>

class SimpleShell {
private:
    bool shouldExit = false;
    
public:
    void run() {
        std::cout << "MyShell Windows Edition - Simplified Implementation" << std::endl;
        std::cout << "Type 'help' for available commands, 'exit' to quit" << std::endl;
        
        while (!shouldExit) {
            showPrompt();
            std::string input;
            std::getline(std::cin, input);
            
            if (input.empty()) continue;
            
            processCommand(input);
        }
    }
    
private:
    void showPrompt() {
        char cwd[MAX_PATH];
        _getcwd(cwd, sizeof(cwd));
        std::cout << "mysh:" << cwd << "$ ";
    }
    
    void processCommand(const std::string& input) {
        std::vector<std::string> tokens = tokenize(input);
        if (tokens.empty()) return;
        
        std::string command = tokens[0];
        
        if (command == "exit") {
            shouldExit = true;
        } else if (command == "help") {
            showHelp();
        } else if (command == "pwd") {
            char cwd[MAX_PATH];
            _getcwd(cwd, sizeof(cwd));
            std::cout << cwd << std::endl;
        } else if (command == "cd") {
            changeDirectory(tokens);
        } else if (command == "echo") {
            echo(tokens);
        } else if (command == "dir" || command == "ls") {
            listDirectory();
        } else if (command == "clear" || command == "cls") {
            system("cls");
        } else {
            executeExternal(tokens);
        }
    }
    
    std::vector<std::string> tokenize(const std::string& input) {
        std::vector<std::string> tokens;
        std::istringstream iss(input);
        std::string token;
        
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        return tokens;
    }
    
    void showHelp() {
        std::cout << "Available commands:" << std::endl;
        std::cout << "  help     - Show this help message" << std::endl;
        std::cout << "  exit     - Exit shell" << std::endl;
        std::cout << "  pwd      - Show current directory" << std::endl;
        std::cout << "  cd [dir] - Change directory" << std::endl;
        std::cout << "  echo     - Display text" << std::endl;
        std::cout << "  dir/ls   - List files" << std::endl;
        std::cout << "  clear/cls- Clear screen" << std::endl;
        std::cout << "  Other commands will be executed as external programs" << std::endl;
    }
    
    void changeDirectory(const std::vector<std::string>& tokens) {
        std::string path;
        if (tokens.size() > 1) {
            path = tokens[1];
        } else {
            // 切换到用户主目录
            char* userProfile = getenv("USERPROFILE");
            if (userProfile) {
                path = userProfile;
            } else {
                path = "C:\\";
            }
        }
        
        if (_chdir(path.c_str()) != 0) {
            std::cerr << "cd: cannot change to directory '" << path << "'" << std::endl;
        }
    }
    
    void echo(const std::vector<std::string>& tokens) {
        for (size_t i = 1; i < tokens.size(); ++i) {
            if (i > 1) std::cout << " ";
            std::cout << tokens[i];
        }
        std::cout << std::endl;
    }
    
    void listDirectory() {
        system("dir");
    }
    
    void executeExternal(const std::vector<std::string>& tokens) {
        std::string command;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (i > 0) command += " ";
            command += tokens[i];
        }
        
        int result = system(command.c_str());
        if (result == -1) {
            std::cerr << "Failed to execute command: " << command << std::endl;
        }
    }
};

int main() {
    SimpleShell shell;
    shell.run();
    return 0;
}