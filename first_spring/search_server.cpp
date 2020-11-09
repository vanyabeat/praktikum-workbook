#include "search_server.h"

size_t SearchServer::GetDocumentCount() const {
	return document_statuses_ratings_.size();
}
SearchServer::SearchServer(const std::string &stop_words_text)
	: SearchServer(SplitIntoWords(stop_words_text)) {
}
std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string &raw_query, int document_id) const {
	DocumentStatus document_status = DocumentStatus::ACTUAL;

	std::vector<std::string> words_doc = GetAllWordsInDocument(document_id);

	auto query_words = ParseQuery(raw_query);

	std::set<std::string> words_doc_set(words_doc.begin(), words_doc.end());


	std::set<std::string> result;

	std::set_intersection(query_words.plus_words.begin(), query_words.plus_words.end(), words_doc_set.begin(), words_doc_set.end(),
						  std::inserter(result, result.begin()));

	std::vector<std::string> result_v(result.begin(), result.end());

	auto needle_document = document_statuses_ratings_.find(document_id);

	if (needle_document != document_statuses_ratings_.end()) {
		document_status = needle_document->second.first;
	}

	for (const auto &m_w : query_words.minus_words) {
		for (const auto &w : words_doc_set) {
			if (m_w == w) {
				return std::tuple<std::vector<std::string>, DocumentStatus>(std::vector<std::string>(), document_status);
			}
		}
	}
	return std::tuple<std::vector<std::string>, DocumentStatus>(result_v, document_status);
}
int SearchServer::GetDocumentId(int index) const {
	std::vector<std::pair<int, std::pair<DocumentStatus, int>>> res(document_statuses_ratings_.begin(), document_statuses_ratings_.end());
	if (index < 0 || index > res.size()) {
		throw std::out_of_range("Out of range in document_statuses_ratings_");
	}
	return res[index].first;
}
void SearchServer::AddDocument(int document_id, const std::string &document, const DocumentStatus &status, const std::vector<int> &ratings) {

	if (document_statuses_ratings_.find(document_id) != document_statuses_ratings_.end()) {
		throw std::invalid_argument(std::string("document_id ") + std::to_string(document_id) + " exist");
	}

	if (document_id < 0) {
		throw std::invalid_argument("Negative document_id");
	}

	const std::vector<std::string> words = SplitIntoWordsNoStop(document);
	const double inv_word_count = 1.0 / words.size();
	for (const std::string &word : words) {
		word_to_document_freqs_[word][document_id] += inv_word_count;
	}

	document_statuses_ratings_.insert(std::pair<int, std::pair<DocumentStatus, int>>(
			document_id, std::pair<DocumentStatus, int>(status, ComputeAverageRating(ratings))));
}
std::vector<Document> SearchServer::FindTopDocuments(const std::string &raw_query, const std::function<bool(int, DocumentStatus, int)> &f) const {

	const Query query = ParseQuery(raw_query);
	auto non_matched_documents = FindAllDocuments(query);
	std::vector<Document> matched_documents;

	for (const auto &doc : non_matched_documents) {
		if (f(doc.id, document_statuses_ratings_.at(doc.id).first, doc.rating)) {
			matched_documents.push_back(doc);
		}
	}

	sort(matched_documents.begin(), matched_documents.end(),
		 [eps = eps_](const Document &lhs, const Document &rhs) {
			 if ((abs(lhs.relevance - rhs.relevance) < eps)) {
				 return lhs.rating > rhs.rating;
			 }
			 return lhs.relevance > rhs.relevance;
		 });

	if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
		matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
	}
	return matched_documents;
}
std::vector<Document> SearchServer::FindTopDocuments(const std::string &raw_query) const {

	return FindTopDocuments(raw_query, [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; });
}
std::vector<Document> SearchServer::FindTopDocuments(const std::string &raw_query, DocumentStatus st) const {

	return FindTopDocuments(raw_query, [&st](int document_id, DocumentStatus status, int rating) { return status == st; });
}
bool SearchServer::IsValidWord(const std::string &word) {
	// A valid word must not contain special characters
	return none_of(word.begin(), word.end(), [](char c) {
		return c >= '\0' && c < ' ';
	});
}
bool SearchServer::IsStopWord(const std::string &word) const {
	return stop_words_.count(word) > 0;
}
std::vector<std::string> SearchServer::GetAllWordsInDocument(const int document_id) const {
	std::vector<std::string> result;
	for (const std::pair<std::string, std::map<int, double>> &item : word_to_document_freqs_) {
		auto str = item.first;
		for (const std::pair<int, double> &item_ : item.second) {
			if (item_.first == document_id) {
				result.push_back(str);
				continue;
			}
		}
	}
	return result;
}
std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string &text) const {
	std::vector<std::string> words;
	for (const std::string &word : SplitIntoWords(text)) {
		if (!IsStopWord(word)) {
			words.push_back(word);
		}
	}
	if (!std::all_of(words.begin(), words.end(), IsValidWord)) {
		throw std::invalid_argument("Some of words are invalid"s);
	}
	return words;
}
int SearchServer::ComputeAverageRating(const std::vector<int> &ratings) {
	int rating_sum = 0;
	for (const int rating : ratings) {
		rating_sum += rating;
	}
	return rating_sum / static_cast<int>(ratings.size());
}
SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string &text) const {
	bool is_minus = false;
	std::string tmp = text;
	// [1] проверяем на пустое слово
	if (text.empty()) {
		throw std::invalid_argument("Empty query word");
	}
	// [2] если это не валидное слово выбрасываем исключение
	if (!IsValidWord(text)) {
		throw std::invalid_argument(std::string("Invalid chars from [0x0 -> 0x20] in query word ") + "\"" + text + "\"");
	}
	// [3] проверяем что первый символ == '-'
	if (text[0] == '-') {
		// [3.1]
		is_minus = true;
		tmp = text.substr(1);
		// [3.2]
		if (tmp.empty()) {
			throw std::invalid_argument(std::string("Void minus in query"));
		}
		// [3.3]
		if (tmp[0] == '-') {
			throw std::invalid_argument("Invalid query word \"" + text + "\"");
		}
	}
	// [4]
	return {
			tmp,
			is_minus,
			IsStopWord(text)};
}
SearchServer::Query SearchServer::ParseQuery(const std::string &text) const {
	Query query;
	for (const std::string &word : SplitIntoWords(text)) {
		const QueryWord query_word = ParseQueryWord(word);
		if (!query_word.is_stop) {
			if (query_word.is_minus) {
				query.minus_words.insert(query_word.data);
			} else {
				query.plus_words.insert(query_word.data);
			}
		}
	}
	return query;
}
std::vector<Document> SearchServer::FindAllDocuments(const SearchServer::Query &query) const {
	std::map<int, double> document_to_relevance;
	for (const std::string &word : query.plus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
		for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
			document_to_relevance[document_id] += term_freq * inverse_document_freq;
		}
	}

	for (const std::string &word : query.minus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
			document_to_relevance.erase(document_id);
		}
	}

	std::vector<Document> matched_documents;
	for (const auto [document_id, relevance] : document_to_relevance) {
		auto item = document_statuses_ratings_.at(document_id);
		matched_documents.emplace_back(document_id, relevance, item.second);
	}
	return matched_documents;
}
double SearchServer::ComputeWordInverseDocumentFreq(const std::string &word) const {
	return log(document_statuses_ratings_.size() * 1.0 / word_to_document_freqs_.at(word).size());
}
SearchServer::SearchServer(const char *stop_words_text) {
	SearchServer(std::string(stop_words_text));
}
