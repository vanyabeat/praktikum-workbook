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

TEST(Sheet, TestSetCell) {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "1");
    sheet->SetCell("A2"_pos, "=A1+1");
    sheet->SetCell("A3"_pos, "=A2+2");

    ASSERT_EQ(sheet->GetPrintableSize(), (Size{3, 1}));

    std::ostringstream texts;
    sheet->PrintValues(texts);
    ASSERT_EQ(texts.str(), "1\n2\n4\n");
}

TEST(Sheet, TestSetCell2) {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "2");
    sheet->SetCell("A2"_pos, "=A1-1");
    sheet->SetCell("A3"_pos, "=A2+A1");

    ASSERT_EQ(sheet->GetPrintableSize(), (Size{3, 1}));

    std::ostringstream texts;
    sheet->PrintValues(texts);
    ASSERT_EQ(texts.str(), "2\n1\n3\n");
}

TEST(Sheet, TestSetCell3) {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "2");
    sheet->SetCell("A2"_pos, "=A1/0");
    sheet->SetCell("A3"_pos, "=A2+A1");

    ASSERT_EQ(sheet->GetPrintableSize(), (Size{3, 1}));

    std::ostringstream texts;
    sheet->PrintValues(texts);
    ASSERT_EQ(texts.str(), "2\n#DIV/0!\n#DIV/0!\n");
}

TEST(Sheet, TestSetCell4) {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "1");
    sheet->SetCell("A2"_pos, "=A1*-1");
    sheet->SetCell("A3"_pos, "=A2");

    ASSERT_EQ(sheet->GetPrintableSize(), (Size{3, 1}));

    std::ostringstream texts;
    sheet->PrintValues(texts);
    ASSERT_EQ(texts.str(), "1\n-1\n-1\n");
}

TEST(Sheet, TestSetCell5) {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "1");
    sheet->SetCell("B1"_pos, "100");
    sheet->SetCell("A3"_pos, "=A1+B1");

    ASSERT_EQ(sheet->GetPrintableSize(), (Size{3, 2}));

    std::ostringstream texts;
    sheet->PrintValues(texts);
    ASSERT_EQ(texts.str(), "1\t100\n\t\n101\t\n");
}

TEST(Sheet, TestCache) {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "1");
    sheet->SetCell("A2"_pos, "2");
    sheet->SetCell("A3"_pos, "=A1+A2");

    ASSERT_EQ(sheet->GetPrintableSize(), (Size{3, 1}));
    auto value = sheet->GetCell("A3"_pos)->GetValue();
    ASSERT_DOUBLE_EQ(std::get<double>(value), 3);
    sheet->SetCell("A1"_pos, "100");
    auto value_new = sheet->GetCell("A3"_pos)->GetValue();
    ASSERT_DOUBLE_EQ(std::get<double>(value_new), 3);
}

TEST(Sheet, TestSetCell6) {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "1");
    sheet->SetCell("B1"_pos, "100");
    sheet->SetCell("A3"_pos, "=A1+B1");

    ASSERT_EQ(sheet->GetPrintableSize(), (Size{3, 2}));

    std::ostringstream texts;
    sheet->PrintValues(texts);
    ASSERT_EQ(texts.str(), "1\t100\n\t\n101\t\n");
}

TEST(Sheet, TestClearCell1) {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "1");
    sheet->SetCell("B1"_pos, "2");
    sheet->SetCell("A2"_pos, "3");
    sheet->SetCell("B2"_pos, "4");

    ASSERT_EQ(sheet->GetPrintableSize(), (Size{2, 2}));
    sheet->ClearCell("A1"_pos);
    sheet->ClearCell("B1"_pos);
    sheet->ClearCell("A2"_pos);
    sheet->ClearCell("B2"_pos);

    ASSERT_EQ(sheet->GetPrintableSize(), (Size{0, 0}));
}

TEST(Sheet, TestSetCell8) {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "1");
    sheet->SetCell("B1"_pos, "100");
    sheet->SetCell("A3"_pos, "=A1+B1");

    ASSERT_EQ(sheet->GetPrintableSize(), (Size{3, 2}));

    std::ostringstream texts;
    sheet->PrintValues(texts);
    ASSERT_EQ(texts.str(), "1\t100\n\t\n101\t\n");
}

TEST(Sheet, Throws) {
    auto sheet = CreateSheet();
    ASSERT_THROW(sheet->SetCell("A1"_pos, "=A1"), CircularDependencyException);
}

TEST(Sheet, Size) {
    auto sheet = CreateSheet();
    sheet->SetCell("ABC123"_pos, "1");
    ASSERT_EQ(sheet->GetPrintableSize(), (Size{123, 731}));
}

TEST(Sheet, GetCell) {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "1");
    ASSERT_EQ(sheet->GetCell("A2"_pos), nullptr);
}

TEST(Sheet, GetCell2) {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "1");
    sheet->SetCell("AB2"_pos, "1");
    sheet->SetCell("A2"_pos, "=AB2+1+A1");
    auto value = sheet->GetCell("A2"_pos)->GetValue();
    ASSERT_DOUBLE_EQ(std::get<double>(value), 3);

}

TEST(Sheet, CircularDependecies) {
    using namespace std;
    auto sheet = CreateSheet();

    sheet->SetCell("A1"_pos, "=E5*5"s);
    sheet->SetCell("B2"_pos, "=A1+1"s);
    sheet->SetCell("C3"_pos, "=A1+B2"s);
    sheet->SetCell("D4"_pos, "=C3+A1+B2"s);

    ASSERT_THROW(sheet->SetCell("E5"_pos, "= B2/C3-D4+A1"s), CircularDependencyException);

}

TEST(Sheet, Deps){
    using namespace std;
    auto sheet = CreateSheet();

    sheet->SetCell("A1"_pos, "3"s);
    sheet->SetCell("A2"_pos, "=A1 + 2"s);
    sheet->SetCell("A3"_pos, "=A2 + 3"s);
    sheet->SetCell("A4"_pos, "=A2 + A1"s);

    ASSERT_TRUE(sheet->GetCell("A1"_pos)->GetReferencedCells().empty());
    ASSERT_EQ(sheet->GetCell("A2"_pos)->GetReferencedCells().size() , 1);
    ASSERT_EQ(sheet->GetCell("A3"_pos)->GetReferencedCells().size() , 2);
    ASSERT_EQ(sheet->GetCell("A4"_pos)->GetReferencedCells().size() , 2);
}