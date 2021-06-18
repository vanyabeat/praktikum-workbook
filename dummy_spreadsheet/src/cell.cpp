#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

Cell::EmptyImpl::EmptyImpl()
        : Impl() {}

Cell::Value Cell::EmptyImpl::GetValue() const {
    return std::string();
}

std::string Cell::EmptyImpl::GetText() const {
    return std::string();
}


Cell::TextImpl::TextImpl(std::string str)
        : Impl(), str_(std::move(str)) {}

Cell::Value Cell::TextImpl::GetValue() const {
    return (str_.front() != ESCAPE_SIGN) ? str_ : str_.substr(1);
}

std::string Cell::TextImpl::GetText() const {
    return str_;
}


Cell::FormulaImpl::FormulaImpl(std::string str)
        : Impl(), formula_(std::move(ParseFormula(str))) {}

Cell::Value Cell::FormulaImpl::GetValue() const {
    auto res = formula_->Evaluate();
    if (std::holds_alternative<double>(res)) {
        return std::get<double>(res);
    }
    return std::get<FormulaError>(res);
}

std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_->GetExpression();
}

// Реализуйте следующие методы
Cell::Cell()
        : CellInterface(), impl_(std::make_unique<EmptyImpl>()) {}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    using namespace std::literals;
    auto tmp = MakeImpl_(std::move(text));
    impl_ = std::move(tmp);
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::unique_ptr<Cell::Impl> Cell::MakeImpl_(std::string text) const {
    using namespace std::literals;
    if (text.empty()) {
        return std::make_unique<EmptyImpl>();
    }
    if (text.front() == FORMULA_SIGN && text.size() > 1) {
        try {
            return std::make_unique<FormulaImpl>(std::move(text.substr(1)));
        }
        catch (...) {
            throw FormulaException("Error with formula"s);
        }
    } else {
        return std::make_unique<TextImpl>(std::move(text));
    }
}
