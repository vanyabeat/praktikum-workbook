#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <memory>

Cell::EmptyImpl::EmptyImpl()
        : Impl() {}

Cell::Value Cell::EmptyImpl::GetValue() const {
    return std::string();
}

std::string Cell::EmptyImpl::GetText() const {
    return std::string();
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const {
    return {};
}

Cell::TextImpl::TextImpl(std::string str)
        : Impl(), str_(std::move(str)) {}

Cell::Value Cell::TextImpl::GetValue() const {
    return (str_.front() != ESCAPE_SIGN) ? str_ : str_.substr(1);
}

std::string Cell::TextImpl::GetText() const {
    return str_;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const {
    return std::vector<Position>();
}


Cell::FormulaImpl::FormulaImpl(std::string str,
                               const SheetInterface &sheet)
        : Impl(), sheet_(sheet), formula_(ParseFormula(str)) {}

Cell::Value Cell::FormulaImpl::GetValue() const {
    auto res = formula_->Evaluate(sheet_);
    if (std::holds_alternative<double>(res)) {
        return std::get<double>(res);
    }
    return std::get<FormulaError>(res);
}

std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

std::unique_ptr<Cell::Impl> Cell::MakeImpl_(std::string text) const {
    using namespace std::literals;
    if (text.empty()) {
        return std::make_unique<EmptyImpl>();
    }
    if (text.front() == FORMULA_SIGN && text.size() > 1) {
        try {
            return std::make_unique<FormulaImpl>(text.substr(1), *sheet_);
        }
        catch (...) {
            throw FormulaException("Error with formula"s);
        }
    } else {
        return std::make_unique<TextImpl>(std::move(text));
    }
}

// Реализуйте следующие методы


Cell::~Cell() {}

void Cell::Set(std::string text) {
    using namespace std;

    auto tmp = MakeImpl_(std::move(text));
    if (IsCyclic(tmp.get())) {
        throw CircularDependencyException("Error formula is сircular"s);
    }
    impl_ = std::move(tmp);
    InitializeCell();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    UpdateCache();
    return cache_.value();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

Cell *Cell::AllocCell(Position pos) const {
//    Cell *cell = dynamic_cast<Cell *>(sheet_.GetCell(pos));
    Cell *cell = static_cast<Cell *>(sheet_->GetCell(pos));
    if (!cell) {
        sheet_->SetCell(pos, std::string());
        cell = AllocCell(pos);
    }
    return cell;
}

Cell::Cell(Sheet *sheet, Position pos)
        : CellInterface(), sheet_(sheet), pos_(pos), impl_(std::make_unique<EmptyImpl>()) {


}

bool Cell::IsEmpty() const {
    return impl_->GetText().empty();
}

Cell::Cell() : CellInterface(), sheet_(nullptr), pos_(Position{}), impl_(std::make_unique<EmptyImpl>()) {

}

std::vector<Position> Cell::GetReferencedCells() const {
    return std::vector<Position>();
}

bool Cell::IsRef() const {
    return !dependencies_o_.empty() || !dependencies_i_.empty();
}
