#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

class Cell;

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

    template<typename P>
    void PrintSheet(std::ostream &output, P print) const {
        Size scope = GetPrintableSize();
        for (int i = 0; i < scope.rows; ++i) {
            for (int j = 0; j < scope.cols - 1; ++j) {
                Position pos{i, j};
                print((cells_.count(pos) != 0) ? cells_.at(pos).get() : nullptr);
                output << '\t';
            }
            Position pos{i, scope.cols - 1};
            print((cells_.count(pos) != 0) ? cells_.at(pos).get() : nullptr);
            output << '\n';
        }
    }
};

