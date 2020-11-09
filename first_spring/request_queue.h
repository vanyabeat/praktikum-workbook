#pragma once
#include "search_server.h"		/// в начале лучше поключать системные заголовочные файлы
#include <deque>

class RequestQueue {
public:
	explicit RequestQueue(const SearchServer &search_server);
	// сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
	template<typename DocumentPredicate>	/// укажу только тут, но в остальных классах таже проблема, реализации шаблонных и инлайновых методов лучше убирать из класса, можно разместить под классом
	std::vector<Document> AddFindRequest(const std::string &raw_query, DocumentPredicate document_predicate) {
		auto request = search_server_.FindTopDocuments(raw_query, document_predicate);
		QueryResult result(request);
		AddQueue(result);
		return request;
	}

	std::vector<Document> AddFindRequest(const std::string &raw_query, DocumentStatus status);

	std::vector<Document> AddFindRequest(const std::string &raw_query);

	size_t GetNoResultRequests() const;

private:
	struct QueryResult {
		std::vector<Document> result;
	};
	std::deque<QueryResult> requests_;

	const static int sec_in_day_ = 1440;
	// возможно, здесь вам понадобится что-то ещё
	size_t count_empty_query;	/// придерживайтесь порядка именования полей, у вас принято в классах в названии полей использовать символ подчеркивания в конце названия
	const SearchServer &search_server_;

	// метод добавления ответа сервера в очередь
	void AddQueue(const QueryResult &request);
};