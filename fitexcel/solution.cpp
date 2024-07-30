#ifndef __PROGTEST__

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
//#include "expression.h"
#include "CSpreadsheet.h"

using namespace std::literals;
using CValue = std::variant<std::monostate, double, std::string>;

/*constexpr unsigned SPREADSHEET_CYCLIC_DEPS = 0x01;
constexpr unsigned SPREADSHEET_FUNCTIONS = 0x02;
constexpr unsigned SPREADSHEET_FILE_IO = 0x04;
constexpr unsigned SPREADSHEET_SPEED = 0x08;
constexpr unsigned SPREADSHEET_PARSER = 0x10;*/
#endif /* __PROGTEST__ */





#ifndef __PROGTEST__

bool valueMatch(const CValue &r,
                const CValue &s) {
    if (r.index() != s.index())
        return false;
    if (r.index() == 0)
        return true;
    if (r.index() == 2)
        return std::get<std::string>(r) == std::get<std::string>(s);
    if (std::isnan(std::get<double>(r)) && std::isnan(std::get<double>(s)))
        return true;
    if (std::isinf(std::get<double>(r)) && std::isinf(std::get<double>(s)))
        return (std::get<double>(r) < 0 && std::get<double>(s) < 0)
               || (std::get<double>(r) > 0 && std::get<double>(s) > 0);
    bool res = fabs(std::get<double>(r) - std::get<double>(s)) <= 1e8 * DBL_EPSILON * fabs(std::get<double>(r));
    if(!res){
        std::cout << std::get<double>(r) << " != " << std::get<double>(s) << std::endl;
    }
    return res;

    //return fabs(std::get<double>(r) - std::get<double>(s)) <= 1e8 * DBL_EPSILON * fabs(std::get<double>(r));
}

int main() {
    CSpreadsheet x0, x1;
    std::ostringstream oss;
    std::istringstream iss;
    std::string data;



    assert(x0.save("savefile.txt"));
    assert(x0.load("savefile.txt"));
    x1 = x0;

    oss.clear();
    oss.str("");
    assert (x1.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert (x1.load(iss));



    assert (x0.setCell(CPos("A1"), "10"));
    assert (x0.setCell(CPos("A2"), "20.5"));
    assert (x0.setCell(CPos("A3"), "3e1"));
    assert (x0.setCell(CPos("A4"), "=40"));
    assert (x0.setCell(CPos("A5"), "=5e+1"));
    assert (x0.setCell(CPos("A6"), "raw text with any characters, including a quote \" or a newline\n"));
    assert (x0.setCell(CPos("A7"),
                       "=\"quoted string, quotes must be doubled: \"\". Moreover, backslashes are needed for C++.\""));

    assert(x0.save("savefile.txt"));
    assert(x0.load("savefile.txt"));
    x1 = x0;

    oss.clear();
    oss.str("");
    assert (x1.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert (x1.load(iss));
    for(auto& cell : x0.cells()){
        assert (valueMatch(x0.getValue(cell.first), x1.getValue(cell.first)));
    }


    assert (valueMatch(x0.getValue(CPos("A1")), CValue(10.0)));
    assert (valueMatch(x0.getValue(CPos("A2")), CValue(20.5)));
    assert (valueMatch(x0.getValue(CPos("A3")), CValue(30.0)));
    assert (valueMatch(x0.getValue(CPos("A4")), CValue(40.0)));
    assert (valueMatch(x0.getValue(CPos("A5")), CValue(50.0)));
    assert (valueMatch(x0.getValue(CPos("A6")),
                       CValue("raw text with any characters, including a quote \" or a newline\n")));
    assert (valueMatch(x0.getValue(CPos("A7")),
                       CValue("quoted string, quotes must be doubled: \". Moreover, backslashes are needed for C++.")));
    assert (valueMatch(x0.getValue(CPos("A8")), CValue()));
    assert (valueMatch(x0.getValue(CPos("AAAA9999")), CValue()));
    assert (x0.setCell(CPos("B1"), "=A1+A2*A3"));
    assert (x0.setCell(CPos("B2"), "= -A1 ^ 2 - A2 / 2   "));
    assert (x0.setCell(CPos("B3"), "= 2 ^ $A$1"));
    assert (x0.setCell(CPos("B4"), "=($A1+A$2)^2"));

    assert (x0.setCell(CPos("B5"), "=B1+B2+B3+B4"));
    assert (x0.setCell(CPos("B6"), "=B1+B2+B3+B4+B5"));
    assert (valueMatch(x0.getValue(CPos("B1")), CValue(625.0)));
    assert (valueMatch(x0.getValue(CPos("B2")), CValue(-110.25)));
    assert (valueMatch(x0.getValue(CPos("B3")), CValue(1024.0)));
    assert (valueMatch(x0.getValue(CPos("B4")), CValue(930.25)));
    assert (valueMatch(x0.getValue(CPos("B5")), CValue(2469.0)));
    assert (valueMatch(x0.getValue(CPos("B6")), CValue(4938.0)));
    assert (x0.setCell(CPos("A1"), "12"));
    assert (valueMatch(x0.getValue(CPos("B1")), CValue(627.0)));
    assert (valueMatch(x0.getValue(CPos("B2")), CValue(-154.25)));
    assert (valueMatch(x0.getValue(CPos("B3")), CValue(4096.0)));
    assert (valueMatch(x0.getValue(CPos("B4")), CValue(1056.25)));
    assert (valueMatch(x0.getValue(CPos("B5")), CValue(5625.0)));
    assert (valueMatch(x0.getValue(CPos("B6")), CValue(11250.0)));
    x1 = x0;
    assert (x0.setCell(CPos("A2"), "100"));
    assert (x1.setCell(CPos("A2"), "=A3+A5+A4"));
    assert (valueMatch(x0.getValue(CPos("B1")), CValue(3012.0)));
    assert (valueMatch(x0.getValue(CPos("B2")), CValue(-194.0)));
    assert (valueMatch(x0.getValue(CPos("B3")), CValue(4096.0)));
    assert (valueMatch(x0.getValue(CPos("B4")), CValue(12544.0)));
    assert (valueMatch(x0.getValue(CPos("B5")), CValue(19458.0)));
    assert (valueMatch(x0.getValue(CPos("B6")), CValue(38916.0)));
    assert (valueMatch(x1.getValue(CPos("B1")), CValue(3612.0)));
    assert (valueMatch(x1.getValue(CPos("B2")), CValue(-204.0)));
    assert (valueMatch(x1.getValue(CPos("B3")), CValue(4096.0)));
    assert (valueMatch(x1.getValue(CPos("B4")), CValue(17424.0)));
    assert (valueMatch(x1.getValue(CPos("B5")), CValue(24928.0)));
    assert (valueMatch(x1.getValue(CPos("B6")), CValue(49856.0)));

    assert(x0.save("savefile.txt"));
    assert(x0.load("savefile.txt"));
    x1 = x0;

    oss.clear();
    oss.str("");
    assert (x1.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert (x1.load(iss));
    for(auto& cell : x0.cells()){
        assert (valueMatch(x0.getValue(cell.first), x1.getValue(cell.first)));
    }


    oss.clear();
    oss.str("");
    assert (x0.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert (x1.load(iss));
    assert (valueMatch(x1.getValue(CPos("B1")), CValue(3012.0)));
    assert (valueMatch(x1.getValue(CPos("B2")), CValue(-194.0)));
    assert (valueMatch(x1.getValue(CPos("B3")), CValue(4096.0)));
    assert (valueMatch(x1.getValue(CPos("B4")), CValue(12544.0)));
    assert (valueMatch(x1.getValue(CPos("B5")), CValue(19458.0)));
    assert (valueMatch(x1.getValue(CPos("B6")), CValue(38916.0)));
    assert (x0.setCell(CPos("A3"), "4e1"));
    assert (valueMatch(x1.getValue(CPos("B1")), CValue(3012.0)));
    assert (valueMatch(x1.getValue(CPos("B2")), CValue(-194.0)));
    assert (valueMatch(x1.getValue(CPos("B3")), CValue(4096.0)));
    assert (valueMatch(x1.getValue(CPos("B4")), CValue(12544.0)));
    assert (valueMatch(x1.getValue(CPos("B5")), CValue(19458.0)));
    assert (valueMatch(x1.getValue(CPos("B6")), CValue(38916.0)));
    oss.clear();
    oss.str("");
    assert (x0.save(oss));
    data = oss.str();
    for (size_t i = 0; i < std::min<size_t>(data.length(), 10); i++)
        data[i] ^= 0x5a;
    iss.clear();
    iss.str(data);
    assert (!x1.load(iss));
    assert (x0.setCell(CPos("D0"), "10"));
    assert (x0.setCell(CPos("D1"), "20"));
    assert (x0.setCell(CPos("D2"), "30"));
    assert (x0.setCell(CPos("D3"), "40"));
    assert (x0.setCell(CPos("D4"), "50"));
    assert (x0.setCell(CPos("E0"), "60"));
    assert (x0.setCell(CPos("E1"), "70"));
    assert (x0.setCell(CPos("E2"), "80"));
    assert (x0.setCell(CPos("E3"), "90"));
    assert (x0.setCell(CPos("E4"), "100"));
    assert (x0.setCell(CPos("F10"), "=D0+5"));
    assert (x0.setCell(CPos("F11"), "=$D0+5"));
    assert (x0.setCell(CPos("F12"), "=D$0+5"));
    assert (x0.setCell(CPos("F13"), "=$D$0+5"));

    assert(x0.save("savefile.txt"));
    assert(x0.load("savefile.txt"));
    x1 = x0;

    oss.clear();
    oss.str("");
    assert (x1.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert (x1.load(iss));
    for(auto& cell : x0.cells()){
        assert (valueMatch(x0.getValue(cell.first), x1.getValue(cell.first)));
    }

    x0.copyRect(CPos("G11"), CPos("F10"), 1, 4);
    assert (valueMatch(x0.getValue(CPos("F10")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F11")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F12")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F13")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F14")), CValue()));
    assert (valueMatch(x0.getValue(CPos("G10")), CValue()));
    assert (valueMatch(x0.getValue(CPos("G11")), CValue(75.0)));
    assert (valueMatch(x0.getValue(CPos("G12")), CValue(25.0)));
    assert (valueMatch(x0.getValue(CPos("G13")), CValue(65.0)));
    assert (valueMatch(x0.getValue(CPos("G14")), CValue(15.0)));
    x0.copyRect(CPos("G11"), CPos("F10"), 2, 4);
    assert (valueMatch(x0.getValue(CPos("F10")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F11")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F12")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F13")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("F14")), CValue()));
    assert (valueMatch(x0.getValue(CPos("G10")), CValue()));
    assert (valueMatch(x0.getValue(CPos("G11")), CValue(75.0)));
    assert (valueMatch(x0.getValue(CPos("G12")), CValue(25.0)));
    assert (valueMatch(x0.getValue(CPos("G13")), CValue(65.0)));
    assert (valueMatch(x0.getValue(CPos("G14")), CValue(15.0)));
    assert (valueMatch(x0.getValue(CPos("H10")), CValue()));
    assert (valueMatch(x0.getValue(CPos("H11")), CValue()));
    assert (valueMatch(x0.getValue(CPos("H12")), CValue()));
    assert (valueMatch(x0.getValue(CPos("H13")), CValue(35.0)));
    assert (valueMatch(x0.getValue(CPos("H14")), CValue()));
    assert (x0.setCell(CPos("F0"), "-27"));
    assert (valueMatch(x0.getValue(CPos("H14")), CValue(-22.0)));
    x0.copyRect(CPos("H12"), CPos("H13"), 1, 2);
    assert (valueMatch(x0.getValue(CPos("H12")), CValue(25.0)));
    assert (valueMatch(x0.getValue(CPos("H13")), CValue(-22.0)));
    assert (valueMatch(x0.getValue(CPos("H14")), CValue(-22.0)));



    assert (x0.setCell(CPos("Z0"), "=Z1+Z2"));
    assert (x0.setCell(CPos("Z1"), "=Z3"));
    assert (x0.setCell(CPos("Z2"), "=Z3"));
    assert (x0.setCell(CPos("Z3"), "1024"));
    assert (valueMatch(x0.getValue(CPos("Z0")), CValue(2048.0)));
    assert(x0.setCell(CPos("Z4"), "=Z4+3"));
    assert(valueMatch(x0.getValue(CPos("Z4")), CValue()));
    assert(x0.setCell(CPos("Z5"), "=Z6+20"));
    assert(x0.setCell(CPos("Z6"), "=Z5-6"));
    assert(valueMatch(x0.getValue(CPos("Z5")), CValue()));
    assert(valueMatch(x0.getValue(CPos("Z6")), CValue()));

    for(int i = 0; i < 40; ++i){
        oss.clear();
        oss.str("");
        assert (x0.save(oss));
        data = oss.str();
        iss.clear();
        iss.str(data);
        x0 = CSpreadsheet();
        assert (x0.load(iss));
        assert (valueMatch(x0.getValue(CPos("Z0")), CValue(2048.0)));
        assert (valueMatch(x0.getValue(CPos("F10")), CValue(15.0)));
        assert (valueMatch(x0.getValue(CPos("F11")), CValue(15.0)));
        assert (valueMatch(x0.getValue(CPos("F12")), CValue(15.0)));
        assert (valueMatch(x0.getValue(CPos("F13")), CValue(15.0)));
        assert (valueMatch(x0.getValue(CPos("F14")), CValue()));
        assert (valueMatch(x0.getValue(CPos("G10")), CValue()));
        assert (valueMatch(x0.getValue(CPos("G11")), CValue(75.0)));
        assert (valueMatch(x0.getValue(CPos("G12")), CValue(25.0)));
        assert (valueMatch(x0.getValue(CPos("G13")), CValue(65.0)));
    }

    assert(x0.save("savefile.txt"));
    assert(x0.load("savefile.txt"));
    x1 = x0;

    oss.clear();
    oss.str("");
    assert (x1.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert (x1.load(iss));
    for(auto& cell : x0.cells()){
        assert (valueMatch(x0.getValue(cell.first), x1.getValue(cell.first)));
    }

    x0 = CSpreadsheet();

    assert(x0.save("savefile.txt"));
    assert(x0.load("savefile.txt"));
    x1 = x0;

    oss.clear();
    oss.str("");
    assert (x1.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert (x1.load(iss));
    for(auto& cell : x0.cells()){
        assert (valueMatch(x0.getValue(cell.first), x1.getValue(cell.first)));
    }

    assert (x0.setCell(CPos("A1"), "10"));
    assert (x0.setCell(CPos("A2"), "20.5"));
    assert (x0.setCell(CPos("A3"), "3e1"));
    assert (x0.setCell(CPos("A4"), "=40"));
    assert (x0.setCell(CPos("A5"), "=5e+1"));
    assert (x0.setCell(CPos("A6"), "raw text with any characters, including a quote \" or a newline\n"));
    assert (x0.setCell(CPos("A7"),
                       "=\"quoted string, quotes must be doubled: \"\". Moreover, backslashes are needed for C++.\""));

    assert(x0.save("savefile.txt"));
    assert(x0.load("savefile.txt"));
    x1 = x0;

    oss.clear();
    oss.str("");
    assert (x1.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert (x1.load(iss));
    for(auto& cell : x0.cells()){
        assert (valueMatch(x0.getValue(cell.first), x1.getValue(cell.first)));
    }

    assert (valueMatch(x0.getValue(CPos("A1")), CValue(10.0)));
    assert (valueMatch(x0.getValue(CPos("A2")), CValue(20.5)));
    assert (valueMatch(x0.getValue(CPos("A3")), CValue(30.0)));
    assert (valueMatch(x0.getValue(CPos("A4")), CValue(40.0)));
    assert (valueMatch(x0.getValue(CPos("A5")), CValue(50.0)));
    assert (valueMatch(x0.getValue(CPos("A6")),
                       CValue("raw text with any characters, including a quote \" or a newline\n")));
    assert (valueMatch(x0.getValue(CPos("A7")),
                       CValue("quoted string, quotes must be doubled: \". Moreover, backslashes are needed for C++.")));
    assert (valueMatch(x0.getValue(CPos("A8")), CValue()));
    assert (valueMatch(x0.getValue(CPos("AAAA9999")), CValue()));
    assert (x0.setCell(CPos("B1"), "=$A$1+$A$2*$A$3"));
    assert (x0.setCell(CPos("B2"), "= -$A$1 ^ 2 - $A$2 / 2   "));
    assert (x0.setCell(CPos("B3"), "= 2 ^ $A$1"));
    assert (x0.setCell(CPos("B4"), "=($A$1+$A$2)^2"));
    assert (x0.setCell(CPos("B5"), "=$B$1+$B$2+$B$3+$B$4"));
    assert (x0.setCell(CPos("B6"), "=$B$1+$B$2+$B$3+$B$4+$B$5"));
    assert (valueMatch(x0.getValue(CPos("B1")), CValue(625.0)));
    assert (valueMatch(x0.getValue(CPos("B2")), CValue(-110.25)));
    assert (valueMatch(x0.getValue(CPos("B3")), CValue(1024.0)));
    assert (valueMatch(x0.getValue(CPos("B4")), CValue(930.25)));
    assert (valueMatch(x0.getValue(CPos("B5")), CValue(2469.0)));
    assert (valueMatch(x0.getValue(CPos("B6")), CValue(4938.0)));
    assert (x0.setCell(CPos("A1"), "12"));
    assert (valueMatch(x0.getValue(CPos("B1")), CValue(627.0)));
    assert (valueMatch(x0.getValue(CPos("B2")), CValue(-154.25)));
    assert (valueMatch(x0.getValue(CPos("B3")), CValue(4096.0)));
    assert (valueMatch(x0.getValue(CPos("B4")), CValue(1056.25)));
    assert (valueMatch(x0.getValue(CPos("B5")), CValue(5625.0)));
    assert (valueMatch(x0.getValue(CPos("B6")), CValue(11250.0)));

    assert(x0.save("savefile.txt"));
    assert(x0.load("savefile.txt"));
    x1 = x0;

    oss.clear();
    oss.str("");
    assert (x1.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert (x1.load(iss));
    for(auto& cell : x0.cells()){
        assert (valueMatch(x0.getValue(cell.first), x1.getValue(cell.first)));
    }

    x0.copyRect(CPos("AA1"), CPos("A1"), 2, 10);
    assert (valueMatch(x0.getValue(CPos("A1")), x0.getValue(CPos("AA1"))));
    assert (valueMatch(x0.getValue(CPos("A2")), x0.getValue(CPos("AA2"))));
    assert (valueMatch(x0.getValue(CPos("A3")), x0.getValue(CPos("AA3"))));
    assert (valueMatch(x0.getValue(CPos("A4")), x0.getValue(CPos("AA4"))));
    assert (valueMatch(x0.getValue(CPos("A5")), x0.getValue(CPos("AA5"))));
    assert (valueMatch(x0.getValue(CPos("A6")), x0.getValue(CPos("AA6"))));
    assert (valueMatch(x0.getValue(CPos("A7")), x0.getValue(CPos("AA7"))));
    //CPos pos = CPos("AB1");
   // std::cout << "pos AB1: col:" << pos.col() << " row: " << pos.row() << std::endl;
    assert (valueMatch(x0.getValue(CPos("B1")), x0.getValue(CPos("AB1"))));
    assert (valueMatch(x0.getValue(CPos("B2")), x0.getValue(CPos("AB2"))));
    assert (valueMatch(x0.getValue(CPos("B3")), x0.getValue(CPos("AB3"))));
    assert (valueMatch(x0.getValue(CPos("B4")), x0.getValue(CPos("AB4"))));
    assert (valueMatch(x0.getValue(CPos("B5")), x0.getValue(CPos("AB5"))));
    assert (valueMatch(x0.getValue(CPos("B6")), x0.getValue(CPos("AB6"))));
    assert (valueMatch(x0.getValue(CPos("B7")), x0.getValue(CPos("AB7"))));
    x0.copyRect(CPos("X1"), CPos("A1"), 2, 10);
    assert (valueMatch(x0.getValue(CPos("A1")), x0.getValue(CPos("X1"))));
    assert (valueMatch(x0.getValue(CPos("A2")), x0.getValue(CPos("X2"))));
    assert (valueMatch(x0.getValue(CPos("A3")), x0.getValue(CPos("X3"))));
    assert (valueMatch(x0.getValue(CPos("A4")), x0.getValue(CPos("X4"))));
    assert (valueMatch(x0.getValue(CPos("A5")), x0.getValue(CPos("X5"))));
    assert (valueMatch(x0.getValue(CPos("A6")), x0.getValue(CPos("X6"))));
    assert (valueMatch(x0.getValue(CPos("A7")), x0.getValue(CPos("X7"))));
    assert (valueMatch(x0.getValue(CPos("B1")), x0.getValue(CPos("Y1"))));
    assert (valueMatch(x0.getValue(CPos("B2")), x0.getValue(CPos("Y2"))));
    assert (valueMatch(x0.getValue(CPos("B3")), x0.getValue(CPos("Y3"))));
    assert (valueMatch(x0.getValue(CPos("B4")), x0.getValue(CPos("Y4"))));
    assert (valueMatch(x0.getValue(CPos("B5")), x0.getValue(CPos("Y5"))));
    assert (valueMatch(x0.getValue(CPos("B6")), x0.getValue(CPos("Y6"))));
    assert (valueMatch(x0.getValue(CPos("B7")), x0.getValue(CPos("Y7"))));


    assert (x0.setCell(CPos("B1"), "=A1+A2*A3"));
    assert (x0.setCell(CPos("B2"), "= -A1 ^ 2 - A2 / 2   "));
    assert (x0.setCell(CPos("B3"), "= 2 ^ A1"));
    assert (x0.setCell(CPos("B4"), "=(A1+A2)^2"));
    assert (x0.setCell(CPos("B5"), "=B1+B2+B3+B4"));
    assert (x0.setCell(CPos("B6"), "=B1+B2+B3+B4+B5"));

    x0.copyRect(CPos("O1"), CPos("A1"), 2, 10);

    assert (valueMatch(x0.getValue(CPos("P1")), x0.getValue(CPos("B1"))));
    assert (valueMatch(x0.getValue(CPos("P2")), x0.getValue(CPos("B2"))));
    assert (valueMatch(x0.getValue(CPos("P3")), x0.getValue(CPos("B3"))));
    assert (valueMatch(x0.getValue(CPos("P4")), x0.getValue(CPos("B4"))));
    assert (valueMatch(x0.getValue(CPos("P5")), x0.getValue(CPos("B5"))));
    assert (valueMatch(x0.getValue(CPos("P6")), x0.getValue(CPos("B6"))));

    x0.copyRect(CPos("Q5"), CPos("A1"), 2, 10);

    assert (valueMatch(x0.getValue(CPos("R5")), x0.getValue(CPos("B1"))));
    assert (valueMatch(x0.getValue(CPos("R6")), x0.getValue(CPos("B2"))));
    assert (valueMatch(x0.getValue(CPos("R7")), x0.getValue(CPos("B3"))));
    assert (valueMatch(x0.getValue(CPos("R8")), x0.getValue(CPos("B4"))));
    assert (valueMatch(x0.getValue(CPos("R9")), x0.getValue(CPos("B5"))));
    assert (valueMatch(x0.getValue(CPos("R10")), x0.getValue(CPos("B6"))));

    assert (x0.setCell(CPos("B1"), "=A$1+A$2*A$3"));

    x0.copyRect(CPos("Y1"), CPos("A1"), 1, 3);

    x0.copyRect(CPos("Z8"), CPos("B1"));

    assert(x0.save("savefile.txt"));
    assert(x0.load("savefile.txt"));
    x1 = x0;

    oss.clear();
    oss.str("");
    assert (x1.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert (x1.load(iss));
    for(auto& cell : x0.cells()){
        assert (valueMatch(x0.getValue(cell.first), x1.getValue(cell.first)));
    }

    //std::cout << x0.cells()[CPos("Z8")]->reconstruct() << std::endl;


    assert (valueMatch(x0.getValue(CPos("Z8")), x0.getValue(CPos("B1"))));


    assert (x0.setCell(CPos("A6"), "raw text with any characters, including a quote \" or a newline\n"));
    assert (x0.setCell(CPos("A7"),
                       "=\"quoted string, quotes must be doubled: \"\". Moreover, backslashes are needed for C++.\""));

    x0.copyRect(CPos("X20"), CPos("A6"), 1, 2);

    assert (valueMatch(x0.getValue(CPos("X20")),
                       CValue("raw text with any characters, including a quote \" or a newline\n")));
    assert (valueMatch(x0.getValue(CPos("X21")),
                       CValue("quoted string, quotes must be doubled: \". Moreover, backslashes are needed for C++.")));



    assert(getString(26) == "Z");
    assert(getString(27) == "AA");
    assert(getString(28) == "AB");
    assert(getInt("Z") == 26);
    assert(getInt("AA") == 27);
    assert(getInt("AB") == 28);



    CPos p("Z1");
    assert(CPos("AA1") == CPos(p.col() + 1, p.row()));
    assert(CPos("AB1") == CPos(p.col() + 2, p.row()));



    assert (x0.setCell(CPos("V5"), "1"));
    assert (x0.setCell(CPos("W5"), "2"));
    assert (x0.setCell(CPos("V6"), "3"));
    assert (x0.setCell(CPos("W6"), "4"));

    x0.copyRect(CPos("W6"), CPos("V5"), 2, 2);
    assert (valueMatch(x0.getValue(CPos("V5")), CValue(1.0)));
    assert (valueMatch(x0.getValue(CPos("W5")), CValue(2.0)));
    assert (valueMatch(x0.getValue(CPos("V6")), CValue(3.0)));
    assert (valueMatch(x0.getValue(CPos("W6")), CValue(1.0)));
    assert (valueMatch(x0.getValue(CPos("X6")), CValue(2.0)));
    assert (valueMatch(x0.getValue(CPos("W7")), CValue(3.0)));
    assert (valueMatch(x0.getValue(CPos("X7")), CValue(4.0)));


    assert(x0.save("savefile.txt"));
    assert(x0.load("savefile.txt"));
    x1 = x0;

    oss.clear();
    oss.str("");
    assert (x1.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert (x1.load(iss));
    for(auto& cell : x0.cells()){
        assert (valueMatch(x0.getValue(cell.first), x1.getValue(cell.first)));
    }

    assert (x0.setCell(CPos("VX5"), "1"));
    assert (x0.setCell(CPos("WZ5"), "2"));
    assert (x0.setCell(CPos("VY6"), "3"));
    assert (x0.setCell(CPos("WZ6"), "4"));
    assert (x0.setCell(CPos("VX55"), "1"));
    assert (x0.setCell(CPos("WZ577"), "2"));
    assert (x0.setCell(CPos("VY61"), "3"));
    assert (x0.setCell(CPos("WZ630"), "4"));


    assert (x0.setCell(CPos("A1"), "1.111111111111111119999999"));
    assert (x0.setCell(CPos("A2"), "20.116116116116116116116116116116116116116"));
    assert (x0.setCell(CPos("A3"), "3e60"));
    assert (x0.setCell(CPos("A4"), "=1.00000000000000000000513644e20"));
    assert (x0.setCell(CPos("A5"), "=5e+1 * 1.000123666"));


    assert (x0.setCell(CPos("BA1"), "=A1+A2*A3"));
    assert (x0.setCell(CPos("AAAB2"), "= -A1 ^ 2 - A2 / 2   "));
    assert (x0.setCell(CPos("BCV3"), "= 2 ^ A1"));
    assert (x0.setCell(CPos("BH4"), "=(A1+A2)^2"));
    assert (x0.setCell(CPos("BIT5"), "=B1+B2+B3+B4"));
    assert (x0.setCell(CPos("BZZ6"), "=B1+B2+B3+B4+B5"));

    x1 = CSpreadsheet();

    assert(x0.save("savefile.txt"));
    assert(x1.load("savefile.txt"));

    for(auto& cell : x0.cells()){
        assert (valueMatch(x0.getValue(cell.first), x1.getValue(cell.first)));
    }

    x0 = CSpreadsheet();

    assert(x0.load("savefile.txt"));
    assert(x1.save("savefile.txt"));
    x1 = CSpreadsheet();
    assert(x1.load("savefile.txt"));

    for(auto& cell : x0.cells()){
        assert (valueMatch(x0.getValue(cell.first), x1.getValue(cell.first)));
    }

    x0 = CSpreadsheet();
    assert(x0.save("savefile.txt"));
    assert(x0.load("savefile.txt"));
    assert(x0.cells().empty());



    return EXIT_SUCCESS;
}

#endif /* __PROGTEST__ */
