#pragma once

#include "inverted_index.h"
#include <vector>
#include <string>
#include <map>
#include <set>

struct RelativeIndex {
    size_t doc_id;
    float rank;

    bool operator==(const RelativeIndex& other) const {
        return (doc_id == other.doc_id && std::abs(rank - other.rank) < 1e-6);
    }
};

class SearchServer {
public:
    explicit SearchServer(InvertedIndex& idx) : _index(idx) {}

    std::vector<std::vector<RelativeIndex>> search(const std::vector<std::string>& queries_input);

private:
    InvertedIndex& _index;

    std::vector<std::string> getUniqueWords(const std::string& query);

    size_t calculateAbsoluteRelevance(size_t doc_id, const std::vector<std::string>& words);

    std::set<size_t> findRelevantDocuments(const std::vector<std::string>& words);
};