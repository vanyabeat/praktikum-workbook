#pragma once
#include <deque>

#include "search_server.h"


class RequestQueue {
public:
	explicit RequestQueue(const SearchServer &search_server);

	template<typename DocumentPredicate>
	std::vector<Document> AddFindRequest(const std::string &raw_query, DocumentPredicate document_predicate);

	std::vector<Document> AddFindRequest(const std::string &raw_query, DocumentStatus status);

	std::vector<Document> AddFindRequest(const std::string &raw_query);

	size_t GetNoResultRequests() const;

private:
	struct QueryResult {
		std::vector<Document> result;
	};
	std::deque<QueryResult> requests_;
	const static int sec_in_day_ = 1440;
	size_t count_empty_query_;
	const SearchServer &search_server_;
	void AddQueue(const QueryResult &request);
};

template<typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string &raw_query, DocumentPredicate document_predicate) {
	auto request = search_server_.FindTopDocuments(raw_query, document_predicate);
	QueryResult result(request);
	AddQueue(result);
	return request;
}