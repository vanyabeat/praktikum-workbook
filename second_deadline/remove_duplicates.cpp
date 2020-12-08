#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer &search_server) {

	std::vector<std::vector<std::string>> all_docs;
	std::vector<int> ids_to_remove;
	for (const int document_id : search_server) {
		auto doc = search_server.GetAllWordsInDocument(document_id);
		if (std::find(all_docs.begin(), all_docs.end(), doc) == all_docs.end()) {
			all_docs.push_back(doc);
		} else {
			std::cout << "Found duplicate document id " << document_id << std::endl;
			ids_to_remove.push_back(document_id);
		}
	}
	for (const auto id : ids_to_remove) {
		search_server.RemoveDocument(id);
	}
}