#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>

struct Entry {
    size_t doc_id;
    size_t count;

    bool operator==(const Entry& other) const {
        return (doc_id == other.doc_id && count == other.count);
    }
};

class InvertedIndex {
public:
    InvertedIndex() = default;

    void updateDocumentBase(const std::vector<std::string>& input_docs);

    std::vector<Entry> getWordCount(const std::string& word);

    size_t getDocsCount() const { return docs.size(); }

private:
    std::vector<std::string> docs;
    std::map<std::string, std::vector<Entry>> freq_dictionary;
    std::mutex freq_dict_mutex;

    std::vector<std::string> splitTextIntoWords(const std::string& text);

    void indexDocument(size_t doc_id, const std::string& text);
};