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

//Следующие методы теперь должны позволять принять string_view вместо строки:
//конструктор;
// AddDocument;
// FindTopDocuments;
// MatchDocument.
//Эти методы должны возвращать string_view вместо строк:
// MatchDocument;
// GetWordFrequencies.
class SearchServer
{
  public:
	template <typename StringContainer> explicit SearchServer(const StringContainer& stop_words);
	explicit SearchServer(const std::string& stop_words_text);
	explicit SearchServer(std::string_view stop_words_text);

	void AddDocument(int document_id, const std::string_view document, DocumentStatus status,
					 const std::vector<int>& ratings);

	template <typename DocumentPredicate>
	std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const;
	std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status) const;
	std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

	std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query,
																	   int document_id) const;
	template <typename ExecutionPolicy>
	std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(ExecutionPolicy e_p,
																	   const std::string& raw_query,
																	   int document_id) const;
	const std::map<std::string, double>& GetWordFrequencies(int document_id) const;
	int GetDocumentCount() const;
	void RemoveDocument(int document_id);

	template <typename ExecutionPolicy> void RemoveDocument(ExecutionPolicy e_p, int document_id);

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

	bool IsStopWord(const std::string& word) const;
	bool IsStopWord(const std::string_view word) const;
//static bool IsValidWord(const std::string& word);
	static bool IsValidWord(const std::string_view word);
	std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;
	std::vector<std::string_view> SplitIntoWordsNoStop(std::string_view text) const;
	static int ComputeAverageRating(const std::vector<int>& ratings);

	struct QueryWord
	{
		std::string data;
		bool is_minus;
		bool is_stop;
	};

	QueryWord ParseQueryWord(const std::string& text) const;

	struct Query
	{
		std::set<std::string> plus_words;
		std::set<std::string> minus_words;
	};

	Query ParseQuery(const std::string& text) const;

	// Existence required
	double ComputeWordInverseDocumentFreq(const std::string& word) const;

	template <typename DocumentPredicate>
	std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const;
};

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words)
	: stop_words_(MakeUniqueNonEmptyStrings(stop_words)) // Extract non-empty stop words
{
	if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord))
	{
		// здесь для использования литерала s нам нужен namespace std
		// мы можем использовать его здесь, так как он будет ограничен контекстом этого блока
		using namespace std;
		throw std::invalid_argument("Some of stop words are invalid"s);
	}
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query,
													 DocumentPredicate document_predicate) const
{
	const auto query = ParseQuery(raw_query);

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

template <typename ExecutionPolicy> void SearchServer::RemoveDocument(ExecutionPolicy e_p, int document_id)
{
	if (typeid(e_p) == typeid(std::execution::seq))
	{
		RemoveDocument(document_id);
		return;
	} // НЕ ПОНИМАЮ ЧТО Я ТУТ УСКОРИЛ :D, главное правило! работает однопоточно за приемлимое время не трожь!!!!
	else if (typeid(e_p) == typeid(std::execution::par))
	{
		auto needle_doc_it_set =
			std::find(std::execution::par, document_ids_.begin(), document_ids_.end(), document_id);
		if (needle_doc_it_set != document_ids_.end())
		{
			auto needle_doc_it_map = document_to_word_freqs_.find(document_id);
			auto needle_it_map = documents_.find(document_id);
			document_ids_.erase(needle_doc_it_set);
			document_to_word_freqs_.erase(needle_doc_it_map);
			documents_.erase(needle_it_map);
		}
		return;
	}
	else
	{
		throw std::runtime_error("Unknown ExecutionPolicy");
	}
}
template <typename ExecutionPolicy>
std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(ExecutionPolicy e_p,
																				 const std::string& raw_query,
																				 int document_id) const
{
	if (typeid(e_p) == typeid(std::execution::seq))
	{
		return MatchDocument(raw_query, document_id);
	}
	else if (typeid(e_p) == typeid(std::execution::par))
	{
		const auto query = ParseQuery(raw_query);

		std::vector<std::string> matched_words;
		std::for_each(std::execution::par, query.plus_words.begin(), query.plus_words.end(), [&](const std::string w) {
			if (word_to_document_freqs_.count(w) == 0)
			{
				// continue;
				return;
			}
			if (word_to_document_freqs_.at(w).count(document_id))
			{
				matched_words.push_back(w);
			}
		});
		std::for_each(std::execution::par, query.minus_words.begin(), query.minus_words.end(),
					  [&](const std::string w) {
						  if (word_to_document_freqs_.count(w) == 0)
						  {
							  return;
						  }
						  if (word_to_document_freqs_.at(w).count(document_id))
						  {
							  matched_words.clear();
						  }
					  });
		return {matched_words, documents_.at(document_id).status};
	}
	else
	{
		throw std::runtime_error("Unknown ExecutionPolicy");
	}
}
