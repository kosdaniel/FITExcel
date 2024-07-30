
#include "CSpreadsheet.h"

CSpreadsheet::CSpreadsheet() = default;


//load stored cells from istream until it is empty
//assume only correctly parsed expressions were saved, return false if any error is encountered
//check whether the hash of a string of concatenated loaded expressions with their coordinates equals the saved hash
//if not return false - some cells were not loaded properly
bool CSpreadsheet::load(std::istream &is){
    CSpreadsheet x;
    int col, row;
    std::string to_hash;
    std::string expr;
    std::string line;
    std::istringstream iss;
    while(std::getline(is, line, '|')){
        iss.clear();
        iss.str(line);
        if(!(iss >> col >> row) || !iss.get() || !std::getline(iss, expr, '\0'))
            return false;
        if(col == 0 && row == 0)
            break;
        to_hash += expr + std::to_string(col) + std::to_string(row);
        if(!x.setCell(CPos(col, row), expr))
            return false;
        line.clear();
        expr.clear();
    }
    iss.clear();
    iss.str(expr);
    size_t saved_hash;
    if(!(iss >> saved_hash) || saved_hash != std::hash<std::string>{}(to_hash) || is.bad() || is.get() != EOF
            || col != 0 || row != 0)
        return false;
    for(auto& cell : cells_)
        delete cell.second;
    cells_.clear();
    for(auto& cell : x.cells_)
        cells_[cell.first] = (cell.second ? cell.second->clone() : nullptr);
    return true;
}

bool CSpreadsheet::load(std::ifstream &ifs){
    if(!ifs.is_open() || ifs.bad()){
        return false;
    }
    if(!load((std::istream&)ifs)){
        ifs.close();
        return false;
    }
    ifs.close();
    return true;
}
bool CSpreadsheet::load(const std::string& filename){
    std::ifstream ifs(filename);
    return load(ifs);
}

//save cell coordinates and expression delimited by '|' for every cell in the output stream,
//if cell is empty save '0' as expression
//also save the hash of a string of concatenated saved expressions with their coordinates for file control in load
bool CSpreadsheet::save(std::ostream &os) const{
    std::string to_hash;
    std::string s;
    for(const auto& cell : cells_){
        if(cell.second == nullptr)
            continue;
        os << cell.first.col() << " " << cell.first.row() << " ";
        s = cell.second->reconstruct();
        os << "=" << s << "|";
        to_hash += "=" + s + std::to_string(cell.first.col()) + std::to_string(cell.first.row());
    }
    os << 0 << " " << 0 << " " << std::hash<std::string>{}(to_hash) << "|";
    if(os.bad())
        return false;
    return true;
}
bool CSpreadsheet::save(std::ofstream &ofs) const{
    if(!ofs.is_open() || ofs.bad()) {
        return false;
    }
    if(!save((std::ostream&)ofs)){
        ofs.close();
        return false;
    }
    ofs.close();
    return true;
}
bool CSpreadsheet::save(const std::string& filename) const{
    std::ofstream ofs(filename);
    return save(ofs);
}


//parse expression, build AST from it and store its root in the corresponding cell
//if a value is to be loaded, turn it into an expression and parse it too
bool CSpreadsheet::setCell(CPos pos, std::string contents){
    if(contents.empty()){
        auto it = cells_.find(pos);
        if(it != cells_.end())
            delete it->second;
        cells_[pos] = nullptr;
        return true;
    }
    if(contents[0] == '='){
        try{
            parseExpression(contents, builder_);
        }
        catch(std::exception& e){
            std::cerr << e.what();
            return false;
        }
        auto it = cells_.find(pos);
        if(it != cells_.end())
            delete cells_[pos];
        cells_[pos] = builder_.getAST();
        return true;
    }
    std::string expression = "=";
    for(char c : contents){
        if(c != '\"')
            expression.push_back(c);
        else
            expression += "\"\"";
    }
    try{
        parseExpression(contents, builder_);
    }
    catch(std::exception& e){
        std::cerr << e.what();
        return false;
    }
    auto it = cells_.find(pos);
    if(it != cells_.end())
        delete it->second;
    cells_[pos] = builder_.getAST();
    return true;
}


//if cell is empty or if the expression inside contains a cycle return CValue()
//else evaluate the expression and return the result
CValue CSpreadsheet::getValue(CPos pos){
    auto it = cells_.find(pos);
    if(it == cells_.end() || it->second == nullptr || hasCycle(it->second))
        return CValue();
    return it->second->evaluate(cells_);
}

//first create a clone of the array that is to be copied
//then copy cells from the clone so that the original values to be copied are not overwritten in the process
void CSpreadsheet::copyRect(CPos dst, CPos src, int w, int h){
    if(dst == src) {
        return;
    }
    std::map<CPos, CNode*> to_copy;
    for(int i = 0; i < w; ++i) // i is column index
        for(int j = 0; j < h; ++j){ // j is row index
            CPos from = CPos(src.col() + i, src.row() + j);
            CNode* expr = nullptr;
            auto it = cells_.find(from);
            if(it != cells_.end() && it->second != nullptr){
                expr = it->second->clone();
            }
            to_copy[from] = expr;
        }
    for(int i = 0; i < w; ++i)
        for(int j = 0; j < h; ++j){
            CPos from = CPos(src.col() + i, src.row() + j);
            CPos to = CPos(dst.col() + i, dst.row() + j);
            CNode* expr = nullptr;
            if(to_copy[from] != nullptr) {
                expr = to_copy[from]; //no need to clone again
                expr->shift_references(to.col() - from.col(), to.row() - from.row());
            }
            auto it = cells_.find(to);
            if(it != cells_.end())
                delete it->second;
            cells_[to] = expr;
        }

}

CSpreadsheet::~CSpreadsheet() {
    for(auto& cell : cells_)
        delete cell.second;
}

CSpreadsheet::CSpreadsheet(const CSpreadsheet &other){
    for(auto& cell : other.cells_){
        cells_[cell.first] = (cell.second ? cell.second->clone() : nullptr);
    }
}

CSpreadsheet& CSpreadsheet::operator =(const CSpreadsheet& src){
    if(this == &src)
        return *this;
    for(auto& cell : cells_)
        delete cell.second;
    cells_.clear();
    for(auto& cell : src.cells_){
        cells_[cell.first] = (cell.second ? cell.second->clone() : nullptr);
    }
    return *this;
}

//run DFS on the expression represented by the AST root in argument and check for oriented cycles
//return true if a cycle exists in the expression
bool CSpreadsheet::hasCycle(CNode* expr) const{
    std::set<CNode*> visited, rec_stack;
    return expr->hasCycle(visited, rec_stack, cells_);
}