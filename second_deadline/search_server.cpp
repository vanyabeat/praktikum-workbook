// source
#include "search_server.h"

#include <cmath>

using namespace std;

SearchServer::SearchServer(const string& stop_words_text)
	: SearchServer(SplitIntoWords(stop_words_text)) // Invoke delegating constructor from string container
{
}

void SearchServer::AddDocument(int document_id, std::string_view document, DocumentStatus status,
							   const vector<int>& ratings)
{
	if ((document_id < 0) || (documents_.count(document_id) > 0))
	{
		throw invalid_argument("Invalid document_id"s);
	}
	const auto words = SplitIntoWordsNoStop(document);

	const double inv_word_count = 1.0 / words.size();
	for (const auto& word : words)
	{
		word_to_document_freqs_[std::string(word)][document_id] += inv_word_count;
		document_to_word_freqs_[document_id][std::string(word)] += inv_word_count;
	}
	documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
	document_ids_.insert(document_id);
}

vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query, DocumentStatus status) const
{
	return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
		return document_status == status;
	});
}

vector<Document> SearchServer::FindTopDocuments(const string_view raw_query) const
{
	return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const
{
	return documents_.size();
}

void SearchServer::RemoveDocument(int document_id)
{
	RemoveDocument(std::execution::seq, document_id);
}

const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const
{
	static std::map<std::string_view, double> empty_map;
	if (document_to_word_freqs_.count(document_id) == 0)
	{
		return empty_map;
	}
	static std::map<std::string_view, double> map_;
	for (const auto& doc : document_to_word_freqs_.at(document_id))
	{
		map_[doc.first] = doc.second;
	}
	return map_;
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::string_view raw_query,
																					  int document_id) const
{
	return MatchDocument(std::execution::seq, raw_query, document_id);
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings)
{
	if (ratings.empty())
	{
		return 0;
	}
	int rating_sum = 0;
	for (const int rating : ratings)
	{
		rating_sum += rating;
	}
	return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view text) const
{
	if (text.empty())
	{
		throw std::invalid_argument("Query word is empty");
	}
	std::string_view word = text;
	bool is_minus = false;
	if (word[0] == '-')
	{
		is_minus = true;
		word = word.substr(1);
	}
	if (word.empty() || word[0] == '-' || !IsValidWord(word))
	{
		throw std::invalid_argument("Query word " + std::string(text) + " is invalid");
	}

	return {word, is_minus, IsStopWord(word)};
}

SearchServer::Query SearchServer::ParseQuery(std::string_view text) const
{
	Query result;
	const auto word_vector = SplitIntoWords(text);
	for (std::string_view word : word_vector)
	{
		auto query_word = ParseQueryWord(word);
		if (!query_word.is_stop)
		{
			if (query_word.is_minus)
			{
				result.minus_words.emplace(query_word.data);
			}
			else
			{
				result.plus_words.emplace(query_word.data);
			}
		}
	}
	return result;
}

// Existence required
double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const
{
	return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

std::set<std::string> SearchServer::GetAllWordsInDocument(const int document_id) const
{
	std::set<std::string> result;
	for (const std::pair<std::string, std::map<int, double>>& item : word_to_document_freqs_)
	{
		if (item.second.find(document_id) != item.second.end())
		{
			result.insert(item.first);
			continue;
		}
	}
	return result;
}

std::set<int>::const_iterator SearchServer::begin() const
{
	return document_ids_.begin();
}

std::set<int>::const_iterator SearchServer::end() const
{
	return document_ids_.end();
}

SearchServer::SearchServer(std::string_view stop_words_text)
	: SearchServer(SplitIntoWords(std::string(stop_words_text)))
{
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(std::string_view text) const
{
	std::vector<std::string_view> words;
	for (std::string_view word : SplitIntoWords(text))
	{
		if (!IsValidWord(word))
		{
			throw std::invalid_argument("Word " + std::string(word) + " is invalid");
		}
		if (!IsStopWord(word))
		{
			words.push_back(word);
		}
	}
	return words;
}

bool SearchServer::IsStopWord(std::string_view word) const
{
	return stop_words_.count(std::string(word)) > 0;
}

bool SearchServer::IsValidWord(std::string_view word)
{
	return std::none_of(word.begin(), word.end(), [](char c) { return c >= '\0' && c < ' '; });
}

// bool SearchServer::IsValidWord(const string& word) const
//{
//	// A valid word must not contain special characters
//	return none_of(word.begin(), word.end(), [](char c) { return c >= '\0' && c < ' '; });
//}