#include "ai_client.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

#if defined(HAVE_CURL) && defined(HAVE_JSONCPP)
#include <curl/curl.h>
#include <json/json.h>

// 回调函数，用于接收HTTP响应数据
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}
#endif

AIClient::AIClient() : initialized_(false), model_("qwen-plus"), useLocalModel_(false) {
    // 默认服务URL（阿里云百炼平台）
    serviceUrl_ = "https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation";
}

AIClient::~AIClient() = default;

bool AIClient::initialize() {
    // 检查是否使用本地模型
    if (useLocalModel_) {
        // 检查本地模型路径是否设置
        if (localModelPath_.empty()) {
            // 尝试从环境变量获取本地模型路径
            const char* envLocalModelPath = std::getenv("LOCAL_AI_MODEL_PATH");
            if (envLocalModelPath) {
                localModelPath_ = std::string(envLocalModelPath);
            } else {
                std::cerr << "AI Client: Local model path not set. Please set LOCAL_AI_MODEL_PATH environment variable." << std::endl;
                return false;
            }
        }
        
        // 检查模型文件是否存在
        std::ifstream modelFile(localModelPath_);
        if (!modelFile.good()) {
            std::cerr << "AI Client: Local model file not found at " << localModelPath_ << std::endl;
            return false;
        }
        modelFile.close();
    } else {
        // 检查是否已配置API密钥
        if (apiKey_.empty()) {
            // 尝试从环境变量获取API密钥
            const char* envApiKey = std::getenv("AI_API_KEY");
            if (envApiKey) {
                apiKey_ = std::string(envApiKey);
            } else {
                std::cerr << "AI Client: API key not set. Please set AI_API_KEY environment variable." << std::endl;
                return false;
            }
        }
    }
    
#if defined(HAVE_CURL) && defined(HAVE_JSONCPP)
    // 初始化libcurl
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "AI Client: Failed to initialize curl." << std::endl;
        return false;
    }
    curl_easy_cleanup(curl);
#endif
    
    initialized_ = true;
    return true;
}

std::string AIClient::ask(const std::string& question) {
    if (!initialized_) {
        if (!initialize()) {
            return "AI Client not properly configured.";
        }
    }
    
    // 检查是否使用本地模型
    if (useLocalModel_) {
        return sendLocalRequest(question);
    }
    
#if defined(HAVE_CURL) && defined(HAVE_JSONCPP)
    // 发送请求到AI服务
    return sendRequest(question);
#else
    // 如果没有启用AI功能，返回模拟回答
    return simulateAnswer(question);
#endif
}

void AIClient::setApiKey(const std::string& apiKey) {
    apiKey_ = apiKey;
}

void AIClient::setServiceUrl(const std::string& url) {
    serviceUrl_ = url;
}

void AIClient::setModel(const std::string& model) {
    model_ = model;
}

void AIClient::setLocalModelPath(const std::string& path) {
    localModelPath_ = path;
}

void AIClient::setUseLocalModel(bool useLocal) {
    useLocalModel_ = useLocal;
}

bool AIClient::isConfigured() const {
    return useLocalModel_ ? !localModelPath_.empty() : !apiKey_.empty();
}

#if defined(HAVE_CURL) && defined(HAVE_JSONCPP)
std::string AIClient::sendRequest(const std::string& question) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "Failed to initialize curl for AI request.";
    }
    
    // 构造JSON请求
    Json::Value jsonRequest;
    Json::Value messages(Json::arrayValue);
    
    Json::Value systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "你是一个有用的助手，集成在MyShell中。MyShell是一个用C++开发的跨平台shell工具，支持Linux、Windows和macOS，提供了完整的命令行交互环境。";
    messages.append(systemMessage);
    
    Json::Value userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = question;
    messages.append(userMessage);
    
    jsonRequest["model"] = model_;
    jsonRequest["input"]["messages"] = messages;
    
    Json::StreamWriterBuilder builder;
    std::string requestBody = Json::writeString(builder, jsonRequest);
    
    // 设置HTTP请求
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, serviceUrl_.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    // 设置HTTP头
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string authHeader = "Authorization: Bearer " + apiKey_;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "X-DashScope-SSE: disable");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // 执行HTTP请求
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::string errorMsg = "Failed to send request: " + std::string(curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return errorMsg;
    }
    
    // 清理资源
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    // 解析JSON响应
    Json::CharReaderBuilder readerBuilder;
    Json::Value jsonResponse;
    std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
    std::string errors;
    
    bool parsingSuccessful = reader->parse(response.c_str(), response.c_str() + response.size(), &jsonResponse, &errors);
    if (!parsingSuccessful) {
        return "Failed to parse response: " + errors;
    }
    
    // 提取并返回AI的回答
    if (jsonResponse.isMember("output") && jsonResponse["output"].isMember("text")) {
        return jsonResponse["output"]["text"].asString();
    } else if (jsonResponse.isMember("error") && jsonResponse["error"].isMember("message")) {
        return "AI service error: " + jsonResponse["error"]["message"].asString();
    } else {
        return "Unexpected response format from AI service.";
    }
}
#endif

std::string AIClient::sendLocalRequest(const std::string& question) {
    // 这里是一个简化的实现，实际使用时可能需要根据具体的本地模型API进行调整
    // 例如，如果是使用Ollama，可以发送HTTP请求到http://localhost:11434/api/generate
    
    // 模拟本地模型响应
    std::string lowerQuestion = question;
    std::transform(lowerQuestion.begin(), lowerQuestion.end(), lowerQuestion.begin(), ::tolower);
    
    // 根据问题内容生成本地模型的回答
    if (lowerQuestion.find("hello") != std::string::npos || 
        lowerQuestion.find("hi") != std::string::npos ||
        lowerQuestion.find("你好") != std::string::npos) {
        return "Hello! I'm an AI assistant running on your local machine. How can I help you today?";
    } else if (lowerQuestion.find("help") != std::string::npos) {
        return "I'm a locally running AI assistant that can answer questions, provide information, and help with various tasks. "
               "You can ask me anything and I'll do my best to assist you! Since I'm running locally, I don't have access to real-time information.";
    } else if (lowerQuestion.find("weather") != std::string::npos) {
        return "I'm sorry, but I don't have access to real-time weather information. "
               "As a locally running AI model, I don't have internet access to fetch current weather data.";
    } else if (lowerQuestion.find("time") != std::string::npos) {
        return "I don't have access to real-time information. You can use the 'date' command in MyShell to check the current time.";
    } else if (lowerQuestion.find("myshell") != std::string::npos) {
        return "MyShell is a cross-platform shell tool developed in C++. It supports command-line interaction, "
               "built-in commands, external program execution, input/output redirection, and pipe operations. "
               "It also features intelligent auto-completion and syntax highlighting. I'm integrated as an AI assistant in MyShell.";
    } else {
        // 默认回答
        return "That's an interesting question! As a locally running AI assistant in MyShell, I can provide information on a wide range of topics. "
               "Keep in mind that I'm running on your local machine, so I don't have access to real-time information or the internet. "
               "Feel free to ask me anything else!";
    }
}

std::string AIClient::simulateAnswer(const std::string& question) {
    // 转换问题为小写以便比较
    std::string lowerQuestion = question;
    std::transform(lowerQuestion.begin(), lowerQuestion.end(), lowerQuestion.begin(), ::tolower);
    
    // 根据问题内容生成模拟回答
    if (lowerQuestion.find("hello") != std::string::npos || 
        lowerQuestion.find("hi") != std::string::npos ||
        lowerQuestion.find("你好") != std::string::npos) {
        return "Hello! I'm an AI assistant integrated into MyShell. How can I help you today?";
    } else if (lowerQuestion.find("help") != std::string::npos) {
        return "I'm an AI assistant that can answer questions, provide information, and help with various tasks. "
               "You can ask me anything and I'll do my best to assist you!";
    } else if (lowerQuestion.find("weather") != std::string::npos) {
        return "I'm sorry, but I don't have access to real-time weather information. "
               "You might want to check a weather service or app for current conditions.";
    } else if (lowerQuestion.find("time") != std::string::npos) {
        return "I don't have access to real-time information. You can use the 'date' command in MyShell to check the current time.";
    } else if (lowerQuestion.find("myshell") != std::string::npos) {
        return "MyShell is a cross-platform shell tool developed in C++. It supports command-line interaction, "
               "built-in commands, external program execution, input/output redirection, and pipe operations. "
               "It also features intelligent auto-completion and syntax highlighting.";
    } else if (lowerQuestion.find("thank") != std::string::npos || 
               lowerQuestion.find("thanks") != std::string::npos) {
        return "You're welcome! Is there anything else I can help you with?";
    } else {
        // 默认回答
        return "That's an interesting question! As an AI assistant in MyShell, I can provide information on a wide range of topics. "
               "For more complex queries, you might want to connect to a more advanced AI service. "
               "Feel free to ask me anything else!";
    }
}