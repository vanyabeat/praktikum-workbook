#include <gtest/gtest.h>
#include "common.h"
#include "FormulaAST.h"

double ExecuteASTFormula(const std::string& expression) {
    return ParseFormulaAST(expression).Execute();
}

TEST(AST, Execute) {
    ASSERT_EQ(ExecuteASTFormula("1"), 1.0);
    ASSERT_EQ(ExecuteASTFormula("1+2*3-4/5"), 6.2);
    try {
        ExecuteASTFormula("1/0");
    } catch (const FormulaError& fe) {
        std::cout << fe.what() << std::endl;
    }

    std::cout << "Tests Passed" << std::endl;
}