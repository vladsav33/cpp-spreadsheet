#pragma once

#include "cell.h"
#include "common.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    using SheetData = std::vector<std::vector<Cell*>>;

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    const Cell* GetConcreteCell(Position pos) const;
    Cell* GetConcreteCell(Position pos);

private:
    struct HashPair {
        size_t operator()(std::pair<int, int> pair_int) const {
            return pair_int.first * 16384 + pair_int.second;
        }
    };

    std::unordered_map<std::pair<int, int>, std::unique_ptr<Cell>, HashPair> data_;
    Size size_;

    void IncreaseSize(Position& pos);

    void DecreaseSize(Position& pos);
};
