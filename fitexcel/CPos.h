
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <climits>
#include <cfloat>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <array>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <variant>
#include <optional>
#include <compare>
#include <charconv>
#include <span>
#include <utility>
#include "expression.h"


//class representing the coordinates of a cell in the spreadsheet
class CPos {
public:
    CPos(std::string_view str);
    int col() const{
        return col_;
    }
    int row() const{
        return row_;
    }
    CPos(int col, int row);
private:
    int col_;
    int row_;
};

bool operator <(const CPos& a, const CPos& b);
bool operator ==(const CPos& a, const CPos& b);

int getInt(const std::string& s);
std::string getString(int n);
