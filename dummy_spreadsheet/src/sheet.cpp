#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    CheckPosition_(pos);
    if (!text.empty()) {
        positions_.insert(pos);
    }
    if (cells_.count(pos) == 0) {
        AllocCell_(pos, std::move(text));
    } else {
        cells_[pos]->Set(std::move(text));
    }
}

const CellInterface *Sheet::GetCell(Position pos) const {
    return const_cast<Sheet *>(this)->GetCell(pos);
}

CellInterface *Sheet::GetCell(Position pos) {
    CheckPosition_(pos);
    if (cells_.count(pos) == 0) {
        return nullptr;
    }
    Cell *cell = cells_.at(pos).get();
    if (!cell) {
        return nullptr;
    }
    return cell;
}

void Sheet::ClearCell(Position pos) {
    SetCell(pos, std::string());
    positions_.erase(pos);
}


Size Sheet::GetPrintableSize() const {
    int m_r = 0;
    int m_c = 0;
    for (auto pos : positions_) {
        m_r = std::max(m_r, pos.row + 1);
        m_c = std::max(m_c, pos.col + 1);
    }
    return {m_r, m_c};
}

void Sheet::PrintValues(std::ostream &output) const {
    auto value_f = [&output](const Cell *cell) {
        if (cell) {
            std::visit([&output](const auto &x) {
                output << x;
            }, cell->GetValue());
        }
    };
    PrintSheet(output, value_f);
}

void Sheet::PrintTexts(std::ostream &output) const {
    auto text_f = [&output](const Cell *cell) {
        if (cell) {
            output << cell->GetText();
        }
    };
    PrintSheet(output, text_f);
}

void Sheet::CheckPosition_(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Wrong pos"s);
    }
}

void Sheet::AllocCell_(Position pos, std::string text) {

    Cell *cell = new Cell(*this, pos);
    cell->Set(std::move(text));
    cells_[pos].reset(cell);

}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}