#include "search_server.h"
#include <sstream>
#include <algorithm>
#include <map>
#include <set>
#include <cmath>
#include <iostream>

std::vector<std::string> SearchServer::getUniqueWords(const std::string& query) {
    std::vector<std::string> words;
    std::stringstream ss(query);
    std::string word;
    std::set<std::string> uniqueWords;

    while (ss >> word) {
        uniqueWords.insert(word);
    }

    words.assign(uniqueWords.begin(), uniqueWords.end());
    return words;
}

size_t SearchServer::calculateAbsoluteRelevance(size_t doc_id, const std::vector<std::string>& words) {
    size_t relevance = 0;

    for (const auto& word : words) {
        auto entries = _index.getWordCount(word);
        for (const auto& entry : entries) {
            if (entry.doc_id == doc_id) {
                relevance += entry.count;
                break;
            }
        }
    }

    return relevance;
}

std::set<size_t> SearchServer::findRelevantDocuments(const std::vector<std::string>& words) {
    if (words.empty()) {
        return {};
    }

    std::vector<std::pair<std::string, size_t>> wordFrequency;
    for (const auto& word : words) {
        size_t frequency = 0;
        auto entries = _index.getWordCount(word);
        for (const auto& entry : entries) {
            frequency += entry.count;
        }
        wordFrequency.emplace_back(word, frequency);
    }

    std::sort(wordFrequency.begin(), wordFrequency.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });

    std::set<size_t> relevantDocs;
    auto firstWordEntries = _index.getWordCount(wordFrequency[0].first);
    for (const auto& entry : firstWordEntries) {
        relevantDocs.insert(entry.doc_id);
    }

    for (size_t i = 1; i < wordFrequency.size(); ++i) {
        std::set<size_t> docsWithWord;
        auto entries = _index.getWordCount(wordFrequency[i].first);
        for (const auto& entry : entries) {
            docsWithWord.insert(entry.doc_id);
        }

        std::set<size_t> intersection;
        std::set_intersection(relevantDocs.begin(), relevantDocs.end(),
                              docsWithWord.begin(), docsWithWord.end(),
                              std::inserter(intersection, intersection.begin()));
        relevantDocs = std::move(intersection);

        if (relevantDocs.empty()) {
            break;
        }
    }

    return relevantDocs;
}

std::vector<std::vector<RelativeIndex>> SearchServer::search(const std::vector<std::string>& queries_input) {
    std::vector<std::vector<RelativeIndex>> results;

    for (const auto& query : queries_input) {
        std::vector<RelativeIndex> queryResults;

        std::vector<std::string> words = getUniqueWords(query);

        if (words.empty()) {
            results.push_back({});
            continue;
        }

        std::set<size_t> relevantDocs = findRelevantDocuments(words);

        if (relevantDocs.empty()) {
            results.push_back({});
            continue;
        }

        std::map<size_t, size_t> absoluteRelevance;
        size_t maxRelevance = 0;

        for (size_t doc_id : relevantDocs) {
            size_t relevance = calculateAbsoluteRelevance(doc_id, words);
            absoluteRelevance[doc_id] = relevance;
            maxRelevance = std::max(maxRelevance, relevance);
        }

        for (const auto& [doc_id, relevance] : absoluteRelevance) {
            if (maxRelevance > 0) {
                queryResults.push_back({
                    doc_id,
                    static_cast<float>(relevance) / static_cast<float>(maxRelevance)
                });
            }
        }

        std::sort(queryResults.begin(), queryResults.end(),
                  [](const RelativeIndex& a, const RelativeIndex& b) {
                      return a.rank > b.rank;
                  });

        results.push_back(queryResults);
    }

    return results;
}