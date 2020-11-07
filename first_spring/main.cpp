#include "dummy_framework.h"

#include "paginator.h"
#include "request_queue.h"
#include "search_server.h"

//Добавление документов.
// Добавленный документ должен находиться по поисковому запросу,// который содержит слова из документа.
void TestAddDocument() {
	{
		SearchServer server("stops"s);
		server.AddDocument(0, "sample document", DocumentStatus::ACTUAL, {2, 4});
		const auto found_docs = server.FindTopDocuments("document"s);
		ASSERT_EQUAL(found_docs.size(), 1);
		const Document &doc0 = found_docs[0];
		ASSERT_EQUAL(doc0.id, 0);
	}
	{
		SearchServer server("stops"s);
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
		SearchServer server("stops"s);
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
		SearchServer server("stops"s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		server.AddDocument(doc_id + 1, content2, DocumentStatus::ACTUAL, ratings);
		const auto list_docs = server.FindTopDocuments("cat", DocumentStatus::ACTUAL);
		ASSERT_EQUAL(list_docs.size(), 1);
		ASSERT_EQUAL(list_docs[0].id, 43);
	}
	{
		SearchServer server("stops"s);
		server.AddDocument(doc_id, content2, DocumentStatus::ACTUAL, ratings);
		const auto actual = server.MatchDocument("cat", 42);
		ASSERT_EQUAL(std::get<0>(actual).size(), 1);
	}
	{
		SearchServer server("stops"s);
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
		SearchServer server("stops"s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		const auto found_docs = server.FindTopDocuments("cat"s);
		ASSERT_EQUAL(found_docs.size(), 1);
		const Document &doc0 = found_docs[0];
		ASSERT_EQUAL(doc0.id, doc_id);
	}
	{
		SearchServer server("stops"s);
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
		SearchServer server("stops"s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		server.AddDocument(doc_id + 1, content2, DocumentStatus::ACTUAL, ratings);
		const auto count = server.GetDocumentCount();
		ASSERT_EQUAL(count, 2);
	}
	{
		SearchServer server("stops"s);
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
		SearchServer server("stops"s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		const auto found_docs = server.MatchDocument("cat city", 42);
		ASSERT_EQUAL(std::get<0>(found_docs).size(), 2);
		ASSERT_EQUAL(static_cast<int>(std::get<1>(found_docs)), static_cast<int>(DocumentStatus::ACTUAL));
	}
	{
		SearchServer server("stops"s);
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
		SearchServer server("stops"s);
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
		SearchServer server("stops"s);
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
		SearchServer server("stops"s);
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
		SearchServer server("stips"s);
		ASSERT_EQUAL(server.GetDocumentCount(), 0);
	}
}

void TestExceptions_Minuses() {
	{
		std::vector<std::string> vec = {std::string("ab"), std::string("b")};
		SearchServer s(vec);
		s.AddDocument(1, "text text", DocumentStatus::ACTUAL, {1, 2, 3});
		auto search = "--search"s;
		try {
			auto res = s.FindTopDocuments(search);
		} catch (const std::exception &e) {
			ASSERT_EQUAL("Invalid query word \"" + search + "\"", e.what());
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
			auto res = s.FindTopDocuments(search);
		} catch (const std::exception &e) {
			ASSERT_EQUAL("Invalid chars from [0x0 -> 0x20] in query word \"" + search + "\"", e.what());
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
			auto res = s.FindTopDocuments(search);
		} catch (const std::exception &e) {
			ASSERT_EQUAL(std::string("Void minus in query"), e.what());
		}
	}
}

void TestExceptions_Ids() {
	{
		std::vector<std::string> vec = {std::string("ab"), std::string("b")};
		SearchServer s(vec);
		s.AddDocument(1, "text text", DocumentStatus::ACTUAL, {1, 2, 3});
		try {
			s.AddDocument(1, "text text2", DocumentStatus::ACTUAL, {1, 2, 3});
		} catch (const std::exception &e) {
			ASSERT_EQUAL(std::string("document_id 1 exist"), e.what());
		}
	}
	{
		std::vector<std::string> vec = {std::string("ab"), std::string("b")};
		SearchServer s(vec);
		s.AddDocument(1, "text text", DocumentStatus::ACTUAL, {1, 2, 3});
		try {
			s.AddDocument(-1, "text text2", DocumentStatus::ACTUAL, {1, 2, 3});
		} catch (const std::exception &e) {
			ASSERT_EQUAL(std::string("Negative document_id"), e.what());
		}
	}
}

void TestExceptions_DocumentIndexes() {
	{
		std::vector<std::string> vec = {std::string("ab"), std::string("b")};
		SearchServer s(vec);
		s.AddDocument(1, "text text", DocumentStatus::ACTUAL, {1, 2, 3});
		s.AddDocument(1000, "text text2", DocumentStatus::ACTUAL, {1, 2, 3});
		s.AddDocument(1001, "text text3", DocumentStatus::ACTUAL, {1, 2, 3});
		ASSERT_EQUAL(s.GetDocumentId(0), 1);
		ASSERT_EQUAL(s.GetDocumentId(2), 1001);
	}
	{
		std::vector<std::string> vec = {std::string("ab"), std::string("b")};
		SearchServer s(vec);
		s.AddDocument(1, "text text", DocumentStatus::ACTUAL, {1, 2, 3});
		try {
			s.GetDocumentId(10000);
		} catch (const std::exception &e) {
			ASSERT_EQUAL(std::string("Out of range in document_statuses_ratings_"), e.what());
		}
	}
}

void TestExceptions_Empty() {
	{
		std::vector<std::string> vec = {std::string("ab"), std::string("b")};
		SearchServer s(vec);
		s.AddDocument(1, "text text", DocumentStatus::ACTUAL, {1, 2, 3});
		try {
			auto res = s.FindTopDocuments("");
		} catch (const std::exception &e) {
			ASSERT_EQUAL(std::string("Empty query word"), e.what());
		}
	}
}

void TestExceptions_Undefined_Stop_Word() {
	{
		std::string stop_word1 = "stop";
		std::string stop_word2 = "stop";
		stop_word2[2] = 0x2;

		std::vector<std::string> vec = {stop_word1, stop_word2};

		try {
			SearchServer s(vec);
		} catch (const std::exception &e) {
			ASSERT_EQUAL("Some of stop words are invalid"s, e.what());
		}
	}
}

void Test_Queue() {
	SearchServer search_server("и в на"s);
	RequestQueue request_queue(search_server);

	search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
	search_server.AddDocument(2, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2, 3});
	search_server.AddDocument(3, "большой кот модный ошейник "s, DocumentStatus::ACTUAL, {1, 2, 8});
	search_server.AddDocument(4, "большой пёс скворец евгений"s, DocumentStatus::ACTUAL, {1, 3, 2});
	search_server.AddDocument(5, "большой пёс скворец василий"s, DocumentStatus::ACTUAL, {1, 1, 1});

	// 1439 запросов с нулевым результатом
	for (int i = 0; i < 1439; ++i) {
		request_queue.AddFindRequest("пустой запрос"s);
	}
	// все еще 1439 запросов с нулевым результатом
	request_queue.AddFindRequest("пушистый пёс"s);
	// новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
	request_queue.AddFindRequest("большой ошейник"s);
	// первый запрос удален, 1437 запросов с нулевым результатом
	request_queue.AddFindRequest("скворец"s);
	ASSERT_EQUAL(1439 - 2, request_queue.GetNoResultRequests() );

}

void Test_Pagination(){
	SearchServer search_server("and with"s);

	search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, {7, 2, 7});
	search_server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2, 3});
	search_server.AddDocument(3, "big cat nasty hair"s, DocumentStatus::ACTUAL, {1, 2, 8});
	search_server.AddDocument(4, "big dog cat Vladislav"s, DocumentStatus::ACTUAL, {1, 3, 2});
	search_server.AddDocument(5, "big dog hamster Borya"s, DocumentStatus::ACTUAL, {1, 1, 1});

	const auto search_results = search_server.FindTopDocuments("curly dog"s);
	int page_size = 2;
	const auto pages = Paginate(search_results, page_size);

	ASSERT_EQUAL(2, std::distance(pages.begin(), pages.end()));
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
	//10
	RUN_TEST(TestSearch);
	//11
	RUN_TEST(TestCountDocuments);
	//12
	RUN_TEST(TestConstructors);
	//13
	RUN_TEST(TestExceptions_Minuses);
	//14
	RUN_TEST(TestExceptions_ValidWord);
	//15
	RUN_TEST(TestExceptions_VoidMinus);
	//16
	RUN_TEST(TestExceptions_Ids);
	//17
	RUN_TEST(TestExceptions_DocumentIndexes);
	//18
	RUN_TEST(TestExceptions_Empty);
	//19
	RUN_TEST(TestExceptions_Undefined_Stop_Word);
	//20
	RUN_TEST(Test_Queue);
	//21
	RUN_TEST(Test_Pagination);
}


int main() {
	TestSearchServer();
	// Если вы видите эту строку, значит все тесты прошли успешно
	std::cout << "Search server testing finished"s << std::endl;
}