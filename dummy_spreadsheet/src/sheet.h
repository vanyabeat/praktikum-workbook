#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Sheet : public SheetInterface {
public:
    using CellsMap = std::unordered_map<Position, std::unique_ptr<Cell>, CellHasher>;

    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface *GetCell(Position pos) const override;

    CellInterface *GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream &output) const override;

    void PrintTexts(std::ostream &output) const override;

    // Можете дополнить ваш класс нужными полями и методами


private:
    // Можете дополнить ваш класс нужными полями и методами
    CellsMap cells_;
    PositionsSet positions_;

    void CheckPosition_(Position pos) const;

    void AllocCell_(Position pos, std::string text);
};