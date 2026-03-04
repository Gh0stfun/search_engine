#include "inverted_index.h"
#include <sstream>
#include <thread>
#include <algorithm>

std::vector<std::string> InvertedIndex::splitTextIntoWords(const std::string& text) {
    std::vector<std::string> words;
    std::stringstream ss(text);
    std::string word;

    while (ss >> word) {
        words.push_back(word);
    }

    return words;
}

void InvertedIndex::indexDocument(size_t doc_id, const std::string& text) {
    std::vector<std::string> words = splitTextIntoWords(text);
    std::map<std::string, size_t> wordCount;

    for (const auto& word : words) {
        wordCount[word]++;
    }

    std::lock_guard<std::mutex> lock(freq_dict_mutex);
    for (const auto& [word, count] : wordCount) {
        freq_dictionary[word].push_back({doc_id, count});
    }
}

void InvertedIndex::updateDocumentBase(const std::vector<std::string>& input_docs) {
    docs = input_docs;
    freq_dictionary.clear();

    std::vector<std::thread> threads;

    for (size_t i = 0; i < docs.size(); ++i) {
        threads.emplace_back(&InvertedIndex::indexDocument, this, i, docs[i]);
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

std::vector<Entry> InvertedIndex::getWordCount(const std::string& word) {
    auto it = freq_dictionary.find(word);
    if (it != freq_dictionary.end()) {
        return it->second;
    }
    return {};
}