#ifndef AI_CLIENT_H
#define AI_CLIENT_H

#include <string>
#include <vector>
#include <memory>

// 检查是否启用了AI功能
#if defined(HAVE_CURL) && defined(HAVE_JSONCPP)
#include <curl/curl.h>
#include <json/json.h>

// 回调函数，用于接收HTTP响应数据
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response);
#endif

// AI客户端类
class AIClient {
public:
    AIClient();
    ~AIClient();
    
    // 初始化AI客户端
    bool initialize();
    
    // 向AI提问
    std::string ask(const std::string& question);
    
    // 设置API密钥
    void setApiKey(const std::string& apiKey);
    
    // 设置AI服务URL
    void setServiceUrl(const std::string& url);
    
    // 设置模型名称
    void setModel(const std::string& model);
    
    // 设置本地模型路径
    void setLocalModelPath(const std::string& path);
    
    // 设置是否使用本地模型
    void setUseLocalModel(bool useLocal);
    
    // 检查是否已配置
    bool isConfigured() const;
    
private:
    std::string apiKey_;
    std::string serviceUrl_;
    std::string model_;
    std::string localModelPath_;
    bool useLocalModel_;
    bool initialized_;
    
#if defined(HAVE_CURL) && defined(HAVE_JSONCPP)
    // 发送请求到AI服务
    std::string sendRequest(const std::string& question);
#endif
    
    // 发送请求到本地模型
    std::string sendLocalRequest(const std::string& question);
    
    // 模拟AI回答
    std::string simulateAnswer(const std::string& question);
};

#endif // AI_CLIENT_H