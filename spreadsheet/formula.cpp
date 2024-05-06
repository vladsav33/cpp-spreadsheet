#include <algorithm>
#include "formula.h"
#include "FormulaAST.h"

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression)
        try : ast_(ParseFormulaAST(expression)) {
    } catch (const FormulaException& exc) {
        std::throw_with_nested(FormulaException(exc.what()));
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        std::function<double(Position)> args = [&sheet](Position pos) -> double {
            if (!pos.IsValid()) {
                throw FormulaError(FormulaError::Category::Ref);
            }

            const CellInterface* cell = sheet.GetCell(pos);
            if (!cell) {
                return 0;
            }

            std::variant<std::string, double, FormulaError> value = cell->GetValue();
            if (std::holds_alternative<double>(value)) {
                return std::get<double>(value);
            }

            if (std::holds_alternative<std::string>(value)) {
                double result = 0;
                std::string str = std::get<std::string>(value);

                if (!str.empty()) {
                    size_t len;

                    try {
                        result = std::stod(str, &len);
                    } catch (...) {
                        throw FormulaError(FormulaError::Category::Value);
                    }
                    if (len != str.length()) {
                        throw FormulaError(FormulaError::Category::Value);
                    }
                }
                return result;
            }
            throw FormulaError(std::get<FormulaError>(value));
        };

        try {
            return ast_.Execute(args);
        } catch (FormulaError& error) {
            return error;
        }
    }

    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        std::vector<Position> cells;
        for (Position cell : ast_.GetCells()) {
            if (cell.IsValid()) {
                cells.push_back(cell);
            }
        }
        cells.resize(std::unique(cells.begin(), cells.end()) - cells.begin());
        return cells;
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    }
    catch (...) {
        throw FormulaException("");
    }
}

FormulaError::FormulaError(Category category)
    : category_(category)
{
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
        case FormulaError::Category::Ref:
            return "#REF!";
        case FormulaError::Category::Value:
            return "#VALUE!";
        case FormulaError::Category::Arithmetic:
            return "#ARITHM!";
    }
    return "";
}
