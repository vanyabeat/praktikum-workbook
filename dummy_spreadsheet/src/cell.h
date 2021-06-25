#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"
#include <variant>

class Sheet;

class Cell : public CellInterface {
public:
    Cell();

    Cell(Sheet *sheet, Position pos);

    ~Cell();

    void Set(std::string text) override;

    void Clear();

    Value GetValue() const override;

    std::string GetText() const override;

    bool IsEmpty() const;

    std::vector<Position> GetReferencedCells() const override;

    bool IsRef() const;

private:

    class Impl {
    public:
        virtual ~Impl() = default;

        virtual Value GetValue() const = 0;

        virtual std::string GetText() const = 0;

        virtual std::vector<Position> GetReferencedCells() const = 0;
    };

    std::unique_ptr<Impl> MakeImpl_(std::string text) const;

    class EmptyImpl : public Impl {
    public:
        EmptyImpl();

        Value GetValue() const override;

        std::string GetText() const override;

        std::vector<Position> GetReferencedCells() const override;
    };

    class TextImpl : public Impl {
    public:
        TextImpl(std::string str);

        Value GetValue() const override;

        std::string GetText() const override;

        std::vector<Position> GetReferencedCells() const override;

    private:
        std::string str_;
    };

    class FormulaImpl : public Impl {
    public:
        FormulaImpl(std::string str,
                    const SheetInterface &sheet);

        Value GetValue() const override;

        std::string GetText() const override;

        std::vector<Position> GetReferencedCells() const override;

    private:
        const SheetInterface &sheet_;
        std::unique_ptr<FormulaInterface> formula_;
    };

    Sheet *sheet_;
    Position pos_;
    std::unique_ptr<Impl> impl_;

    Cell *AllocCell_(Position pos) const;

    PositionsSet dependencies_o_;
    mutable std::optional<Value> cache_;
    PositionsSet dependencies_i_;

    bool IsCyclicFormula_(const PositionsSet &dependencies, PositionsSet &inspectors) const;

    bool IsCyclic_(const Impl *impl) const;

    void UpdateCache_() const;

    void InvalidateCache_();

    void DeleteDependencies_();

    void CreateDependencies_();

    void InvalidateAllDependentCaches_(const PositionsSet &deps,
                                       PositionsSet &invalids);

    void InitializeCell_();
};