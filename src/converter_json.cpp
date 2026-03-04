#include "converter_json.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

bool ConverterJSON::fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

std::string ConverterJSON::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void ConverterJSON::writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    file << content;
}

std::string ConverterJSON::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

std::string ConverterJSON::removeQuotes(const std::string& str) {
    std::string result = trim(str);
    if (result.size() >= 2 && result.front() == '"' && result.back() == '"') {
        return result.substr(1, result.size() - 2);
    }
    return result;
}

size_t ConverterJSON::findMatchingBrace(const std::string& str, size_t pos) {
    if (pos >= str.length() || (str[pos] != '{' && str[pos] != '[')) {
        return std::string::npos;
    }

    char openChar = str[pos];
    char closeChar = (openChar == '{') ? '}' : ']';

    int depth = 1;
    bool inString = false;

    for (size_t i = pos + 1; i < str.length(); ++i) {
        if (str[i] == '"' && (i == 0 || str[i-1] != '\\')) {
            inString = !inString;
        }

        if (!inString) {
            if (str[i] == openChar) {
                depth++;
            } else if (str[i] == closeChar) {
                depth--;
                if (depth == 0) {
                    return i;
                }
            }
        }
    }

    return std::string::npos;
}

std::string ConverterJSON::getJsonValue(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);

    if (keyPos == std::string::npos) {
        return "";
    }

    size_t colonPos = json.find(':', keyPos + searchKey.length());
    if (colonPos == std::string::npos) {
        return "";
    }

    size_t valueStart = colonPos + 1;
    while (valueStart < json.length() && std::isspace(json[valueStart])) {
        valueStart++;
    }

    if (valueStart >= json.length()) {
        return "";
    }

    if (json[valueStart] == '"') {
        size_t valueEnd = valueStart + 1;
        bool inEscape = false;

        while (valueEnd < json.length()) {
            if (json[valueEnd] == '\\' && !inEscape) {
                inEscape = true;
            } else if (json[valueEnd] == '"' && !inEscape) {
                break;
            } else {
                inEscape = false;
            }
            valueEnd++;
        }

        if (valueEnd < json.length()) {
            return json.substr(valueStart, valueEnd - valueStart + 1);
        }
    } else if (json[valueStart] == '{' || json[valueStart] == '[') {
        size_t valueEnd = findMatchingBrace(json, valueStart);
        if (valueEnd != std::string::npos) {
            return json.substr(valueStart, valueEnd - valueStart + 1);
        }
    } else {
        size_t valueEnd = valueStart;
        while (valueEnd < json.length() &&
               (std::isdigit(json[valueEnd]) || json[valueEnd] == '.' ||
                json[valueEnd] == '-' || json[valueEnd] == '+' ||
                json[valueEnd] == 'e' || json[valueEnd] == 'E' ||
                json[valueEnd] == 't' || json[valueEnd] == 'f' ||
                json[valueEnd] == 'n')) {
            valueEnd++;
        }

        if (valueEnd > valueStart) {
            return json.substr(valueStart, valueEnd - valueStart);
        }
    }

    return "";
}

std::map<std::string, std::string> ConverterJSON::parseJsonObject(const std::string& json, const std::string& objectName) {
    std::map<std::string, std::string> result;

    std::string objectJson;

    if (objectName.empty()) {
        size_t start = json.find('{');
        size_t end = findMatchingBrace(json, start);
        if (start != std::string::npos && end != std::string::npos) {
            objectJson = json.substr(start + 1, end - start - 1);
        }
    } else {
        std::string objValue = getJsonValue(json, objectName);
        if (!objValue.empty() && objValue.front() == '{') {
            objectJson = objValue.substr(1, objValue.length() - 2);
        }
    }

    if (objectJson.empty()) {
        return result;
    }

    size_t pos = 0;
    while (pos < objectJson.length()) {
        while (pos < objectJson.length() && std::isspace(objectJson[pos])) pos++;
        if (pos >= objectJson.length()) break;

        if (objectJson[pos] != '"') break;
        size_t keyStart = pos;
        size_t keyEnd = pos + 1;
        bool inEscape = false;

        while (keyEnd < objectJson.length()) {
            if (objectJson[keyEnd] == '\\' && !inEscape) {
                inEscape = true;
            } else if (objectJson[keyEnd] == '"' && !inEscape) {
                break;
            } else {
                inEscape = false;
            }
            keyEnd++;
        }

        if (keyEnd >= objectJson.length()) break;

        std::string key = removeQuotes(objectJson.substr(keyStart, keyEnd - keyStart + 1));
        pos = keyEnd + 1;

        while (pos < objectJson.length() && std::isspace(objectJson[pos])) pos++;
        if (pos >= objectJson.length() || objectJson[pos] != ':') break;
        pos++;

        while (pos < objectJson.length() && std::isspace(objectJson[pos])) pos++;
        if (pos >= objectJson.length()) break;

        size_t valueStart = pos;
        size_t valueEnd;

        if (objectJson[pos] == '"') {
            valueEnd = pos + 1;
            inEscape = false;

            while (valueEnd < objectJson.length()) {
                if (objectJson[valueEnd] == '\\' && !inEscape) {
                    inEscape = true;
                } else if (objectJson[valueEnd] == '"' && !inEscape) {
                    break;
                } else {
                    inEscape = false;
                }
                valueEnd++;
            }

            if (valueEnd < objectJson.length()) {
                result[key] = objectJson.substr(valueStart, valueEnd - valueStart + 1);
                pos = valueEnd + 1;
            } else {
                break;
            }
        } else if (objectJson[pos] == '{' || objectJson[pos] == '[') {
            valueEnd = findMatchingBrace(objectJson, pos);
            if (valueEnd != std::string::npos) {
                result[key] = objectJson.substr(valueStart, valueEnd - valueStart + 1);
                pos = valueEnd + 1;
            } else {
                break;
            }
        } else {
            valueEnd = valueStart;
            while (valueEnd < objectJson.length() &&
                   !std::isspace(objectJson[valueEnd]) &&
                   objectJson[valueEnd] != ',' &&
                   objectJson[valueEnd] != '}') {
                valueEnd++;
            }

            result[key] = objectJson.substr(valueStart, valueEnd - valueStart);
            pos = valueEnd;
        }

        while (pos < objectJson.length() && std::isspace(objectJson[pos])) pos++;
        if (pos < objectJson.length() && objectJson[pos] == ',') {
            pos++;
        }
    }

    return result;
}

std::vector<std::string> ConverterJSON::parseJsonArray(const std::string& json, const std::string& arrayName) {
    std::vector<std::string> result;

    std::string arrayValue = getJsonValue(json, arrayName);

    if (arrayValue.empty() || arrayValue.front() != '[') {
        return result;
    }

    std::string arrayContent = arrayValue.substr(1, arrayValue.length() - 2);

    size_t pos = 0;
    while (pos < arrayContent.length()) {
        while (pos < arrayContent.length() && std::isspace(arrayContent[pos])) pos++;
        if (pos >= arrayContent.length()) break;

        size_t elementStart = pos;
        size_t elementEnd;

        if (arrayContent[pos] == '"') {
            elementEnd = pos + 1;
            bool inEscape = false;

            while (elementEnd < arrayContent.length()) {
                if (arrayContent[elementEnd] == '\\' && !inEscape) {
                    inEscape = true;
                } else if (arrayContent[elementEnd] == '"' && !inEscape) {
                    break;
                } else {
                    inEscape = false;
                }
                elementEnd++;
            }

            if (elementEnd < arrayContent.length()) {
                std::string element = arrayContent.substr(elementStart, elementEnd - elementStart + 1);
                result.push_back(removeQuotes(element));
                pos = elementEnd + 1;
            } else {
                break;
            }
        } else if (arrayContent[pos] == '{' || arrayContent[pos] == '[') {
            elementEnd = findMatchingBrace(arrayContent, pos);
            if (elementEnd != std::string::npos) {
                result.push_back(arrayContent.substr(elementStart, elementEnd - elementStart + 1));
                pos = elementEnd + 1;
            } else {
                break;
            }
        } else {
            elementEnd = elementStart;
            while (elementEnd < arrayContent.length() &&
                   !std::isspace(arrayContent[elementEnd]) &&
                   arrayContent[elementEnd] != ',') {
                elementEnd++;
            }

            result.push_back(arrayContent.substr(elementStart, elementEnd - elementStart));
            pos = elementEnd;
        }

        while (pos < arrayContent.length() && std::isspace(arrayContent[pos])) pos++;
        if (pos < arrayContent.length() && arrayContent[pos] == ',') {
            pos++;
        }
    }

    return result;
}

std::string ConverterJSON::createJsonResponse(const std::vector<std::vector<std::pair<int, float>>>& answers) {
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"answers\": {\n";

    for (size_t i = 0; i < answers.size(); ++i) {
        std::string requestId = "request" + std::string(3 - std::to_string(i + 1).length(), '0') +
                                 std::to_string(i + 1);

        ss << "    \"" << requestId << "\": {\n";

        if (answers[i].empty()) {
            ss << "      \"result\": \"false\"\n";
        } else {
            ss << "      \"result\": \"true\",\n";
            ss << "      \"relevance\": [\n";

            for (size_t j = 0; j < answers[i].size(); ++j) {
                ss << "        {\n";
                ss << "          \"docid\": " << answers[i][j].first << ",\n";
                ss << "          \"rank\": " << answers[i][j].second << "\n";
                ss << "        }";
                if (j < answers[i].size() - 1) {
                    ss << ",";
                }
                ss << "\n";
            }

            ss << "      ]\n";
        }

        ss << "    }";
        if (i < answers.size() - 1) {
            ss << ",";
        }
        ss << "\n";
    }

    ss << "  }\n";
    ss << "}\n";

    return ss.str();
}

std::vector<std::string> ConverterJSON::getTextDocuments() {
    if (!fileExists(configPath)) {
        throw std::runtime_error("config file is missing");
    }

    std::string configContent = readFile(configPath);
    if (configContent.empty()) {
        throw std::runtime_error("config file is empty");
    }

    auto configObj = parseJsonObject(configContent, "config");
    if (configObj.empty()) {
        throw std::runtime_error("config file is empty");
    }

    std::vector<std::string> documents;
    std::vector<std::string> filePaths = parseJsonArray(configContent, "files");

    for (const auto& filePath : filePaths) {
        std::ifstream file(filePath);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            documents.push_back(buffer.str());
            file.close();
        } else {
            std::cerr << "File not found: " << filePath << std::endl;
        }
    }

    return documents;
}

int ConverterJSON::getResponsesLimit() {
    if (!fileExists(configPath)) {
        return 5;
    }

    std::string configContent = readFile(configPath);
    auto configObj = parseJsonObject(configContent, "config");

    auto it = configObj.find("max_responses");
    if (it != configObj.end()) {
        return std::stoi(it->second);
    }

    return 5;
}

std::vector<std::string> ConverterJSON::getRequests() {
    if (!fileExists(requestsPath)) {
        return {};
    }

    std::string requestsContent = readFile(requestsPath);
    return parseJsonArray(requestsContent, "requests");
}

void ConverterJSON::putAnswers(const std::vector<std::vector<std::pair<int, float>>>& answers) {
    std::string jsonResponse = createJsonResponse(answers);
    writeFile(answersPath, jsonResponse);
}

std::string ConverterJSON::getEngineName() {
    if (!fileExists(configPath)) {
        return "SearchEngine";
    }

    std::string configContent = readFile(configPath);
    auto configObj = parseJsonObject(configContent, "config");

    auto it = configObj.find("name");
    if (it != configObj.end()) {
        return removeQuotes(it->second);
    }

    return "SearchEngine";
}

bool ConverterJSON::checkConfigVersion() {
    if (!fileExists(configPath)) {
        return false;
    }

    std::string configContent = readFile(configPath);
    auto configObj = parseJsonObject(configContent, "config");

    auto it = configObj.find("version");
    if (it != configObj.end()) {
        return removeQuotes(it->second) == appVersion;
    }

    return false;
}