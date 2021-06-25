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

    Cell *AllocCell(Position pos) const;

    PositionsSet dependencies_o_;
    mutable std::optional<Value> cache_;
    PositionsSet dependencies_i_;

    bool IsCyclicFormula(const PositionsSet &dependencies, PositionsSet &inspectors) const {
        if (dependencies.count(pos_) != 0) {
            return true;
        }
        for (auto pos : dependencies) {
            if (!pos.IsValid() || inspectors.count(pos) != 0) {
                continue;
            }
            inspectors.insert(pos);
            Cell *cell = AllocCell(pos);
            if (IsCyclicFormula(cell->dependencies_o_, inspectors)) {
                return true;
            }
        }
        return false;
    }

    bool IsCyclic(const Impl *impl) const {
        auto positions = impl->GetReferencedCells();
        PositionsSet dependencies(positions.begin(), positions.end());
        PositionsSet inspectors;
        return IsCyclicFormula(dependencies, inspectors);
    }


    void UpdateCache() const {
        if (!cache_.has_value()) {
            cache_ = impl_->GetValue();
        }
    }

    void InvalidateCache() {
        cache_ = std::nullopt;
    }

    void DeleteDependencies() {
        for (auto pos : dependencies_o_) {
            if (!pos.IsValid()) {
                continue;
            }
            Cell *cell = AllocCell(pos);
            cell->dependencies_i_.erase(pos);
        }
        dependencies_o_.clear();
    }

    void CreateDependencies() {
        for (auto pos : GetReferencedCells()) {
            if (!pos.IsValid()) {
                continue;
            }
            dependencies_o_.insert(pos);
            Cell *cell = AllocCell(pos);
            cell->dependencies_i_.insert(pos);
        }
    }

    void InvalidAllDependentCaches(const PositionsSet &effects,
                                   PositionsSet &invalids) {
        for (auto pos : effects) {
            if (!pos.IsValid()) {
                continue;
            }
            Cell *cell = AllocCell(pos);
            cell->InvalidateCache();
            invalids.insert(pos);
            cell->InvalidAllDependentCaches(cell->dependencies_i_, invalids);
        }
    }

    void InitializeCell() {
        InvalidateCache();
        DeleteDependencies();
        CreateDependencies();
        PositionsSet invalids;
        InvalidAllDependentCaches(dependencies_i_, invalids);
    }
};