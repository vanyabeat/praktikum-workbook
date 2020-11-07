#include "request_queue.h"

RequestQueue::RequestQueue(const SearchServer &search_server)
		: count_empty_query(0),
		  search_server_(search_server) {
	// напишите реализацию
}
std::vector<Document> RequestQueue::AddFindRequest(const std::string &raw_query, DocumentStatus status) {
	auto request = search_server_.FindTopDocuments(raw_query, status);
	QueryResult result;
	result.result = request;
	AddQueue(result);
	return request;
}
std::vector<Document> RequestQueue::AddFindRequest(const std::string &raw_query) {
	auto request = search_server_.FindTopDocuments(raw_query);
	QueryResult result;
	result.result = request;
	AddQueue(result);
	return request;
}
size_t RequestQueue::GetNoResultRequests() const {
	return count_empty_query;
}
void RequestQueue::AddQueue(const RequestQueue::QueryResult &request) {
	if (requests_.size() == sec_in_day_) {
		if (requests_.front().result.empty()) {
			--count_empty_query;
		}
		requests_.pop_front();
	}
	requests_.push_back(request);
	if (request.result.empty()) {
		++count_empty_query;
	}
}
