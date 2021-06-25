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
    if (IsCyclic_(tmp.get())) {
        throw CircularDependencyException("Error formula is сircular"s);
    }
    impl_ = std::move(tmp);
    InitializeCell_();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    UpdateCache_();
    return cache_.value();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

Cell *Cell::AllocCell_(Position pos) const {
//    Cell *cell = dynamic_cast<Cell *>(sheet_.GetCell(pos));
    Cell *cell = static_cast<Cell *>(sheet_->GetCell(pos));
    if (!cell) {
        sheet_->SetCell(pos, std::string());
        cell = AllocCell_(pos);
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
    return impl_->GetReferencedCells();
}

bool Cell::IsRef() const {
    return !dependencies_o_.empty() || !dependencies_i_.empty();
}

bool Cell::IsCyclicFormula_(const PositionsSet &dependencies, PositionsSet &inspectors) const {
    if (dependencies.count(pos_) != 0) {
        return true;
    }
    for (auto pos : dependencies) {
        if (!pos.IsValid() || inspectors.count(pos) != 0) {
            continue;
        }
        inspectors.insert(pos);
        Cell *cell = AllocCell_(pos);
        if (IsCyclicFormula_(cell->dependencies_o_, inspectors)) {
            return true;
        }
    }
    return false;
}

void Cell::InitializeCell_() {
    InvalidateCache_();
    DeleteDependencies_();
    CreateDependencies_();
    PositionsSet invalids;
    InvalidateAllDependentCaches_(dependencies_i_, invalids);
}

void Cell::InvalidateAllDependentCaches_(const PositionsSet &deps, PositionsSet &invalids) {
    for (auto pos : deps) {
        if (!pos.IsValid()) {
            continue;
        }
        Cell *cell = AllocCell_(pos);
        cell->InvalidateCache_();
        invalids.insert(pos);
        cell->InvalidateAllDependentCaches_(cell->dependencies_i_, invalids);
    }
}

void Cell::CreateDependencies_() {
    for (auto pos : GetReferencedCells()) {
        if (!pos.IsValid()) {
            continue;
        }
        dependencies_o_.insert(pos);
        Cell *cell = AllocCell_(pos);
        cell->dependencies_i_.insert(pos);
    }
}

void Cell::DeleteDependencies_() {
    for (auto pos : dependencies_o_) {
        if (!pos.IsValid()) {
            continue;
        }
        Cell *cell = AllocCell_(pos);
        cell->dependencies_i_.erase(pos);
    }
    dependencies_o_.clear();
}

void Cell::InvalidateCache_() {
    cache_ = std::nullopt;
}

void Cell::UpdateCache_() const {
    if (!cache_.has_value()) {
        cache_ = impl_->GetValue();
    }
}

bool Cell::IsCyclic_(const Cell::Impl *impl) const {
    auto positions = impl->GetReferencedCells();
    PositionsSet dependencies(positions.begin(), positions.end());
    PositionsSet inspectors;
    return IsCyclicFormula_(dependencies, inspectors);
}
