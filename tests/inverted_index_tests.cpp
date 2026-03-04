#include <gtest/gtest.h>
#include "inverted_index.h"

using namespace std;

void testInvertedIndexFunctionality(
    const vector<string>& docs,
    const vector<string>& requests,
    const vector<vector<Entry>>& expected
) {
    vector<vector<Entry>> result;
    InvertedIndex idx;
    idx.updateDocumentBase(docs);

    for (auto& request : requests) {
        vector<Entry> word_count = idx.getWordCount(request);
        result.push_back(word_count);
    }

    ASSERT_EQ(result, expected);
}

TEST(InvertedIndexTest, TestBasic) {
    const vector<string> docs = {
        "london is the capital of great britain",
        "big ben is the nickname for the Great bell of the striking clock"
    };
    const vector<string> requests = {"london", "the"};
    const vector<vector<Entry>> expected = {
        {{0, 1}},
        {{0, 1}, {1, 3}}
    };

    testInvertedIndexFunctionality(docs, requests, expected);
}

TEST(InvertedIndexTest, TestMissingWord) {
    const vector<string> docs = {
        "milk sugar salt",
        "milk a milk b milk c milk d"
    };
    const vector<string> requests = {"milk", "coffee", "salt"};
    const vector<vector<Entry>> expected = {
        {{0, 1}, {1, 4}},
        {},
        {{0, 1}}
    };

    testInvertedIndexFunctionality(docs, requests, expected);
}

TEST(InvertedIndexTest, TestMultipleDocuments) {
    const vector<string> docs = {
        "apple banana",
        "apple apple banana",
        "banana banana",
        "apple"
    };
    const vector<string> requests = {"apple", "banana"};
    const vector<vector<Entry>> expected = {
        {{0, 1}, {1, 2}, {3, 1}},
        {{0, 1}, {1, 1}, {2, 2}}
    };

    testInvertedIndexFunctionality(docs, requests, expected);
}