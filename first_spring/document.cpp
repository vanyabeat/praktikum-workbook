#include "document.h"
Document::Document(int id, double relevance, int rating)
	: id(id), relevance(relevance), rating(rating) {
}
Document::Document(int id, double relevance, int rating, DocumentStatus status)
	: id(id), relevance(relevance), rating(rating), status(status) {
}
std::ostream &operator<<(std::ostream &out, Document document) {
	out << "{ document_id = " << document.id << ", relevance = " << document.relevance << ", rating = " << document.rating << " }";
	return out;
}
