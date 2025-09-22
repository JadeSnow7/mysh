#include "../platform.h"

#ifdef PLATFORM_WINDOWS

#include <iostream>
#include <sstream>
#include <algorithm>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

class WindowsPlatform : public PlatformInterface {
private:
    std::string wideToMultiByte(const std::wstring& wide) {
        if (wide.empty()) return "";
        int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string result(size - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &result[0], size, nullptr, nullptr);
        return result;
    }
    
    std::wstring multiByteToWide(const std::string& multi) {
        if (multi.empty()) return L"";
        int size = MultiByteToWideChar(CP_UTF8, 0, multi.c_str(), -1, nullptr, 0);
        std::wstring result(size - 1, 0);
        MultiByteToWideChar(CP_UTF8, 0, multi.c_str(), -1, &result[0], size);
        return result;
    }

public:
    ProcessId createProcess(const std::string& executable, 
                           const std::vector<std::string>& args) override {
        // 构建命令行
        std::string cmdLine = executable;
        for (const auto& arg : args) {
            cmdLine += " \"" + arg + "\"";
        }
        
        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi = { 0 };
        
        if (CreateProcessA(nullptr, 
                          const_cast<char*>(cmdLine.c_str()),
                          nullptr, nullptr, FALSE, 0, nullptr, nullptr,
                          &si, &pi)) {
            CloseHandle(pi.hThread);
            return pi.dwProcessId;
        }
        
        return 0;
    }
    
    int waitForProcess(ProcessHandle handle, bool background) override {
        if (background) {
            std::cout << "[Background] Process " << handle << " started" << std::endl;
            return 0;
        }
        
        HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, 
                                     FALSE, handle);
        if (hProcess) {
            WaitForSingleObject(hProcess, INFINITE);
            
            DWORD exitCode;
            GetExitCodeProcess(hProcess, &exitCode);
            CloseHandle(hProcess);
            
            return static_cast<int>(exitCode);
        }
        
        return 1;
    }
    
    bool killProcess(ProcessHandle handle) override {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, handle);
        if (hProcess) {
            bool success = TerminateProcess(hProcess, 1);
            CloseHandle(hProcess);
            return success;
        }
        return false;
    }
    
    bool changeDirectory(const std::string& path) override {
        return SetCurrentDirectoryA(path.c_str()) != 0;
    }
    
    std::string getCurrentDirectory() override {
        char buffer[MAX_PATH];
        DWORD length = GetCurrentDirectoryA(MAX_PATH, buffer);
        return length > 0 ? std::string(buffer, length) : "";
    }
    
    bool fileExists(const std::string& path) override {
        DWORD attrib = GetFileAttributesA(path.c_str());
        return attrib != INVALID_FILE_ATTRIBUTES;
    }
    
    bool isExecutable(const std::string& path) override {
        if (!fileExists(path)) return false;
        
        // Windows中检查是否是可执行文件
        std::string ext = path.substr(path.find_last_of('.') + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        return ext == "exe" || ext == "bat" || ext == "cmd" || ext == "com";
    }
    
    bool setupRedirection(const std::string& inputFile, 
                         const std::string& outputFile, 
                         bool appendOutput) override {
        // Windows下的重定向实现（简化版）
        if (!inputFile.empty()) {
            HANDLE hInput = CreateFileA(inputFile.c_str(), GENERIC_READ, 
                                       FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                       FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hInput != INVALID_HANDLE_VALUE) {
                SetStdHandle(STD_INPUT_HANDLE, hInput);
            }
        }
        
        if (!outputFile.empty()) {
            DWORD creationDisposition = appendOutput ? OPEN_ALWAYS : CREATE_ALWAYS;
            HANDLE hOutput = CreateFileA(outputFile.c_str(), GENERIC_WRITE,
                                        FILE_SHARE_READ, nullptr, creationDisposition,
                                        FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hOutput != INVALID_HANDLE_VALUE) {
                if (appendOutput) {
                    SetFilePointer(hOutput, 0, nullptr, FILE_END);
                }
                SetStdHandle(STD_OUTPUT_HANDLE, hOutput);
            }
        }
        
        return true;
    }
    
    std::vector<FileDescriptor> createPipe() override {
        HANDLE hRead, hWrite;
        SECURITY_ATTRIBUTES sa = { sizeof(sa), nullptr, TRUE };
        
        if (CreatePipe(&hRead, &hWrite, &sa, 0)) {
            return {hRead, hWrite};
        }
        
        return {};
    }
    
    std::string getEnvironmentVariable(const std::string& name) override {
        char buffer[32767]; // Windows环境变量最大长度
        DWORD length = GetEnvironmentVariableA(name.c_str(), buffer, sizeof(buffer));
        return length > 0 ? std::string(buffer, length) : "";
    }
    
    void setEnvironmentVariable(const std::string& name, 
                               const std::string& value) override {
        SetEnvironmentVariableA(name.c_str(), value.c_str());
    }
    
    void unsetEnvironmentVariable(const std::string& name) override {
        SetEnvironmentVariableA(name.c_str(), nullptr);
    }
    
    std::string getUserName() override {
        char buffer[256];
        DWORD size = sizeof(buffer);
        if (GetUserNameA(buffer, &size)) {
            return std::string(buffer);
        }
        return "unknown";
    }
    
    std::string getHomeDirectory() override {
        std::string userProfile = getEnvironmentVariable("USERPROFILE");
        if (!userProfile.empty()) return userProfile;
        
        std::string homeDrive = getEnvironmentVariable("HOMEDRIVE");
        std::string homePath = getEnvironmentVariable("HOMEPATH");
        return homeDrive + homePath;
    }
    
    std::vector<std::string> splitPath(const std::string& path) override {
        std::vector<std::string> result;
        std::istringstream iss(path);
        std::string dir;
        
        while (std::getline(iss, dir, PATH_DELIMITER)) {
            if (!dir.empty()) {
                result.push_back(dir);
            }
        }
        
        return result;
    }
    
    std::string findExecutable(const std::string& command) override {
        // 如果包含路径分隔符，直接检查
        if (command.find(PATH_SEPARATOR) != std::string::npos) {
            return isExecutable(command) ? command : "";
        }
        
        // 尝试添加常见扩展名
        std::vector<std::string> extensions = {"", ".exe", ".bat", ".cmd", ".com"};
        
        // 先检查当前目录
        for (const auto& ext : extensions) {
            std::string testPath = command + ext;
            if (isExecutable(testPath)) {
                return testPath;
            }
        }
        
        // 在PATH中搜索
        std::string path = getEnvironmentVariable("PATH");
        auto paths = splitPath(path);
        
        for (const auto& dir : paths) {
            for (const auto& ext : extensions) {
                std::string fullPath = dir + PATH_SEPARATOR + command + ext;
                if (isExecutable(fullPath)) {
                    return fullPath;
                }
            }
        }
        
        return "";
    }
    
    void setupSignalHandlers() override {
        // Windows下的信号处理（简化）
        SetConsoleCtrlHandler([](DWORD dwCtrlType) -> BOOL {
            switch (dwCtrlType) {
                case CTRL_C_EVENT:
                case CTRL_BREAK_EVENT:
                    return TRUE; // 忽略中断信号
                default:
                    return FALSE;
            }
        }, TRUE);
    }
};

#endif // PLATFORM_WINDOWS