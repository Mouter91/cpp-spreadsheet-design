#pragma once

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
public:
    Cell();
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

private:
    class Impl {
    public:
        virtual ~Impl() = default;
        virtual std::string GetText() const = 0;
        virtual Value GetValue() const = 0;
    };

    class EmptyImpl : public Impl {
    public:
        std::string GetText() const override { 
            return ""; 
        }
        Value GetValue() const override { 
            return ""; 
        } 
    };

    class TextImpl : public Impl {
    public:
        explicit TextImpl(std::string text) : text_(std::move(text)) {}

        std::string GetText() const override { 
            return text_; 
            }
        Value GetValue() const override { 
            if(text_[0] == '\'') {
                return text_.substr(1);
            }
            return text_; 
            } 
    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        explicit FormulaImpl(std::string text, const SheetInterface& sheet, Cell* self)
            : formula_(ParseFormula(text.substr(1))), sheet_(sheet), self_(self) {
            UpdateDependencies();
            CheckCircularDependency(self);
            InvalidateCache();
        }

        std::string GetText() const override { 
            return "=" + formula_->GetExpression(); 
        }

        Value GetValue() const override {
            if (!cache_) {
                cache_ = formula_->Evaluate(sheet_);
            }
            return *cache_;
        }

        std::vector<Cell*> GetDependencies() const {
            return dependencies_;
        }

        void InvalidateCache();

    private:
        void UpdateDependencies();
        void CheckCircularDependency(Cell* self);

        std::unique_ptr<FormulaInterface> formula_;
        std::vector<Cell*> dependencies_;
        mutable std::optional<Value> cache_;
        const SheetInterface& sheet_;
        Cell* self_;
    };

    std::unique_ptr<Impl> impl_;
    const SheetInterface& sheet;
};
