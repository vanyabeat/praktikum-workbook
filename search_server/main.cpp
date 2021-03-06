#include "dummy_framework.h"

#include "log_duration.h"
#include "paginator.h"
#include "process_queries.h"
#include "remove_duplicates.h"
#include "request_queue.h"
#include "search_server.h"

using namespace std;
//Добавление документов.
// Добавленный документ должен находиться по поисковому запросу,// который содержит слова из документа.
void TestAddDocument()
{
	using std::string_literals::operator""s;
	{
		SearchServer server("stops"s);
		server.AddDocument(0, "sample document", DocumentStatus::ACTUAL, {2, 4});
		const auto found_docs = server.FindTopDocuments("document"s);
		ASSERT_EQUAL(found_docs.size(), 1);
		const Document& doc0 = found_docs[0];
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
void TestExcludeStopWordsFromAddedDocumentContent()
{
	using std::string_literals::operator""s;
	const int doc_id = 42;
	const std::string content = "cat in the city"s;
	const std::vector<int> ratings = {1, 2, 3};
	// Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
	// находит нужный документ
	{
		SearchServer server("stops"s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		const auto found_docs = server.FindTopDocuments("in"s);
		ASSERT(
			found_docs.size() ==
			1); /// по возможности лучше использовать ASSERT_EQUAL, т.к. в случае срабатывания, он более информативный
		const Document& doc0 = found_docs[0];
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
void TestMinusWords()
{
	using std::string_literals::operator""s;
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
void TestSearch()
{
	using std::string_literals::operator""s;
	const int doc_id = 42;
	const std::string content = "cat in the city"s;
	const std::vector<int> ratings = {1, 2, 3};

	{
		SearchServer server("stops"s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		const auto found_docs = server.FindTopDocuments("cat"s);
		ASSERT_EQUAL(found_docs.size(), 1);
		const Document& doc0 = found_docs[0];
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
void TestCountDocuments()
{
	using std::string_literals::operator""s;
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
// возвращены все слова из поискового запроса, // присутствующие в документе. Если есть соответствие хотя бы по //
// одному минус-слову, должен возвращаться пустой список слов.
void TestMatchDocument()
{
	using std::string_literals::operator""s;
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
void TestRelevanceSort()
{
	using std::string_literals::operator""s;
	std::vector<std::string> docs = {"cat dog flower", "cat cat flower", "cat dog dog"};

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
void TestRating()
{
	using std::string_literals::operator""s;
	const int doc_id = 42;
	const std::string content = "cat in the city"s;
	const std::vector<int> ratings = {1, 2, 3};

	{
		SearchServer server("stops"s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		const auto found_docs = server.FindTopDocuments("cat"s);
		ASSERT_EQUAL(found_docs.size(), 1);
		const Document& doc0 = found_docs[0];
		ASSERT_EQUAL(doc0.id, doc_id);
		ASSERT_EQUAL(doc0.rating, 2);
	}
}

// Поиск документов, имеющих заданный статус.
void TestDocumentWithStatus()
{
	using std::string_literals::operator""s;
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
		const Document& doc0 = found_docs[0];
		ASSERT_EQUAL(doc0.id, 43);
		ASSERT_EQUAL(doc0.rating, 4);
	}
}

//Фильтрация результатов поиска с использованием предиката, задаваемого пользователем.
void TestDocumentPredicate()
{
	using std::string_literals::operator""s;
	SearchServer search_server("и в на"s);

	search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3});
	search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
	search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
	search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, {9});

	for (const Document &document :
		 search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status,
																	  int rating) { return document_id % 2 == 0; }))
	{
		ASSERT_EQUAL(document.id % 2, 0);
	}
}

bool double_equals(double a, double b, double epsilon = 1e-6)
{
	return std::abs(a - b) < epsilon;
}
//Корректное вычисление релевантности найденных документов.
void TestRelevance()
{
	using std::string_literals::operator""s;
	SearchServer search_server("и в на"s);
	std::vector<std::string> docs = {"white cat"s, "black cat"s, "orange dog"s, "ping pig"s};
	search_server.AddDocument(0, docs[0], DocumentStatus::ACTUAL, {8, -3});
	search_server.AddDocument(1, docs[1], DocumentStatus::ACTUAL, {7, 2, 7});
	search_server.AddDocument(2, docs[2], DocumentStatus::ACTUAL, {5, -12, 2, 1});
	search_server.AddDocument(3, docs[3], DocumentStatus::BANNED, {9});

	auto result = search_server.FindTopDocuments("cat");
	double actual_relevance = (1.0 / 2.0) * log(docs.size() / 2);
	ASSERT_EQUAL(search_server.GetDocumentCount(), docs.size());
	ASSERT(double_equals(result[0].relevance,
						 actual_relevance)); // Пользуюсь gtests , у них был и есть отдельный макрос для даблов
											 // ASSERT_DOUBLE_EQ ну не суть, поняли друг друга
}

void TestConstructors()
{
	using std::string_literals::operator""s;
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

void TestExceptions_Minuses()
{
	using std::string_literals::operator""s;
	{
		std::vector<std::string> vec = {std::string("ab"), std::string("b")};
		SearchServer s(vec);
		s.AddDocument(1, "text text"s, DocumentStatus::ACTUAL, {1, 2, 3});
		auto search = "--search"s;
		try
		{
			auto res = s.FindTopDocuments(search);
		}
		catch (const std::exception& e)
		{
			ASSERT_EQUAL("Query word " + search + " is invalid", e.what());
		}
	}
	{
		std::vector<std::string> vec = {std::string("word1"), std::string("word2")};
		SearchServer s(vec);
		s.AddDocument(1, "Lorem ipsum -word"s, DocumentStatus::ACTUAL, {1, 2, 3});
		s.AddDocument(2, "word word"s, DocumentStatus::ACTUAL, {1, 2, 3});
		std::vector<std::string_view> documents;
		DocumentStatus status;
		std::tie(documents, status) = s.MatchDocument(std::string("-Lorem word"), 1);
		ASSERT(documents.empty());
	}
}

void TestExceptions_ValidWord()
{
	using std::string_literals::operator""s;
	{
		std::vector<std::string> vec = {std::string("ab"), std::string("b")};
		SearchServer s(vec);
		s.AddDocument(1, "text text", DocumentStatus::ACTUAL, {1, 2, 3});
		std::string search = "скворец"s;
		search[1] = 0x1;
		try
		{
			auto res = s.FindTopDocuments(search);
		}
		catch (const std::exception& e)
		{
			ASSERT_EQUAL("Query word " + search + " is invalid", e.what());
		}
	}
}

void TestExceptions_VoidMinus()
{
	using std::string_literals::operator""s;
	{
		std::vector<std::string> vec = {std::string("ab"), std::string("b")};
		SearchServer s(vec);
		s.AddDocument(1, "text text", DocumentStatus::ACTUAL, {1, 2, 3});
		std::string search = "- скворец"s;
		try
		{
			auto res = s.FindTopDocuments(search);
		}
		catch (const std::exception& e)
		{
			ASSERT_EQUAL(std::string("Query word - is invalid"), e.what());
		}
	}
}

void TestExceptions_Ids()
{
	using std::string_literals::operator""s;
	{
		std::vector<std::string> vec = {std::string("ab"), std::string("b")};
		SearchServer s(vec);
		s.AddDocument(1, "text text", DocumentStatus::ACTUAL, {1, 2, 3});
		try
		{
			s.AddDocument(1, "text text2", DocumentStatus::ACTUAL, {1, 2, 3});
		}
		catch (const std::exception& e)
		{
			ASSERT_EQUAL(std::string("Invalid document_id"), e.what());
		}
	}
	{
		std::vector<std::string> vec = {std::string("ab"), std::string("b")};
		SearchServer s(vec);
		s.AddDocument(1, "text text", DocumentStatus::ACTUAL, {1, 2, 3});
		try
		{
			s.AddDocument(-1, "text text2", DocumentStatus::ACTUAL, {1, 2, 3});
		}
		catch (const std::exception& e)
		{
			ASSERT_EQUAL(std::string("Invalid document_id"), e.what());
		}
	}
}

// void TestExceptions_DocumentIndexes() {
// 	using std::string_literals::operator""s;
// 	{
// 		std::vector<std::string> vec = {std::string("ab"), std::string("b")};
// 		SearchServer s(vec);
// 		s.AddDocument(1, "text text", DocumentStatus::ACTUAL, {1, 2, 3});
// 		s.AddDocument(1000, "text text2", DocumentStatus::ACTUAL, {1, 2, 3});
// 		s.AddDocument(1001, "text text3", DocumentStatus::ACTUAL, {1, 2, 3});
// 		ASSERT_EQUAL(s.GetDocumentId(0), 1);
// 		ASSERT_EQUAL(s.GetDocumentId(2), 1001);
// 	}
// 	{
// 		std::vector<std::string> vec = {std::string("ab"), std::string("b")};
// 		SearchServer s(vec);
// 		s.AddDocument(1, "text text", DocumentStatus::ACTUAL, {1, 2, 3});
// 		try {
// 			s.GetDocumentId(10000);
// 		} catch (const std::exception &e) {
// 			ASSERT_EQUAL(std::string("Out of range in document_statuses_ratings_"), e.what());
// 		}
// 	}
// }

void TestExceptions_Empty()
{
	using std::string_literals::operator""s;
	{
		std::vector<std::string> vec = {std::string("ab"), std::string("b")};
		SearchServer s(vec);
		s.AddDocument(1, "text text", DocumentStatus::ACTUAL, {1, 2, 3});
		try
		{
			auto res = s.FindTopDocuments("");
		}
		catch (const std::exception& e)
		{
			ASSERT_EQUAL(std::string("Query word is empty"), e.what());
		}
	}
}

void TestExceptions_Undefined_Stop_Word()
{
	using std::string_literals::operator""s;
	{
		std::string stop_word1 = "stop";
		std::string stop_word2 = "stop";
		stop_word2[2] = 0x2;

		std::vector<std::string> vec = {stop_word1, stop_word2};

		try
		{
			SearchServer s(vec);
		}
		catch (const std::exception& e)
		{
			ASSERT_EQUAL("Some of stop words are invalid"s, e.what());
		}
	}
}

void Test_Queue()
{
	using std::string_literals::operator""s;
	SearchServer search_server("и в на"s);
	RequestQueue request_queue(search_server);

	search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
	search_server.AddDocument(2, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2, 3});
	search_server.AddDocument(3, "большой кот модный ошейник "s, DocumentStatus::ACTUAL, {1, 2, 8});
	search_server.AddDocument(4, "большой пёс скворец евгений"s, DocumentStatus::ACTUAL, {1, 3, 2});
	search_server.AddDocument(5, "большой пёс скворец василий"s, DocumentStatus::ACTUAL, {1, 1, 1});

	// 1439 запросов с нулевым результатом
	for (int i = 0; i < 1439; ++i)
	{
		request_queue.AddFindRequest("пустой запрос"s);
	}
	// все еще 1439 запросов с нулевым результатом
	request_queue.AddFindRequest("пушистый пёс"s);
	// новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
	request_queue.AddFindRequest("большой ошейник"s);
	// первый запрос удален, 1437 запросов с нулевым результатом
	request_queue.AddFindRequest("скворец"s);
	ASSERT_EQUAL(1439 - 2, request_queue.GetNoResultRequests());
}

void Test_Pagination()
{
	using std::string_literals::operator""s;
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

void AddDocument(SearchServer& s, size_t id, std::string str, DocumentStatus status, std::vector<int> ids)
{
	s.AddDocument(id, str, status, ids);
}

void DuplicatesTest()
{

	using std::string_literals::operator""s;
	SearchServer search_server("and with"s);

	AddDocument(search_server, 1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, {7, 2, 7});
	AddDocument(search_server, 2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2});

	// дубликат документа 2, будет удалён
	AddDocument(search_server, 3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2});

	// отличие только в стоп-словах, считаем дубликатом
	AddDocument(search_server, 4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, {1, 2});

	// множество слов такое же, считаем дубликатом документа 1
	AddDocument(search_server, 5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, {1, 2});

	// добавились новые слова, дубликатом не является
	AddDocument(search_server, 6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, {1, 2});

	// множество слов такое же, как в id 6, несмотря на другой порядок, считаем дубликатом
	AddDocument(search_server, 7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, {1, 2});

	// есть не все слова, не является дубликатом
	AddDocument(search_server, 8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, {1, 2});

	// слова из разных документов, не является дубликатом
	AddDocument(search_server, 9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, {1, 2});

	ASSERT_EQUAL(search_server.GetDocumentCount(), 9);
	RemoveDuplicates(search_server);
	ASSERT_EQUAL(search_server.GetDocumentCount(), 5);
}

void ReduceTest()
{
	using namespace std;
	SearchServer search_server("and with"s);

	int id = 0;
	for (const string& text : {
			 "funny pet and nasty rat"s,
			 "funny pet with curly hair"s,
			 "funny pet and not very nasty rat"s,
			 "pet with rat and rat and rat"s,
			 "nasty rat with curly hair"s,
		 })
	{
		search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
	}

	const vector<string> queries = {"nasty rat -not"s, "not very funny nasty pet"s, "curly hair"s};
	id = 0;
	for (const auto& documents : ProcessQueries(search_server, queries))
	{
		cout << documents.size() << " documents for query ["s << queries[id++] << "]"s << endl;
	}
}

void JoinAndReduce()
{
	using namespace std;
	SearchServer search_server("and with"s);

	int id = 0;
	for (const string& text : {
			 "funny pet and nasty rat"s,
			 "funny pet with curly hair"s,
			 "funny pet and not very nasty rat"s,
			 "pet with rat and rat and rat"s,
			 "nasty rat with curly hair"s,
		 })
	{
		search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
	}

	const vector<string> queries = {"nasty rat -not"s, "not very funny nasty pet"s, "curly hair"s};
	for (const Document& document : ProcessQueriesJoined(search_server, queries))
	{
		cout << "Document "s << document.id << " matched with relevance "s << document.relevance << endl;
	}
}

void RemoveExecutionPolicy()
{
	using namespace std;
	SearchServer search_server("and with"s);

	int id = 0;
	for (const string& text : {
			 "funny pet and nasty rat"s,
			 "funny pet with curly hair"s,
			 "funny pet and not very nasty rat"s,
			 "pet with rat and rat and rat"s,
			 "nasty rat with curly hair"s,
		 })
	{
		search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
	}

	const vector<string> queries = {"nasty rat -not"s, "not very funny nasty pet"s, "curly hair"s};
	search_server.RemoveDocument(std::execution::seq, 1);
}
void ParralelMatch()
{
	using namespace std;
	SearchServer search_server("and with"s);

	int id = 0;
	for (const string& text : {
			 "funny pet and nasty rat"s,
			 "funny pet with curly hair"s,
			 "funny pet and not very nasty rat"s,
			 "pet with rat and rat and rat"s,
			 "nasty rat with curly hair"s,
		 })
	{
		search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
	}

	const string query = "curly and funny -not"s;

	{
		const auto [words, status] = search_server.MatchDocument(query, 1);
		cout << words.size() << " words for document 1"s << endl;
		// 1 words for document 1
	}

	{
		const auto [words, status] = search_server.MatchDocument(execution::seq, query, 2);
		cout << words.size() << " words for document 2"s << endl;
		// 2 words for document 2
	}

	{
		const auto [words, status] = search_server.MatchDocument(execution::par, query, 3);
		cout << words.size() << " words for document 3"s << endl;
		// 0 words for document 3
	}
}
string GenerateWord(mt19937& generator, int max_length)
{
	const int length = uniform_int_distribution(1, max_length)(generator);
	string word;
	word.reserve(length);
	for (int i = 0; i < length; ++i)
	{
		word.push_back(uniform_int_distribution('a', 'z')(generator));
	}
	return word;
}

vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length)
{
	vector<string> words;
	words.reserve(word_count);
	for (int i = 0; i < word_count; ++i)
	{
		words.push_back(GenerateWord(generator, max_length));
	}
	words.erase(unique(words.begin(), words.end()), words.end());
	return words;
}

string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int word_count, double minus_prob = 0)
{
	string query;
	for (int i = 0; i < word_count; ++i)
	{
		if (!query.empty())
		{
			query.push_back(' ');
		}
		if (uniform_real_distribution<>(0, 1)(generator) < minus_prob)
		{
			query.push_back('-');
		}
		query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
	}
	return query;
}

vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count,
							   int max_word_count)
{
	vector<string> queries;
	queries.reserve(query_count);
	for (int i = 0; i < query_count; ++i)
	{
		queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
	}
	return queries;
}

template <typename ExecutionPolicy>
void Test(string_view mark, const SearchServer& search_server, const vector<string>& queries, ExecutionPolicy&& policy)
{
	LOG_DURATION(std::string(mark));
	double total_relevance = 0;
	for (const string_view query : queries)
	{
		for (const auto& document : search_server.FindTopDocuments(policy, query))
		{
			total_relevance += document.relevance;
		}
	}
	cout << total_relevance << endl;
}

//#define TEST(policy) Test(#mode, search_server, queries, execution::policy)

void ParralelFind()
{
	mt19937 generator;

	const auto dictionary = GenerateDictionary(generator, 1000, 10);
	const auto documents = GenerateQueries(generator, dictionary, 10'000, 70);

	SearchServer search_server(dictionary[0]);
	for (size_t i = 0; i < documents.size(); ++i)
	{
		search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
	}

	const auto queries = GenerateQueries(generator, dictionary, 100, 70);

	{
		LOG_DURATION("SEQ");
		Test("seq"s, search_server, queries, std::execution::seq);
	}
	{
		LOG_DURATION("PAR");
		Test("par"s, search_server, queries, std::execution::par);
	}

}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer()
{
//	// 1
//	RUN_TEST(TestAddDocument);
//	// 2
//	RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
//	// 3
//	RUN_TEST(TestMinusWords);
//	// 4
//	RUN_TEST(TestMatchDocument);
//	// 5
//	RUN_TEST(TestRelevanceSort);
//	// 6
//	RUN_TEST(TestRating);
//	// 7
//	RUN_TEST(TestDocumentPredicate);
//	// 8
//	RUN_TEST(TestDocumentWithStatus);
//	// 9
//	RUN_TEST(TestRelevance);
//	// 10
//	RUN_TEST(TestSearch);
//	// 11
//	RUN_TEST(TestCountDocuments);
//	// 12
//	//	RUN_TEST(TestConstructors);
//	// 13
//	//	RUN_TEST(TestExceptions_Minuses);
//	// 14
//	//	RUN_TEST(TestExceptions_ValidWord);
//	// 15
//	//	RUN_TEST(TestExceptions_VoidMinus);
//	//	// 16
//	//	RUN_TEST(TestExceptions_Ids);
//	// 17
//	// RUN_TEST(TestExceptions_DocumentIndexes);
//	// 18
//	//	RUN_TEST(TestExceptions_Empty);
//	//	// 19
//	//	RUN_TEST(TestExceptions_Undefined_Stop_Word);
//	//	// 20
//	RUN_TEST(Test_Queue);
//	//	// 21
//	RUN_TEST(Test_Pagination);
//	//	// 22
//	RUN_TEST(DuplicatesTest);
//	//	// 23
//	RUN_TEST(ReduceTest);
//	// 24
//	RUN_TEST(JoinAndReduce);
//	// 25
//	RUN_TEST(RemoveExecutionPolicy);
//	// 26
//	RUN_TEST(ParralelMatch);
	// 27
	RUN_TEST(ParralelFind);
}

int main()
{
	TestSearchServer();
}

// int main(int argc, char** argv)
//{
//	::testing::InitGoogleTest(&argc, argv);
//	return RUN_ALL_TESTS();
//}