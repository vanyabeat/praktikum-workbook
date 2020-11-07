#include "document.h"

Document::Document(int id_, double relevance_, int rating_, DocumentStatus status_)
	: id(id_), relevance(relevance_), rating(rating_), status(status_) {
}
Document::Document(int id_, double relevance_, int rating_)
	: id(id_), relevance(relevance_), rating(rating_) {
}
std::ostream &operator<<(std::ostream &out, Document document) {
	out << "{ document_id = " << document.id << ", relevance = " << document.relevance << ", rating = " << document.rating << " }";
	return out;
}
