#include "dummy_framework.h"

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED
};

const int MAX_RESULT_DOCUMENT_COUNT = 5;

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
    Document() = default;

    Document(int id, double relevance, int rating)
        : id(id), relevance(relevance), rating(rating) {
    }

    Document(int id, double relevance, int rating, DocumentStatus status)
        : id(id), relevance(relevance), rating(rating), status(status) {
    }

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
    DocumentStatus status = DocumentStatus::ACTUAL;
};

class SearchServer {
public:
    SearchServer() = default;
    
    template<typename StringContainer>
    explicit SearchServer(const StringContainer &stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
        for (const auto &item : stop_words) {
            QueryChecker(item, __FUNCTION__);
        }
    }

    explicit SearchServer(const std::string &stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text)) {
        QueryChecker(stop_words_text, __FUNCTION__);
    }

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string &raw_query, int document_id) const {
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

    int GetDocumentId(int index) const {
        std::vector<std::pair<int, std::pair<DocumentStatus, int>>> res(document_statuses_ratings_.begin(), document_statuses_ratings_.end());
        if (index < 0 || index > res.size()) {
            throw std::out_of_range("Out of range in document_statuses_ratings_");
        }
        return res[index].first;
    }

    void AddDocument(int document_id, const std::string &document, const DocumentStatus &status, const std::vector<int> &ratings) {

        if (document_statuses_ratings_.find(document_id) != document_statuses_ratings_.end()) {
            throw std::invalid_argument(std::string("Invalid document_id ") + std::to_string(document_id));
        }
        if (document_id < 0) {
            throw std::invalid_argument("");
        }

        QueryChecker(document, __FUNCTION__);
        const std::vector<std::string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const std::string &word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }

        document_statuses_ratings_.insert(std::pair<int, std::pair<DocumentStatus, int>>(
                document_id, std::pair<DocumentStatus, int>(status, ComputeAverageRating(ratings))));
    }

    std::vector<Document> FindTopDocuments(const std::string &raw_query, const std::function<bool(int, DocumentStatus, int)> &f) const {

        QueryChecker(raw_query, __FUNCTION__);

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

    size_t GetDocumentCount() const;

private:
    static bool IsValidWord(const std::string &word) {
        // A valid word must not contain special characters
        return none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';
        });
    }

    static bool IsHaveVoidMinus(const std::string &query) {
        if (query == "-") {
            return true;
        }

        if (query[query.size() - 1] == '-') {
            return true;
        }

        for (auto i = 0; i < query.size(); ++i) {
            if (query[i] == '-' && i != query.size() - 1) {
                if (query[i + 1] == ' ') {
                    return true;
                }
            }
        }

        return false;
    }

    void SetStopWords(const std::string &text) {
        for (const std::string &word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    static void QueryChecker(const std::string &text, const std::string &func) {
        // Если нашли два минуса сразу нафиг такой запрос!
        if (text.find("--") != std::string::npos) {
            throw std::invalid_argument("Invalid std::string(\"" + text + "\") in " + std::string(func));
        } else if (!IsValidWord(text)) {
            throw std::invalid_argument("Invalid chars from [0x0 -> 0x20] in std::string(\"" + text + "\") in " + std::string(func));
        } else if (IsHaveVoidMinus(text)) {
            throw std::invalid_argument("Invalid void minus std::string(\"" + text + "\") in " + std::string(func));
        }
    }
    const double eps_ = 1e-6;
    std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, std::pair<DocumentStatus, int>> document_statuses_ratings_;

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
        QueryChecker(text, __FUNCTION__);
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

    template<typename StringContainer>
    static std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer &strings) {
        std::set<std::string> non_empty_strings;
        for (const std::string &str : strings) {
            if (!str.empty()) {
                non_empty_strings.insert(str);
            }
        }
        return non_empty_strings;
    }
};

size_t SearchServer::GetDocumentCount() const {
    return document_statuses_ratings_.size();
}

//Добавление документов.
// Добавленный документ должен находиться по поисковому запросу,// который содержит слова из документа.
void TestAddDocument() {
    {
        SearchServer server;
        server.AddDocument(0, "sample document", DocumentStatus::ACTUAL, {2, 4});
        const auto found_docs = server.FindTopDocuments("document"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document &doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, 0);
    }
    {
        SearchServer server;
        const auto found_docs = server.FindTopDocuments("document"s);
        ASSERT(found_docs.empty());
    }
}

//Поддержка стоп-слов.
// Стоп-слова исключаются из текста документов.
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
        ASSERT(found_docs.size() == 1);/// по возможности лучше использовать ASSERT_EQUAL, т.к. в случае срабатывания, он более информативный
        const Document &doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in"s).empty());
    }
}

//Поддержка минус-слов. Документы, содержащие минус-слова поискового запроса,
// не должны включаться в результаты поиска.
void TestMinusWords() {
    const int doc_id = 42;
    const std::string content = "-cat in the city"s;
    const std::string content2 = "cat in the city2"s;
    const std::string content3 = "-cat in -the city3"s;
    const std::vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id + 1, content2, DocumentStatus::ACTUAL, ratings);
        const auto list_docs = server.FindTopDocuments("cat", DocumentStatus::ACTUAL);
        ASSERT_EQUAL(list_docs.size(), 1);
        ASSERT_EQUAL(list_docs[0].id, 43);
    }
    {
        SearchServer server;
        server.AddDocument(doc_id, content2, DocumentStatus::ACTUAL, ratings);
        const auto actual = server.MatchDocument("cat", 42);
        ASSERT_EQUAL(std::get<0>(actual).size(), 1);
    }
    {
        SearchServer server;
        server.AddDocument(doc_id, content3, DocumentStatus::ACTUAL, ratings);
        const auto actual = server.MatchDocument("city3", 42);
        ASSERT_EQUAL(std::get<0>(actual).size(), 1);
        ASSERT_EQUAL(static_cast<int>(std::get<1>(actual)), static_cast<int>(DocumentStatus::ACTUAL));
    }
}

// Добавление документов. Добавленный документ должен находиться по поисковому запросу,
// который содержит слова из документа.
void TestSearch() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = {1, 2, 3};

    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("cat"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document &doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("dog going fast"s);
        ASSERT(found_docs.empty());
    }
}

// Тест проверяет, что поисковая система правильно считает кол-во документов
void TestCountDocuments() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::string content2 = "cat in the city2"s;
    const std::vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id + 1, content2, DocumentStatus::ACTUAL, ratings);
        const auto count = server.GetDocumentCount();
        ASSERT_EQUAL(count, 2);
    }
    {
        SearchServer server;
        const auto count = server.GetDocumentCount();
        ASSERT_EQUAL(count, 0);
    }
}

//Матчинг документов. При матчинге документа по поисковому запросу должны быть
// возвращены все слова из поискового запроса, // присутствующие в документе. Если есть соответствие хотя бы по // одному минус-слову, должен возвращаться пустой список слов.
void TestMatchDocument() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::string content2 = "long long long long -short"s;
    const std::vector<int> ratings = {1, 2, 3};

    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.MatchDocument("cat city", 42);
        ASSERT_EQUAL(std::get<0>(found_docs).size(), 2);
        ASSERT_EQUAL(static_cast<int>(std::get<1>(found_docs)), static_cast<int>(DocumentStatus::ACTUAL));
    }
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.MatchDocument("long short", 42);
        ASSERT_EQUAL(std::get<0>(found_docs).size(), 0);
        ASSERT_EQUAL(static_cast<int>(std::get<1>(found_docs)), static_cast<int>(DocumentStatus::ACTUAL));
    }
}

//Сортировка найденных документов по релевантности.
// Возвращаемые при поиске документов результаты должны быть отсортированы в
// порядке убывания релевантности.
void TestRelevanceSort() {
    std::vector<std::string> docs = {
            "cat dog flower",
            "cat cat flower",
            "cat dog dog"};

    {
        SearchServer server;
        server.AddDocument(0, docs[0], DocumentStatus::ACTUAL, {1, 2, 3});
        server.AddDocument(1, docs[1], DocumentStatus::ACTUAL, {4, 1, 3});
        server.AddDocument(2, docs[2], DocumentStatus::ACTUAL, {9, 1, 2});
        const auto found_docs = server.FindTopDocuments("dog");
        ASSERT(found_docs[0].relevance > found_docs[1].relevance);
    }
}

//Вычисление рейтинга документов.
// Рейтинг добавленного документа равен среднему
// арифметическому оценок документа.
void TestRating() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = {1, 2, 3};

    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("cat"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document &doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
        ASSERT_EQUAL(doc0.rating, 2);
    }
}

// Поиск документов, имеющих заданный статус.
void TestDocumentWithStatus() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::string content2 = "all dogs go to heaven"s;
    const std::vector<int> ratings = {1, 2, 3};
    const std::vector<int> ratings2 = {2, 6};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id + 1, content2, DocumentStatus::BANNED, ratings2);
        const auto found_docs = server.FindTopDocuments("dogs", DocumentStatus::BANNED);

        ASSERT_EQUAL(found_docs.size(), 1);
        const Document &doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, 43);
        ASSERT_EQUAL(doc0.rating, 4);
    }
}

//Фильтрация результатов поиска с использованием предиката, задаваемого пользователем.
void TestDocumentPredicate() {
    SearchServer search_server("и в на"s);

    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, {9});

    for (const Document &document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
        ASSERT_EQUAL(document.id % 2, 0);
    }
}

bool double_equals(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}
//Корректное вычисление релевантности найденных документов.
void TestRelevance() {
    SearchServer search_server("и в на"s);
    std::vector<std::string> docs = {"white cat"s, "black cat"s, "orange dog"s, "ping pig"s};
    search_server.AddDocument(0, docs[0], DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, docs[1], DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, docs[2], DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(3, docs[3], DocumentStatus::BANNED, {9});

    auto result = search_server.FindTopDocuments("cat");
    double actual_relevance = (1.0 / 2.0) * log(docs.size() / 2);
    ASSERT_EQUAL(search_server.GetDocumentCount(), docs.size());
    ASSERT(double_equals(result[0].relevance, actual_relevance));// Пользуюсь gtests , у них был и есть отдельный макрос для даблов ASSERT_DOUBLE_EQ ну не суть, поняли друг друга
}

void TestConstructors() {
    {
        std::vector<std::string> vec = {std::string("ab"), std::string("b")};
        SearchServer s(vec);
        s.AddDocument(1, "docmunts", DocumentStatus::ACTUAL, {1, 2, 3});
        ASSERT_EQUAL(s.GetDocumentCount(), 1);
    }
    {
        SearchServer s;
        ASSERT_EQUAL(s.GetDocumentCount(), 0);
    }
}

void TestExceptions_Minuses() {
    {
        std::vector<std::string> vec = {std::string("ab"), std::string("b")};
        SearchServer s(vec);
        s.AddDocument(1, "text text", DocumentStatus::ACTUAL, {1, 2, 3});
        auto search = "--search"s;
        try {
            s.FindTopDocuments(search);
        } catch (const std::exception &e) {
            ASSERT_EQUAL("Invalid std::string(\"" + search + "\") in FindTopDocuments", e.what());
        }
    }
}

void TestExceptions_ValidWord() {
    {
        std::vector<std::string> vec = {std::string("ab"), std::string("b")};
        SearchServer s(vec);
        s.AddDocument(1, "text text", DocumentStatus::ACTUAL, {1, 2, 3});
        std::string search = "скворец"s;
        search[1] = 0x1;
        try {
            s.FindTopDocuments(search);
        } catch (const std::exception &e) {
            ASSERT_EQUAL("Invalid chars from [0x0 -> 0x20] in std::string(\"" + search + "\") in FindTopDocuments", e.what());
        }
    }
}

void TestExceptions_VoidMinus() {
    {
        std::vector<std::string> vec = {std::string("ab"), std::string("b")};
        SearchServer s(vec);
        s.AddDocument(1, "text text", DocumentStatus::ACTUAL, {1, 2, 3});
        std::string search = "- скворец"s;
        try {
            s.FindTopDocuments(search);
        } catch (const std::exception &e) {
            ASSERT_EQUAL("Invalid void minus std::string(\"" + search + "\") in FindTopDocuments", e.what());
        }
    }
}
// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    //1
    RUN_TEST(TestAddDocument);
    //2
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    //3
    RUN_TEST(TestMinusWords);
    //4
    RUN_TEST(TestMatchDocument);
    //5
    RUN_TEST(TestRelevanceSort);
    //6
    RUN_TEST(TestRating);
    //7
    RUN_TEST(TestDocumentPredicate);
    //8
    RUN_TEST(TestDocumentWithStatus);
    //9
    RUN_TEST(TestRelevance);

    RUN_TEST(TestSearch);

    RUN_TEST(TestCountDocuments);

    RUN_TEST(TestConstructors);

    RUN_TEST(TestExceptions_Minuses);

    RUN_TEST(TestExceptions_ValidWord);

    RUN_TEST(TestExceptions_VoidMinus);
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    std::cout << "Search server testing finished"s << std::endl;
}
