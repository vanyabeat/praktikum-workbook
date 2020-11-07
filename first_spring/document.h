#pragma once
#include <iostream>

enum class DocumentStatus {
	ACTUAL,
	IRRELEVANT,
	BANNED,
	REMOVED
};

struct Document {
	Document(int id_, double relevance_, int rating_, DocumentStatus status_);
	Document(int id_, double relevance_, int rating_);
	int id = 0;
	double relevance = 0.0;
	int rating = 0;
	DocumentStatus status;
};

std::ostream &operator<<(std::ostream &out, Document document);

