#include "common.h"

#include <cctype>
#include <sstream>


// че за эксель то такой ?! MAX_POS_LETTER_COUNT ? AAAA столбца быть не можнт ?
const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = {-1, -1};

// Реализуйте методы:
bool Position::operator==(const Position rhs) const {
    return row == rhs.row && col == rhs.col;
}

bool Position::operator<(const Position rhs) const {
    return std::make_pair(row, col) < std::make_pair(rhs.row, rhs.col);
}

bool Position::IsValid() const {
    return row >= 0 && col >= 0 && row < MAX_ROWS && col < MAX_COLS;
}

std::string Position::ToString() const {
    if (!IsValid()) {
        return "";
    }

    std::string result;
    result.reserve(MAX_POSITION_LENGTH);
    int c = col;
    while (c >= 0) {
        result.insert(result.begin(), 'A' + c % LETTERS);
        c = c / LETTERS - 1;
    }

    result += std::to_string(row + 1);

    return result;
}

Position Position::FromString(std::string_view str) {
    auto it = std::find_if(str.begin(), str.end(), [](const char c) {
        return !(std::isalpha(c) && std::isupper(c));
    });
    auto let = str.substr(0, it - str.begin());
    auto number = str.substr(it - str.begin());

    if (let.empty() || number.empty()) {
        return Position::NONE;
    }
    if (let.size() > MAX_POS_LETTER_COUNT) {
        return Position::NONE;
    }

    if (!std::isdigit(number[0])) {
        return Position::NONE;
    }

    int row;
    std::istringstream row_in{std::string{number}};
    if (!(row_in >> row) || !row_in.eof()) {
        return Position::NONE;
    }

    int col = 0;
    for (char ch : let) {
        col *= LETTERS;
        col += ch - 'A' + 1;
    }

    return Position{row - 1, col - 1};
}