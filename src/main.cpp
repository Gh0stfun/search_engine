#include "converter_json.h"
#include "inverted_index.h"
#include "search_server.h"
#include <iostream>

int main() {
    try {
        ConverterJSON converter;

        std::cout << "Starting " << converter.getEngineName() << std::endl;

        if (!converter.checkConfigVersion()) {
            std::cerr << "config.json has incorrect file version" << std::endl;
            return 1;
        }

        std::cout << "Loading documents..." << std::endl;
        std::vector<std::string> documents = converter.getTextDocuments();

        if (documents.empty()) {
            std::cerr << "No valid documents found" << std::endl;
            return 1;
        }

        std::cout << "Loaded " << documents.size() << " documents" << std::endl;

        std::cout << "Indexing documents..." << std::endl;
        InvertedIndex index;
        index.updateDocumentBase(documents);

        std::vector<std::string> requests = converter.getRequests();

        if (requests.empty()) {
            std::cout << "No requests to process" << std::endl;
            return 0;
        }

        std::cout << "Processing " << requests.size() << " requests..." << std::endl;

        SearchServer searchServer(index);
        std::vector<std::vector<RelativeIndex>> searchResults = searchServer.search(requests);

        int limit = converter.getResponsesLimit();
        std::vector<std::vector<std::pair<int, float>>> limitedResults;

        for (const auto& queryResult : searchResults) {
            std::vector<std::pair<int, float>> limitedQuery;
            int count = 0;
            for (const auto& result : queryResult) {
                if (count >= limit) break;
                limitedQuery.emplace_back(result.doc_id, result.rank);
                count++;
            }
            limitedResults.push_back(limitedQuery);
        }

        converter.putAnswers(limitedResults);
        std::cout << "Search completed. Results saved to answers.json" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}