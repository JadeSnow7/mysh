#ifndef HISTORY_H
#define HISTORY_H

#include <string>
#include <vector>
#include <fstream>

class History {
public:
    History();
    ~History();
    
    // 添加命令到历史记录
    void addCommand(const std::string& command);
    
    // 获取历史记录
    std::vector<std::string> getHistory() const;
    
    // 获取指定索引的命令
    std::string getCommand(size_t index) const;
    
    // 获取历史记录大小
    size_t size() const;
    
    // 清空历史记录
    void clear();
    
    // 显示历史记录
    void show() const;
    
    // 搜索历史记录
    std::vector<size_t> search(const std::string& pattern) const;
    
    // 设置最大历史记录数
    void setMaxSize(size_t maxSize);
    
private:
    std::vector<std::string> commands;
    size_t maxHistorySize;
    std::string historyFile;
    
    // 加载历史记录文件
    void loadFromFile();
    
    // 保存到历史记录文件
    void saveToFile();
    
    // 获取历史文件路径
    std::string getHistoryFilePath();
    
    // 修剪历史记录（保持在最大大小内）
    void trimHistory();
};

#endif // HISTORY_H