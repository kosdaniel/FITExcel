

#include "CPos.h"

//parse string argument for CPos, throw an exception if it is invalid, store two values representing
//horizontal and vertical coordinates in the spreadsheet
CPos::CPos(std::string_view str) : col_(0), row_(0){
    size_t idx = 0;
    std::string col;
    char c;
    for(; idx < str.size(); ++idx){
        c = str[idx];
        if(c < 'A' || c > 'Z')
            break;
        col.push_back(c);
    }
    if(idx == 0 || c < '0' || c > '9' || idx == str.size() || (c == '0' && idx != str.size() - 1))
        throw std::invalid_argument("");
    for(size_t i = idx + 1; i < str.size(); ++i)
        if(str[i] < '0' || str[i] > '9')
            throw std::invalid_argument("");
    col_ = getInt(col);
    row_ = std::atoi(&(str[idx]));
}

CPos::CPos(int col, int row) : col_(col), row_(row) {}


bool operator <(const CPos& a, const CPos& b) {
    if(a.col() != b.col())
        return a.col() < b.col();
    return a.row() < b.row();
}

bool operator ==(const CPos& a, const CPos& b){
    return a.col() == b.col() && a.row() == b.row();
}


//transform a string representing the vertical coordinate in the spreadsheet into a corresponding integer value
//assume string has the correct format of uppercase letters
int getInt(const std::string& s){
    int n = 0;
    int i = 0;
    for(auto it = s.rbegin(); it != s.rend(); ++it){
        n += (*it - 'A' + 1) * (int)pow(26, i++);
    }
    return n;
}


//transform integer value representing vertical coordinate in the spreadsheet into the corresponding string
std::string getString(int n){
    std::string s;
    while(n > 0){
        char c = 'A' + (n - 1) % 26;
        s.push_back(c);
        n = (n - 1) / 26;
    }
    std::reverse(s.begin(), s.end());
    return s;
}
