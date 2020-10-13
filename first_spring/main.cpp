#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using std::string_literals::operator""s;

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED
};

const int MAX_RESULT_DOCUMENT_COUNT = 5;

std::string ReadLine() {
    std::string s;
    getline(std::cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    std::cin >> result;
    ReadLine();
    return result;
}

std::vector<std::string> SplitIntoWords(const std::string &text) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == ' ') {
            words.push_back(word);
            word = "";
        } else {
            word += c;
        }
    }
    words.push_back(word);

    return words;
}

struct Document {
    int id;
    double relevance;
    int rating;
    DocumentStatus status;
};

class SearchServer {
public:
    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string &raw_query, int document_id) const {
        DocumentStatus document_status = DocumentStatus::ACTUAL;

        std::vector<std::string> words_doc = GetAllWordsInDocument(document_id);

        auto query_words = ParseQuery(raw_query);

        std::set<std::string> words_doc_set(words_doc.begin(), words_doc.end());


        std::set<std::string> result;

        std::set_intersection(query_words.plus_words.begin(), query_words.plus_words.end(), words_doc_set.begin(), words_doc_set.end(),
                              std::inserter(result, result.begin()));

        std::vector<std::string> result_v(result.begin(), result.end());

        //        std::sort(result_v.begin(), result_v.end(), [](std::string &l, std::string &r) {
        //            return l < r;
        //        }); UPD: Этот кусок действительно не нужен т.к сет в нутри держит отсортированную структуру именно по возрастанию

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

    void SetStopWords(const std::string &text) {
        for (const std::string &word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const std::string &document, const DocumentStatus &status, const std::vector<int> &ratings) {

        const std::vector<std::string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const std::string &word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }

        document_statuses_ratings_.insert(std::pair<int, std::pair<DocumentStatus, int>>(
                document_id, std::pair<DocumentStatus, int>(status, ComputeAverageRating(ratings))));
    }

    std::vector<Document> FindTopDocuments(const std::string &raw_query, const std::function<bool(int, DocumentStatus, int)> &f) const {
        const Query query = ParseQuery(raw_query);
        auto non_matched_documents = FindAllDocuments(query);
        std::vector<Document> matched_documents;

        for (const auto &doc : non_matched_documents) {
            if (f(doc.id, doc.status, doc.rating)) {
                matched_documents.push_back(doc);
            }
        }

        sort(matched_documents.begin(), matched_documents.end(),
             [eps = eps_](const Document &lhs, const Document &rhs) {
                 if ((abs(lhs.relevance - rhs.relevance) < eps)) {
                     return true;
                 }
                 return lhs.relevance > rhs.relevance;
             });

        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    std::vector<Document> FindTopDocuments(const std::string &raw_query) const {

        return FindTopDocuments(raw_query, [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; });
    }

    std::vector<Document> FindTopDocuments(const std::string &raw_query, const DocumentStatus st) const {

        return FindTopDocuments(raw_query, [&st](int document_id, DocumentStatus status, int rating) { return status == st; });
    }

private:
    const double eps_ = 1e-6;
    std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, std::pair<DocumentStatus, int>> document_statuses_ratings_;        /// правильней объявить структуру, она более информативная

    bool IsStopWord(const std::string &word) const {
        return stop_words_.count(word) > 0;
    }
    std::vector<std::string> GetAllWordsInDocument(const int document_id) const {
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
    std::vector<std::string> SplitIntoWordsNoStop(const std::string &text) const {
        std::vector<std::string> words;
        for (const std::string &word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const std::vector<int> &ratings) {
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(const std::string &text) const {
        std::string tmp = text;
        bool is_minus = false;
        if (text[0] == '-') {
            is_minus = true;
            tmp = text.substr(1);
        }
        return {
                tmp,
                is_minus,
                IsStopWord(text)};
    }

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    Query ParseQuery(const std::string &text) const {
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


    double ComputeWordInverseDocumentFreq(const std::string &word) const {
        return log(document_statuses_ratings_.size() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    std::vector<Document> FindAllDocuments(const Query &query) const {
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
            matched_documents.push_back({document_id,
                                         relevance,
                                         item.second,
                                         item.first});
        }
        return matched_documents;
    }
};

void PrintDocument(const Document &document) {
    std::cout << "{ "s
              << "document_id = "s << document.id << ", "s
              << "relevance = "s << document.relevance << ", "s
              << "rating = "s << document.rating
              << " }"s << std::endl;
}

//int main() {
//    SearchServer search_server;
//    search_server.SetStopWords("и в на"s);
//
//    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3});
//    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
//    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
//    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, {9});
//
//    std::cout << "ACTUAL by default:"s << std::endl;
//    for (const Document &document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
//        PrintDocument(document);
//    }
//
//    std::cout << "BANNED:"s << std::endl;
//    for (const Document &document : search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)) {
//        PrintDocument(document);
//    }
//
//    std::cout << "Even ids:"s << std::endl;
//    for (const Document &document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
//        PrintDocument(document);
//    }
//
//    return 0;
//}

/* Подставьте вашу реализацию класса SearchServer сюда */

// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = {1, 2, 3};
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        assert(found_docs.size() == 1);
        const Document& doc0 = found_docs[0];
        assert(doc0.id == doc_id);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        assert(server.FindTopDocuments("in"s).empty());
    }
}

/*
Разместите код остальных тестов здесь
*/

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    TestExcludeStopWordsFromAddedDocumentContent();
    // Не забудьте вызывать остальные тесты здесь
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    std::cout << "Search server testing finished"s << std::endl;
}