#include "cell.h"
#include <optional>
#include "sheet.h"
#include <stack>
#include <string>

class Cell::Impl {
public:
    virtual Value GetValue() = 0;
    virtual std::string GetText() = 0;
    virtual std::vector<Position> GetReferencedCells() {
        return {};
    }
    virtual void ClearCache() {
    }

protected:
    Value value_;
    std::string text_;
};

class Cell::EmptyImpl : public Impl {
public:
    EmptyImpl() {
        value_ = "";
        text_ = "";
    }
    Value GetValue() override {
        return value_;
    };
    std::string GetText() override {
        return text_;
    };
};

class Cell::TextImpl : public Impl {
public:
    TextImpl(std::string str) {
        text_ = str;
        if (str[0] == ESCAPE_SIGN) {
            value_ = str.substr(1);
        } else {
            value_ = str;
        }
    }

    Value GetValue() override {
        return value_;
    }
    std::string GetText() override {
        return text_;
    }
};

class Cell::FormulaImpl : public Impl {
public:
    FormulaImpl(std::string& str, SheetInterface& sheet)
        : sheet_(sheet) {
        value_ = str.substr(1);
        formula_ptr_ = ParseFormula(str.substr(1));
        text_ = "=" + formula_ptr_->GetExpression();
    }

    Value GetValue() override {
        std::variant<double, FormulaError> value = formula_ptr_->Evaluate(sheet_);
        if (!cache_) {
            cache_ = value;
        }

        auto result = value;
        if (std::holds_alternative<double>(result)) {
            return std::get<double>(result);
        } else {
            return std::get<FormulaError>(result);
        }
    }

    std::string GetText() override {
        return FORMULA_SIGN + formula_ptr_->GetExpression();
    }

    void ClearCache() {
        cache_.reset();
    }

    std::vector<Position> GetReferencedCells() {
        return formula_ptr_->GetReferencedCells();
    }

private:
    std::unique_ptr<FormulaInterface> formula_ptr_;
    std::optional<FormulaInterface::Value> cache_;
    SheetInterface& sheet_;
};

bool Cell::IsCircular(Impl& impl) {
    std::vector<Position> referenced = impl.GetReferencedCells();
    if (referenced.empty()) {
        return false;
    }

    std::unordered_set<Cell*> referenced_set;
    std::stack<Cell*> to_visit;
    for (auto it : referenced) {
        referenced_set.insert(sheet_.GetConcreteCell(it));
    }

    to_visit.push(this);
    while(!to_visit.empty()) {
        Cell* cell = to_visit.top();
        to_visit.pop();
        if (referenced_set.find(cell) != referenced_set.end()) {
            return true;
        }
        for (auto up_cell : cell->up) {
            to_visit.push(up_cell);
        }
    }
    return false;
}

// Реализуйте следующие методы
Cell::Cell(Sheet& sheet)
    : impl_(std::make_unique<EmptyImpl>()),
      sheet_(sheet) {
}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    if (text.length() == 0) {
        impl_ = std::make_unique<EmptyImpl>();
    } else if (text.length() == 1 || text[0] != FORMULA_SIGN) {
        impl_ = std::make_unique<TextImpl>(text);
    } else {
        auto impl = std::make_unique<FormulaImpl>(text, sheet_);
        if (IsCircular(*impl)) {
            throw CircularDependencyException("");
        }
        impl_ = std::move(impl);
    }


    for (auto down_cells : down) {
        down_cells->up.erase(this);
    }

    down.clear();
    for (auto pos : impl_->GetReferencedCells()) {
        Cell* down_cell = sheet_.GetConcreteCell(pos);
        if (down_cell == nullptr) {
            sheet_.SetCell(pos, "");
            down_cell= sheet_.GetConcreteCell(pos);
        }
        down.insert(down_cell);
        down_cell->up.insert(this);
    }
    ClearCache();
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

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

void Cell::ClearCache() {
    impl_->ClearCache();
    for (auto it : up) {
        it->ClearCache();
    }
}


