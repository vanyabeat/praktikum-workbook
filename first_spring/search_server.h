#pragma once
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <functional>
#include <cmath>

#include "document.h"
#include "string_processing.h"

using std::string_literals::operator""s;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer {
public:
	template<typename StringContainer>
	explicit SearchServer(const StringContainer &stop_words)
		: stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
		if (!std::all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
			throw std::invalid_argument("Some of stop words are invalid"s);
		}
	}

	explicit SearchServer(const std::string &stop_words_text);

	explicit SearchServer(const char * c_string);

	[[nodiscard]] std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string &raw_query, int document_id) const;

	int GetDocumentId(int index) const;

	void AddDocument(int document_id, const std::string &document, const DocumentStatus &status, const std::vector<int> &ratings);

	std::vector<Document> FindTopDocuments(const std::string &raw_query, const std::function<bool(int, DocumentStatus, int)> &f) const;

	[[nodiscard]] std::vector<Document> FindTopDocuments(const std::string &raw_query) const;

	[[nodiscard]] std::vector<Document> FindTopDocuments(const std::string &raw_query, const DocumentStatus st) const;

	[[nodiscard]] size_t GetDocumentCount() const;

private:
	static bool IsValidWord(const std::string &word);

	const double eps_ = 1e-6;
	std::set<std::string> stop_words_;
	std::map<std::string, std::map<int, double>> word_to_document_freqs_;
	std::map<int, std::pair<DocumentStatus, int>> document_statuses_ratings_;

	[[nodiscard]] bool IsStopWord(const std::string &word) const;
	[[nodiscard]] std::vector<std::string> GetAllWordsInDocument(const int document_id) const;
	[[nodiscard]] std::vector<std::string> SplitIntoWordsNoStop(const std::string &text) const;

	static int ComputeAverageRating(const std::vector<int> &ratings);

	struct QueryWord {
		std::string data;
		bool is_minus;
		bool is_stop;
	};

	/// все же отсутствует логика проверки, только отдельные признаки по заданию, из-за этого повторяющиеся проверки на символ "-".
	/// если метод полагается на то, что split не создает пустое слово, то это должно быть соответсвующе указано, но правильней, метод должен самостоятельно обеспечивать свою корректность
	/// и кстати, ваш split это не гарантирует, можете протестировать с "cat   in    the   city"
	/// предлагаю такой порядок проверки:
	/// 1 проверить на пустое слово (не text == "", и не text.size() == 0, а использовать только empty()), если да, то бросаем исключение
	/// 2 !IsValidWord
	/// 3 проверяете, что первый символ == '-', если да, то:
	///   3.1 вся логика с проверкий этого символа, которая была
	///   3.2 после удаления первого символа опять проверяем, что tmp непустое, если пустое, то бросаем исключение
	///   3.3 проверяем tmp[0] == '-', если да, то бросаем исключение
	/// 4 возврат результата

	[[nodiscard]] QueryWord ParseQueryWord(const std::string &text) const;

	struct Query {
		std::set<std::string> plus_words;
		std::set<std::string> minus_words;
	};

	[[nodiscard]] Query ParseQuery(const std::string &text) const;


	[[nodiscard]] double ComputeWordInverseDocumentFreq(const std::string &word) const;

	[[nodiscard]] std::vector<Document> FindAllDocuments(const Query &query) const;

};
