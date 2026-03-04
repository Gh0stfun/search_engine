#include <gtest/gtest.h>
#include "inverted_index.h"
#include "search_server.h"

using namespace std;

TEST(SearchServerTest, TestSimple) {
    const vector<string> docs = {
        "milk milk milk milk water water water",
        "milk water water",
        "milk milk milk milk milk water water water water water",
        "americano cappuccino"
    };
    const vector<string> request = {"milk water", "sugar"};
    
    InvertedIndex idx;
    idx.updateDocumentBase(docs);
    SearchServer srv(idx);
    
    vector<vector<RelativeIndex>> result = srv.search(request);

    ASSERT_EQ(result[0].size(), 3);

    sort(result[0].begin(), result[0].end(),
         [](const RelativeIndex& a, const RelativeIndex& b) {
             return a.doc_id < b.doc_id;
         });
    
    EXPECT_EQ(result[0][0].doc_id, 0);
    EXPECT_NEAR(result[0][0].rank, 0.7f, 0.01f);
    EXPECT_EQ(result[0][1].doc_id, 1);
    EXPECT_NEAR(result[0][1].rank, 0.3f, 0.01f);
    EXPECT_EQ(result[0][2].doc_id, 2);
    EXPECT_NEAR(result[0][2].rank, 1.0f, 0.01f);

    EXPECT_TRUE(result[1].empty());
}

TEST(SearchServerTest, TestTop5) {
    const vector<string> docs = {
        "london is the capital of great britain",
        "paris is the capital of france",
        "berlin is the capital of germany",
        "rome is the capital of italy",
        "madrid is the capital of spain",
        "lisboa is the capital of portugal",
        "bern is the capital of switzerland",
        "moscow is the capital of russia",
        "kiev is the capital of ukraine",
        "minsk is the capital of belarus",
        "astana is the capital of kazakhstan",
        "beijing is the capital of china",
        "tokyo is the capital of japan",
        "bangkok is the capital of thailand",
        "welcome to moscow the capital of russia the third rome",
        "amsterdam is the capital of netherlands",
        "helsinki is the capital of finland",
        "oslo is the capital of norway",
        "stockholm is the capital of sweden",
        "riga is the capital of latvia",
        "tallinn is the capital of estonia",
        "warsaw is the capital of poland"
    };
    
    const vector<string> request = {"moscow is the capital of russia"};
    
    InvertedIndex idx;
    idx.updateDocumentBase(docs);
    SearchServer srv(idx);
    
    vector<vector<RelativeIndex>> result = srv.search(request);

    ASSERT_GE(result[0].size(), 5);

    bool foundDoc7 = false;
    bool foundDoc14 = false;
    
    for (size_t i = 0; i < 5 && i < result[0].size(); ++i) {
        if (result[0][i].doc_id == 7) foundDoc7 = true;
        if (result[0][i].doc_id == 14) foundDoc14 = true;
    }
    
    EXPECT_TRUE(foundDoc7);
    EXPECT_TRUE(foundDoc14);
}