#include "cell.h"
#include "common.h"
#include "formula.h"

#include "sheet.h"
#include <gtest/gtest.h>

inline std::ostream &operator<<(std::ostream &output, const CellInterface::Value &value) {
    std::visit([&](const auto &x) { output << x; }, value);
    return output;
}

std::unique_ptr<CellInterface> CreateCell(const std::string &str) {
    auto pos = Position{0, 0};

    std::unique_ptr<CellInterface> cell = std::make_unique<Cell>(nullptr, pos);
    cell->Set(str);
    return cell;
}

TEST(Cell, MacroTest) {

    auto simple_text = CreateCell("simple_text");
    ASSERT_EQ(simple_text
                      ->

                              GetText(),

              "simple_text");
    ASSERT_EQ(std::get<std::string>(simple_text->GetValue()),
              "simple_text");

    auto empty_apostroph = CreateCell("'");
    ASSERT_EQ(empty_apostroph
                      ->

                              GetText(),

              "'");
    ASSERT_EQ(std::get<std::string>(empty_apostroph->GetValue()),
              "");

    auto apostroph = CreateCell("'apostroph");
    ASSERT_EQ(apostroph
                      ->

                              GetText(),

              "'apostroph");
    ASSERT_EQ(std::get<std::string>(apostroph->GetValue()),
              "apostroph");

    auto text_formula = CreateCell("'=1+2");
    ASSERT_EQ(text_formula
                      ->

                              GetText(),

              "'=1+2");
    ASSERT_EQ(std::get<std::string>(text_formula->GetValue()),
              "=1+2");

    auto empty_formula = CreateCell("=");
    ASSERT_EQ(empty_formula
                      ->

                              GetText(),

              "=");
    ASSERT_EQ(std::get<std::string>(empty_formula->GetValue()),
              "=");

    auto formula = CreateCell("=1+2");
    ASSERT_EQ(formula
                      ->

                              GetText(),

              "=1+2");
    ASSERT_EQ(std::get<double>(formula->GetValue()),
              3);

    auto switch_text = CreateCell("1+2");
    ASSERT_EQ(switch_text
                      ->

                              GetText(),

              "1+2");
    ASSERT_EQ(std::get<std::string>(switch_text->GetValue()),
              "1+2");

    switch_text->Set("=1+2");
    ASSERT_EQ(switch_text
                      ->

                              GetText(),

              "=1+2");
    ASSERT_EQ(std::get<double>(switch_text->GetValue()),
              3);

    switch_text->Set("=1/0");
    ASSERT_EQ(switch_text
                      ->

                              GetText(),

              "=1/0");
    std::cout <<
              std::get<FormulaError>(switch_text
                                             ->

                                                     GetValue()

              ) <<
              std::endl;

    std::cout << "All tests passed" <<
              std::endl;
}