

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
#include "CASTBuilder.h"

using namespace std::literals;
using CValue = std::variant<std::monostate, double, std::string>;


constexpr unsigned SPREADSHEET_CYCLIC_DEPS = 0x01;
constexpr unsigned SPREADSHEET_FUNCTIONS = 0x02;
constexpr unsigned SPREADSHEET_FILE_IO = 0x04;
constexpr unsigned SPREADSHEET_SPEED = 0x08;
constexpr unsigned SPREADSHEET_PARSER = 0x10;



class CSpreadsheet {
public:
    static unsigned capabilities() {
        return SPREADSHEET_CYCLIC_DEPS | /*SPREADSHEET_FUNCTIONS |*/ SPREADSHEET_FILE_IO | SPREADSHEET_SPEED/* | SPREADSHEET_PARSER*/;
    }

    CSpreadsheet();

    ~CSpreadsheet();

    CSpreadsheet(const CSpreadsheet& other);

    CSpreadsheet& operator =(const CSpreadsheet& src);

    bool load(std::istream &is);
    bool load(std::ifstream &ifs);
    bool load(const std::string& filename);

    bool save(std::ostream &os) const;
    bool save(std::ofstream &ofs) const;
    bool save(const std::string& filename) const;

    bool setCell(CPos pos,
                 std::string contents);

    CValue getValue(CPos pos);

    void copyRect(CPos dst,
                  CPos src,
                  int w = 1,
                  int h = 1);

    bool hasCycle(CNode* expr) const;

    const std::map<CPos, CNode*>& cells() const { return cells_; }

private:
    std::map<CPos, CNode*> cells_;
    CASTBuilder builder_;
};


