#pragma once

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
public:
    Cell();

    ~Cell();

    void Set(std::string text) override;

    void Clear();

    Value GetValue() const override;

    std::string GetText() const override;

private:

    class Impl {
    public:
        virtual ~Impl() = default;

        virtual Value GetValue() const = 0;

        virtual std::string GetText() const = 0;
    };
    std::unique_ptr<Impl> MakeImpl_(std::string text) const;
    class EmptyImpl : public Impl {
    public:
        EmptyImpl();

        Value GetValue() const override;

        std::string GetText() const override;
    };

    class TextImpl : public Impl {
    public:
        TextImpl(std::string str);

        Value GetValue() const override;

        std::string GetText() const override;


    private:
        std::string str_;
    };

    class FormulaImpl : public Impl {
    public:
        FormulaImpl(std::string str);

        Value GetValue() const override;

        std::string GetText() const override;

    private:
        std::unique_ptr<FormulaInterface> formula_;
    };


    std::unique_ptr<Impl> impl_;


};