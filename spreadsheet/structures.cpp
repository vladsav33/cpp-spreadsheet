#include <cctype>
#include "common.h"
#include <iostream>
#include <string>

using namespace std;

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = {-1, -1};

static pair<string, string> SplitString (std::string_view str) {
    string first = "";
    string second = "";
    for (size_t i = 0; i < str.length(); ++i) {
        char ch = str[i];
        if (isalpha(ch) && isupper(ch)) {
            first += ch;
        } else {
            second = str.substr(i);
            return {first, second};
        }
    }
    return {first, second};
}

// Реализуйте методы:
bool Position::operator==(const Position rhs) const {
    return row == rhs.row && col == rhs.col;
}

bool Position::operator<(const Position rhs) const {
    return std::tie(row, col) < std::tie(rhs.row, rhs.col);
}

bool Position::IsValid() const {
    return row >=0 && row < MAX_ROWS && col >= 0 && col < MAX_COLS;
}

std::string Position::ToString() const {
    if (col < 0 || row < 0 || col > 16383 || row > 16383) {
        return "";
    }

    int result_col = col;
    string result;
    result = string(1, 'A' + result_col % 26) + result;
    result_col /= 26;
    while (result_col > 0) {
        result = string(1, 'A' + (result_col - 1) % 26) + result;
        result_col = (result_col - 1) / 26;
    }
    return result + to_string(row + 1);
}

Position Position::FromString(std::string_view str) {
    if (str.length() == 0) {
        return Position::NONE;
    }
    auto cell_string = SplitString(str);
    if (cell_string.first.length() == 0 || cell_string.first.length() > 3 || cell_string.second.length() == 0
        || cell_string.second.length() > 5) {
        return Position::NONE;
    }

    Position result;
    for (std::size_t i = 0; i < cell_string.second.length(); ++i) {
        if (!isdigit(cell_string.second[i])) {
            return Position::NONE;
        }
    }
    try {
        int y = stoi(cell_string.second);
        result.row = y - 1;
    } catch (std::invalid_argument&) {
        return Position::NONE;
    }
    int col = 0;
    int pow = 1;
    col += (cell_string.first[cell_string.first.length() - 1] - 'A') * pow;
    for (int i = cell_string.first.length() - 2; i >= 0; --i) {
        pow *= 26;
        col += (cell_string.first[i] - 'A' + 1) * pow;
    }
    if (col > 16383) {
        return Position::NONE;
    }
    result.col = col;
    return result;
}

bool Size::operator==(Size rhs) const {
    return rows == rhs.rows && cols == rhs.cols;
}

