#pragma once
#include "search_server.h"
#include <deque>

class RequestQueue {
public:
	explicit RequestQueue(const SearchServer &search_server);
	// сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
	template<typename DocumentPredicate>
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
		explicit QueryResult(const std::vector<Document> &r);
		std::vector<Document> result;
	};
	std::deque<QueryResult> requests_;

	const static int sec_in_day_ = 1440;
	// возможно, здесь вам понадобится что-то ещё
	size_t count_empty_query;
	const SearchServer &search_server_;

	// метод добавления ответа сервера в очередь
	void AddQueue(const QueryResult &request);
};