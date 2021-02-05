#pragma once

#include "document.h"
#include "string_processing.h"

#include <algorithm>
#include <execution>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer
{
  public:
	template <typename StringContainer>
	explicit SearchServer(const StringContainer& stop_words)
		: stop_words_(MakeUniqueNonEmptyStrings<StringContainer>(stop_words))
	{
		if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord))
		{
			throw std::invalid_argument("Some of stop words are invalid");
		}
	}

	explicit SearchServer(const std::string& stop_words_text);

	explicit SearchServer(std::string_view stop_words_text);

	void AddDocument(int document_id, const std::string_view document, DocumentStatus status,
					 const std::vector<int>& ratings);

	template <typename DocumentPredicate>
	std::vector<Document> FindTopDocuments(const std::string_view raw_query,
										   DocumentPredicate document_predicate) const;
	std::vector<Document> FindTopDocuments(const std::string_view raw_query, DocumentStatus status) const;
	std::vector<Document> FindTopDocuments(const std::string_view raw_query) const;



	template <typename ExecutionPolicy>
	std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(ExecutionPolicy&& policy,
																			std::string_view raw_query,
																			int document_id) const;

	std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::string_view raw_query,
																			int document_id) const;

	const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;
	int GetDocumentCount() const;
	void RemoveDocument(int document_id);

	template <typename ExecutionPolicy> void RemoveDocument(ExecutionPolicy&& policy, int document_id)
	{
		if (document_to_word_freqs_.count(document_id) == 1)
		{
			auto find_id = find(policy, document_ids_.begin(), document_ids_.end(), document_id);
			document_ids_.erase(find_id);
			documents_.erase(document_id);
			std::for_each(policy, word_to_document_freqs_.begin(), word_to_document_freqs_.end(),
						  [document_id](auto& doc) { doc.second.erase(document_id); });
			document_to_word_freqs_.erase(document_id);
		}
	}

	/// не знаю, на сколько это корректно понять тип результата по условию задачи, оставим это на вашем понимании задачи
	//
	std::set<std::string> GetAllWordsInDocument(const int document_id) const;

	std::set<int>::const_iterator begin() const;

	std::set<int>::const_iterator end() const;

  private:
	struct DocumentData
	{
		int rating;
		DocumentStatus status;
	};
	const std::set<std::string> stop_words_;
	std::map<std::string, std::map<int, double>> word_to_document_freqs_;
	std::map<int, std::map<std::string, double>> document_to_word_freqs_;
	std::map<int, DocumentData> documents_;
	std::set<int> document_ids_;

	bool IsStopWord(const std::string_view word) const;
	static bool IsValidWord(const std::string_view word);
	std::vector<std::string_view> SplitIntoWordsNoStop(std::string_view text) const;
	static int ComputeAverageRating(const std::vector<int>& ratings);

	struct QueryWord
	{
		std::string_view data;
		bool is_minus;
		bool is_stop;
	};

	QueryWord ParseQueryWord(std::string_view text) const;

	struct Query
	{
		std::set<std::string, std::less<>> plus_words;
		std::set<std::string, std::less<>> minus_words;
	};
	Query ParseQuery(std::string_view text) const;

	// Existence required
	double ComputeWordInverseDocumentFreq(const std::string& word) const;

	template <typename DocumentPredicate>
	std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const;
};

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query,
													 DocumentPredicate document_predicate) const
{
	const auto query = ParseQuery(std::string(raw_query));

	auto matched_documents = FindAllDocuments(query, document_predicate);

	std::sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
		if (std::abs(lhs.relevance - rhs.relevance) < 1e-6)
		{
			return lhs.rating > rhs.rating;
		}
		else
		{
			return lhs.relevance > rhs.relevance;
		}
	});
	if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
	{
		matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
	}

	return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const
{
	std::map<int, double> document_to_relevance;
	for (const std::string& word : query.plus_words)
	{
		if (word_to_document_freqs_.count(word) == 0)
		{
			continue;
		}
		const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
		for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word))
		{
			const auto& document_data = documents_.at(document_id);
			if (document_predicate(document_id, document_data.status, document_data.rating))
			{
				document_to_relevance[document_id] += term_freq * inverse_document_freq;
			}
		}
	}

	for (const std::string& word : query.minus_words)
	{
		if (word_to_document_freqs_.count(word) == 0)
		{
			continue;
		}
		for (const auto [document_id, _] : word_to_document_freqs_.at(word))
		{
			document_to_relevance.erase(document_id);
		}
	}

	std::vector<Document> matched_documents;
	for (const auto [document_id, relevance] : document_to_relevance)
	{
		matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
	}
	return matched_documents;
}

template <typename ExecutionPolicy>
std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(ExecutionPolicy&& policy,
																					  std::string_view raw_query,
																					  int document_id) const
{
	constexpr bool is_par = std::is_same_v<ExecutionPolicy, std::execution::parallel_policy>;
	if (is_par)
	{
		const auto query = ParseQuery(raw_query);

		const auto status = documents_.at(document_id).status;

		const auto word_checker = [this, document_id](std::string_view word) {
			const auto it = word_to_document_freqs_.find(std::string(word));
			return it != word_to_document_freqs_.end() && it->second.count(document_id);
		};

		if (any_of(std::execution::par, query.minus_words.begin(), query.minus_words.end(), word_checker))
		{
			return {{}, status};
		}

		std::vector<std::string_view> matched_words(query.plus_words.size());
		auto words_end = copy_if(std::execution::par, query.plus_words.begin(), query.plus_words.end(),
								 matched_words.begin(), word_checker);
		sort(matched_words.begin(), words_end);
		words_end = unique(matched_words.begin(), words_end);
		matched_words.erase(words_end, matched_words.end());

		return {matched_words, status};
	}
	else
	{
		const auto query = ParseQuery(raw_query);

		const auto status = documents_.at(document_id).status;

		for (const std::string_view word : query.minus_words)
		{
			if (word_to_document_freqs_.count(std::string(word)) == 0)
			{
				continue;
			}
			if (word_to_document_freqs_.at(std::string(word)).count(document_id))
			{
				return {{}, status};
			}
		}

		std::vector<std::string_view> matched_words;
		for (const std::string_view word : query.plus_words)
		{
			if (word_to_document_freqs_.count(std::string(word)) == 0)
			{
				continue;
			}
			if (word_to_document_freqs_.at(std::string(word)).count(document_id))
			{
				matched_words.push_back(word);
			}
		}
		return {matched_words, status};
	}
}
