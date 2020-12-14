#include "remove_duplicates.h"
#include <set>

/// вы воспользовались вектором как универсмальным контейнером, но другие типы контейнеров имеюют преимущества перед ним
/// старайтесь после решения пересмотреть, может можно как то улучшить, и у вектора может быть достаточно дорогая вставка
/// у вас all_docs должен хранить уникальные значения, есть контейнер с возможность хранения уникальных значений, поиск в нем будет гораздо эффективней,
/// чем поиск в векторе
/// слова тоже храните в векторе, вектор не гарантирует, порядок (он может быть различный) и уникальность, лучше тоже использховать другой контейнер
/// в таком виде данная фукция не пройдет требования задачи по сложности, оне будет где-то O(N*N)

void RemoveDuplicates(SearchServer &search_server) {

	std::set<std::set<std::string>> all_docs;

	std::set<int> ids_to_remove;
	for (const int document_id : search_server) {
		auto doc = search_server.GetAllWordsInDocument(document_id);
		if (std::find(all_docs.begin(), all_docs.end(), doc) == all_docs.end()) {
			all_docs.insert(doc);
		} else {
			std::cout << "Found duplicate document id " << document_id << std::endl;
			ids_to_remove.insert(document_id);
		}
	}
	for (const auto id : ids_to_remove) {
		search_server.RemoveDocument(id);
	}
}