#pragma once
#include <cmath>
#include <functional>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>

#include "document.h"
#include "string_processing.h"


class SearchServer {
public:
	template<typename StringContainer>
	explicit SearchServer(const StringContainer &stop_words);

	explicit SearchServer(const std::string &stop_words_text);

	[[nodiscard]] std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string &raw_query, int document_id) const;

	int GetDocumentId(int index) const;

	void AddDocument(int document_id, const std::string &document, const DocumentStatus &status, const std::vector<int> &ratings);

	std::vector<Document> FindTopDocuments(const std::string &raw_query, const std::function<bool(int, DocumentStatus, int)> &f) const;

	[[nodiscard]] std::vector<Document> FindTopDocuments(const std::string &raw_query) const;

	[[nodiscard]] std::vector<Document> FindTopDocuments(const std::string &raw_query, const DocumentStatus st) const;

	[[nodiscard]] size_t GetDocumentCount() const;

	std::vector<int>::const_iterator begin() const;

	std::vector<int>::const_iterator end() const;

	const std::map<std::string, double> &GetWordFrequencies(int document_id) const;

private:
	static bool IsValidWord(const std::string &word);

	const double eps_ = 1e-6;
	std::set<std::string> stop_words_;
	std::map<std::string, std::map<int, double>> word_to_document_freqs_;
	std::map<int, std::pair<DocumentStatus, int>> document_statuses_ratings_;
	std::vector<int> document_ids_;

	[[nodiscard]] bool IsStopWord(const std::string &word) const;
	[[nodiscard]] std::vector<std::string> GetAllWordsInDocument(const int document_id) const;
	[[nodiscard]] std::vector<std::string> SplitIntoWordsNoStop(const std::string &text) const;

	static int ComputeAverageRating(const std::vector<int> &ratings);

	struct QueryWord {
		std::string data;
		bool is_minus;
		bool is_stop;
	};

	[[nodiscard]] QueryWord ParseQueryWord(const std::string &text) const;

	struct Query {
		std::set<std::string> plus_words;
		std::set<std::string> minus_words;
	};

	[[nodiscard]] Query ParseQuery(const std::string &text) const;

	[[nodiscard]] double ComputeWordInverseDocumentFreq(const std::string &word) const;

	[[nodiscard]] std::vector<Document> FindAllDocuments(const Query &query) const;
};

template<typename StringContainer>
SearchServer::SearchServer(const StringContainer &stop_words)
	: stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
	if (!std::all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
		throw std::invalid_argument(std::string("Some of stop words are invalid"));
	}
}
