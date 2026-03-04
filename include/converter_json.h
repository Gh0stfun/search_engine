#pragma once

#include <string>
#include <vector>
#include <map>

class ConverterJSON {
public:
    ConverterJSON() = default;

    std::vector<std::string> getTextDocuments();

    int getResponsesLimit();

    std::vector<std::string> getRequests();

    void putAnswers(const std::vector<std::vector<std::pair<int, float>>>& answers);

    std::string getEngineName();

    bool checkConfigVersion();

private:
    const std::string configPath = "../config/config.json";
    const std::string requestsPath = "../config/requests.json";
    const std::string answersPath = "../config/answers.json";
    const std::string appVersion = "0.1";

    std::string readFile(const std::string& path);
    std::map<std::string, std::string> parseJsonObject(const std::string& json, const std::string& objectName);
    std::vector<std::string> parseJsonArray(const std::string& json, const std::string& arrayName);
    std::string getJsonValue(const std::string& json, const std::string& key);
    std::string createJsonResponse(const std::vector<std::vector<std::pair<int, float>>>& answers);
    bool fileExists(const std::string& path);
    void writeFile(const std::string& path, const std::string& content);

    size_t findMatchingBrace(const std::string& str, size_t pos);
    std::string trim(const std::string& str);
    std::string removeQuotes(const std::string& str);
};