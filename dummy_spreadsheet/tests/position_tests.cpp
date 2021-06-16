#include <gtest/gtest.h>
#include "common.h"

inline std::ostream &operator<<(std::ostream &output, Position pos) {
    return output << "(" << pos.row << ", " << pos.col << ")";
}

inline Position operator "" _pos(const char *str, std::size_t) {
    return Position::FromString(str);
}

TEST(Position, TestPositionAndStringConversion) {
    auto test_single = [](Position pos, std::string_view str) {
        ASSERT_EQ(pos.ToString(), str);
        ASSERT_EQ(Position::FromString(str), pos);
    };

    for (int i = 0; i < 25; ++i) {
        test_single(Position{i, i}, char('A' + i) + std::to_string(i + 1));
    }

    test_single(Position{0, 0}, "A1");
    test_single(Position{0, 1}, "B1");
    test_single(Position{0, 25}, "Z1");
    test_single(Position{0, 26}, "AA1");
    test_single(Position{0, 27}, "AB1");
    test_single(Position{0, 51}, "AZ1");
    test_single(Position{0, 52}, "BA1");
    test_single(Position{0, 53}, "BB1");
    test_single(Position{0, 77}, "BZ1");
    test_single(Position{0, 78}, "CA1");
    test_single(Position{0, 701}, "ZZ1");
    test_single(Position{0, 702}, "AAA1");
    test_single(Position{136, 2}, "C137");
    test_single(Position{Position::MAX_ROWS - 1, Position::MAX_COLS - 1}, "XFD16384");
}

TEST(Position, TestPositionToStringInvalid) {
    ASSERT_EQ((Position::NONE).ToString(), "");
    ASSERT_EQ((Position{-10, 0}).ToString(), "");
    ASSERT_EQ((Position{1, -3}).ToString(), "");
}

TEST(Position, Invalids) {
    ASSERT_TRUE(!Position::FromString("").IsValid());
    ASSERT_TRUE(!Position::FromString("A").IsValid());
    ASSERT_TRUE(!Position::FromString("1").IsValid());
    ASSERT_TRUE(!Position::FromString("e2").IsValid());
    ASSERT_TRUE(!Position::FromString("A0").IsValid());
    ASSERT_TRUE(!Position::FromString("A-1").IsValid());
    ASSERT_TRUE(!Position::FromString("A+1").IsValid());
    ASSERT_TRUE(!Position::FromString("R2D2").IsValid());
    ASSERT_TRUE(!Position::FromString("C3PO").IsValid());
    ASSERT_TRUE(!Position::FromString("XFD16385").IsValid());
    ASSERT_TRUE(!Position::FromString("XFE16384").IsValid());
    ASSERT_TRUE(!Position::FromString("A1234567890123456789").IsValid());
    ASSERT_TRUE(!Position::FromString("ABCDEFGHIJKLMNOPQRS8").IsValid());
}