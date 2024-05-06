#include <iostream>
#include "sheet.h"

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) throw InvalidPositionException("");
    if (data_.count({pos.row, pos.col}) == 0) {
        data_[{pos.row, pos.col}] = std::make_unique<Cell>(*this);
    }
    IncreaseSize(pos);
    data_[{pos.row, pos.col}]->Set(text);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return GetConcreteCell(pos);
}
CellInterface* Sheet::GetCell(Position pos) {
    return GetConcreteCell(pos);
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) throw InvalidPositionException("");
    if (data_.count({pos.row, pos.col}) != 0) {
        data_.erase({pos.row, pos.col});
    }
    DecreaseSize(pos);
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i) {
        for (int j = 0; j < size_.cols; ++j) {
            if (data_.count({i, j}) > 0) {
                auto value = data_.at({i, j})->GetValue();
                if (std::holds_alternative<std::string>(value)) {
                    output << std::get<std::string>(value);
                }
                if (std::holds_alternative<double>(value)) {
                    output << std::get<double>(value);
                }
                if (std::holds_alternative<FormulaError>(value)) {
                    output << std::get<FormulaError>(value);
                }
            }

            if (j != size_.cols - 1) {
                output << "\t";
            } else {
                output << "\n";
            }
        }
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i) {
        for (int j = 0; j < size_.cols; ++j) {
            if (data_.count({i, j}) > 0) {
                output << data_.at({i, j})->GetText();
            }
            if (j != size_.cols - 1) {
                output << "\t";
            } else {
                output << "\n";
            }
        }
    }
}

void Sheet::IncreaseSize(Position& pos) {
    size_.rows = pos.row + 1 > size_.rows ? pos.row + 1 : size_.rows;
    size_.cols = pos.col + 1 > size_.cols ? pos.col + 1 : size_.cols;
}

void Sheet::DecreaseSize(Position& pos) {
    Size result{0, 0};
    for (auto& it : data_) {
        if (it.first.first + 1 > result.rows) {
            result.rows = it.first.first + 1;
        }
        if (it.first.second + 1 > result.cols) {
            result.cols = it.first.second + 1;
        }
    }
    size_ = result;
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

Cell* Sheet::GetConcreteCell(Position pos) {
    if (!pos.IsValid()) throw InvalidPositionException("");
    if (data_.count({pos.row, pos.col}) == 0) {
        return nullptr;
    }
    return data_.at({pos.row, pos.col}).get();
}

const Cell* Sheet::GetConcreteCell(Position pos) const {
    if (!pos.IsValid()) throw InvalidPositionException("");
    if (data_.count({pos.row, pos.col}) == 0) {
        return nullptr;
    }
    return data_.at({pos.row, pos.col}).get();
}
