#include "process_queries.h"
#include "execution"

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server,
												  const std::vector<std::string>& queries)
{
	std::vector<std::vector<Document>> result(queries.size());

	std::transform(std::execution::par, queries.begin(), queries.end(), result.begin(),
				   [&search_server](const std::string query) { return search_server.FindTopDocuments(query); });
	return result;
}
std::vector<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries)
{
	std::vector<Document> result;
	for (const auto& local_documents : ProcessQueries(search_server, queries))
	{
		result.insert(result.end(), local_documents.begin(), local_documents.end());
	}
	return result;
}
