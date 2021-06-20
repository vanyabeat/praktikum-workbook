#include "common.h"
#include <gtest/gtest.h>

inline std::ostream &operator<<(std::ostream &output, Position pos) {
    return output << "(" << pos.row << ", " << pos.col << ")";
}

inline Position operator "" _pos(const char *str, std::size_t) {
    return Position::FromString(str);
}

inline std::ostream &operator<<(std::ostream &output, Size size) {
    return output << "(" << size.rows << ", " << size.cols << ")";
}

inline std::ostream &operator<<(std::ostream &output, const CellInterface::Value &value) {
    std::visit(
            [&](const auto &x) {
                output << x;
            },
            value);
    return output;
}


TEST(Sheet, TestEmpty) {
    auto sheet = CreateSheet();
    ASSERT_EQ(sheet->GetPrintableSize(), (Size{0, 0}));
}

TEST(Sheet, TestInvalidPosition) {
    auto sheet = CreateSheet();
    try {
        sheet->SetCell(Position{-1, 0}, "");
    } catch (const InvalidPositionException &) {
    }
    try {
        sheet->GetCell(Position{0, -2});
    } catch (const InvalidPositionException &) {
    }
    try {
        sheet->ClearCell(Position{Position::MAX_ROWS, 0});
    } catch (const InvalidPositionException &) {
    }
}

TEST(Sheet, TestSetCellPlainText) {
    auto sheet = CreateSheet();

    auto checkCell = [&](Position pos, std::string text) {
        sheet->SetCell(pos, text);
        CellInterface *cell = sheet->GetCell(pos);
        ASSERT_TRUE(cell != nullptr);
        ASSERT_EQ(cell->GetText(), text);
        ASSERT_EQ(std::get<std::string>(cell->GetValue()), text);
    };

    checkCell("A1"_pos, "Hello");
    checkCell("A1"_pos, "World");
    checkCell("B2"_pos, "Purr");
    checkCell("A3"_pos, "Meow");

    const SheetInterface &constSheet = *sheet;
    ASSERT_EQ(constSheet.GetCell("B2"_pos)->GetText(), "Purr");

    sheet->SetCell("A3"_pos, "'=escaped");
    CellInterface *cell = sheet->GetCell("A3"_pos);
    ASSERT_EQ(cell->GetText(), "'=escaped");
    ASSERT_EQ(std::get<std::string>(cell->GetValue()), "=escaped");
}

TEST(Sheet, TestClearCell) {
    auto sheet = CreateSheet();

    sheet->SetCell("C2"_pos, "Me gusta");
    sheet->ClearCell("C2"_pos);
    ASSERT_TRUE(sheet->GetCell("C2"_pos) == nullptr);

    sheet->ClearCell("A1"_pos);
    sheet->ClearCell("J10"_pos);
}

TEST(Sheet, TestPrint) {
    auto sheet = CreateSheet();
    sheet->SetCell("A2"_pos, "meow");
    sheet->SetCell("B2"_pos, "=1+2");
    sheet->SetCell("A1"_pos, "=1/0");

    ASSERT_EQ(sheet->GetPrintableSize(), (Size{2, 2}));

    std::ostringstream texts;
    sheet->PrintTexts(texts);
    ASSERT_EQ(texts.str(), "=1/0\t\nmeow\t=1+2\n");

    std::ostringstream values;
    sheet->PrintValues(values);
    ASSERT_EQ(values.str(), "#DIV/0!\t\nmeow\t3\n");

    sheet->ClearCell("B2"_pos);
    ASSERT_EQ(sheet->GetPrintableSize(), (Size{2, 1}));
}