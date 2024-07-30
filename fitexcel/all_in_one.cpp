


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




//abstract class representing a node in the AST
struct CNode {
    //recursively traverse the AST and return the value of the expression it represents
    //this function is only called if the AST contains no cycles
    virtual CValue evaluate([[maybe_unused]]std::map<CPos, CNode*>& cells) const = 0;

    virtual CNode* clone() const = 0;

    virtual ~CNode() = default;

    //if the node this is called on represents a cell reference, shift its coordinates that are not absolute references
    virtual void shift_references([[maybe_unused]]int w, [[maybe_unused]]int h){}

    //recursively traverse the AST and reconstruct the original string containing the expression represented by the AST
    virtual std::string reconstruct() const = 0;

    //DFS helper function for detecting cycles
    //if a back edge is detected, the AST contains a cycle, return true
    virtual bool hasCycle([[maybe_unused]]std::set<CNode*>& visited, [[maybe_unused]]std::set<CNode*> rec_stack, [[maybe_unused]]const std::map<CPos, CNode*>& cells) const{ return false; }
};

struct BinaryOpNode : public CNode{
    BinaryOpNode(CNode* left, CNode* right) : left_(left), right_(right) {}
    BinaryOpNode(const BinaryOpNode& src){
        left_ = src.left_->clone();
        right_ = src.right_->clone();
    }
    ~BinaryOpNode() override{
        delete left_;
        delete right_;
    }
    BinaryOpNode& operator =(const BinaryOpNode& other){
        if(this == &other)
            return *this;
        delete left_;
        delete right_;
        left_ = other.left_->clone();
        right_ = other.right_->clone();
        return *this;
    }
    virtual void shift_references(int w, int h) override{
        left_->shift_references(w, h);
        right_->shift_references(w, h);
    }
    virtual bool hasCycle(std::set<CNode*>& visited, std::set<CNode*> rec_stack, const std::map<CPos, CNode*>& cells) const override{
        if(!rec_stack.insert((CNode*)this).second)
            return true;
        if(!visited.insert((CNode*)this).second)
            return false;
        return left_->hasCycle(visited, rec_stack, cells) || right_->hasCycle(visited, rec_stack, cells);
    }

    CNode* left_;
    CNode* right_;
};

struct AddNode : public BinaryOpNode{
    using BinaryOpNode::BinaryOpNode;
    CValue evaluate(std::map<CPos, CNode*>& cells) const override{
        CValue left = left_->evaluate(cells);
        CValue right = right_->evaluate(cells);
        if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
            return std::get<double>(left) + std::get<double>(right);
        if(std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)){
            return std::get<std::string>(left) + std::get<std::string>(right);
        }
        return CValue();
    }
    CNode* clone() const override{
        return new AddNode(*this);
    }
    std::string reconstruct() const override{
        return "(" + left_->reconstruct() + "+" + right_->reconstruct() + ")";
    }
};

struct SubNode : public BinaryOpNode{
    using BinaryOpNode::BinaryOpNode;
    CValue evaluate(std::map<CPos, CNode*>& cells) const override{
        CValue left = left_->evaluate(cells);
        CValue right = right_->evaluate(cells);
        if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
            return std::get<double>(left) - std::get<double>(right);
        return CValue();
    }
    CNode* clone() const override{
        return new SubNode(*this);
    }
    std::string reconstruct() const override{
        return "(" + left_->reconstruct() + "-" + right_->reconstruct() + ")";
    }
};

struct MulNode : public BinaryOpNode{
    using BinaryOpNode::BinaryOpNode;
    CValue evaluate(std::map<CPos, CNode*>& cells) const override{
        CValue left = left_->evaluate(cells);
        CValue right = right_->evaluate(cells);
        if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
            return std::get<double>(left) * std::get<double>(right);
        return CValue();
    }
    CNode* clone() const override{
        return new MulNode(*this);
    }
    std::string reconstruct() const override{
        return "(" + left_->reconstruct() + "*" + right_->reconstruct() + ")";
    }
};

struct DivNode : public BinaryOpNode{
    using BinaryOpNode::BinaryOpNode;
    CValue evaluate(std::map<CPos, CNode*>& cells) const override{
        CValue left = left_->evaluate(cells);
        CValue right = right_->evaluate(cells);
        if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right) && std::get<double>(right) != 0)
            return std::get<double>(left) / std::get<double>(right);
        return CValue();
    }
    CNode* clone() const override{
        return new DivNode(*this);
    }
    std::string reconstruct() const override{
        return "(" + left_->reconstruct() + "/" + right_->reconstruct() + ")";
    }
};

struct PowNode : public BinaryOpNode{
    using BinaryOpNode::BinaryOpNode;
    CValue evaluate(std::map<CPos, CNode*>& cells) const override{
        CValue left = left_->evaluate(cells);
        CValue right = right_->evaluate(cells);
        if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
            return pow(std::get<double>(left), std::get<double>(right));
        return CValue();
    }
    CNode* clone() const override{
        return new PowNode(*this);
    }
    std::string reconstruct() const override{
        return "(" + left_->reconstruct() + "^" + right_->reconstruct() + ")";
    }
};

struct EqNode : public BinaryOpNode{
    using BinaryOpNode::BinaryOpNode;
    CValue evaluate(std::map<CPos, CNode*>& cells) const override{
        CValue left = left_->evaluate(cells);
        CValue right = right_->evaluate(cells);
        return (left == right) * 1.0;
    }
    CNode* clone() const override{
        return new EqNode(*this);
    }
    std::string reconstruct() const override{
        return "(" + left_->reconstruct() + "=" + right_->reconstruct() + ")";
    }
};

struct NeNode : public BinaryOpNode{
    using BinaryOpNode::BinaryOpNode;
    CValue evaluate(std::map<CPos, CNode*>& cells) const override{
        CValue left = left_->evaluate(cells);
        CValue right = right_->evaluate(cells);
        return (left != right) * 1.0;
    }
    CNode* clone() const override{
        return new NeNode(*this);
    }
    std::string reconstruct() const override{
        return "(" + left_->reconstruct() + "<>" + right_->reconstruct() + ")";
    }
};

struct LtNode : public BinaryOpNode{
    using BinaryOpNode::BinaryOpNode;
    CValue evaluate(std::map<CPos, CNode*>& cells) const override{
        CValue left = left_->evaluate(cells);
        CValue right = right_->evaluate(cells);
        if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
            return (std::get<double>(left) < std::get<double>(right)) * 1.0;
        }
        if(std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)){
            return (std::get<std::string>(left) < std::get<std::string>(right)) * 1.0;
        }
        return CValue();
    }
    CNode* clone() const override{
        return new LtNode(*this);
    }
    std::string reconstruct() const override{
        return "(" + left_->reconstruct() + "<" + right_->reconstruct() + ")";
    }
};

struct LeNode : public BinaryOpNode{
    using BinaryOpNode::BinaryOpNode;
    CValue evaluate(std::map<CPos, CNode*>& cells) const override{
        CValue left = left_->evaluate(cells);
        CValue right = right_->evaluate(cells);
        if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
            return (std::get<double>(left) <= std::get<double>(right)) * 1.0;
        }
        if(std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)){
            return (std::get<std::string>(left) <= std::get<std::string>(right)) * 1.0;
        }
        return CValue();
    }
    CNode* clone() const override{
        return new LeNode(*this);
    }
    std::string reconstruct() const override{
        return "(" + left_->reconstruct() + "<=" + right_->reconstruct() + ")";
    }
};

struct GtNode : public BinaryOpNode{
    using BinaryOpNode::BinaryOpNode;
    CValue evaluate(std::map<CPos, CNode*>& cells) const override{
        CValue left = left_->evaluate(cells);
        CValue right = right_->evaluate(cells);
        if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
            return (std::get<double>(left) > std::get<double>(right)) * 1.0;
        }
        if(std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)){
            return (std::get<std::string>(left) > std::get<std::string>(right)) * 1.0;
        }
        return CValue();
    }
    CNode* clone() const override{
        return new GtNode(*this);
    }
    std::string reconstruct() const override{
        return "(" + left_->reconstruct() + ">" + right_->reconstruct() + ")";
    }
};

struct GeNode : public BinaryOpNode{
    using BinaryOpNode::BinaryOpNode;
    CValue evaluate(std::map<CPos, CNode*>& cells) const override{
        CValue left = left_->evaluate(cells);
        CValue right = right_->evaluate(cells);
        if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
            return (std::get<double>(left) >= std::get<double>(right)) * 1.0;
        }
        if(std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)){
            return (std::get<std::string>(left) >= std::get<std::string>(right)) * 1.0;
        }
        return CValue();
    }
    CNode* clone() const override{
        return new GeNode(*this);
    }
    std::string reconstruct() const override{
        return "(" + left_->reconstruct() + ">=" + right_->reconstruct() + ")";
    }
};

struct UnaryOpNode : public CNode{
    UnaryOpNode(CNode* child) : child_(child) {}
    UnaryOpNode(const UnaryOpNode& src){
        child_ = src.child_->clone();
    }
    ~UnaryOpNode() override{
        delete child_;
    }
    UnaryOpNode& operator =(const UnaryOpNode& other){
        if(this == &other)
            return *this;
        delete child_;
        child_ = other.child_->clone();
        return *this;
    }
    virtual void shift_references(int w, int h) override{
        child_->shift_references(w, h);
    }
    virtual bool hasCycle(std::set<CNode*>& visited, std::set<CNode*> rec_stack, const std::map<CPos, CNode*>& cells) const override{
        if(!rec_stack.insert((CNode*)this).second)
            return true;
        if(!visited.insert((CNode*)this).second)
            return false;
        return child_->hasCycle(visited, rec_stack, cells);
    }

    CNode* child_;
};

struct NegNode : public UnaryOpNode{
    using UnaryOpNode::UnaryOpNode;
    CValue evaluate(std::map<CPos, CNode*>& cells) const override{
        CValue child = child_->evaluate(cells);
        if(std::holds_alternative<double>(child))
            return -std::get<double>(child);
        return CValue();
    }
    CNode* clone() const override{
        return new NegNode(*this);
    }
    std::string reconstruct() const override{
        return "(-" + child_->reconstruct() + ")";
    }
};

struct ValNrNode : public CNode{
    ValNrNode(double d) : num_(d) {}
    CValue evaluate([[maybe_unused]]std::map<CPos, CNode*>& cells) const override{
        return num_;
    }
    CNode* clone() const override{
        return new ValNrNode(*this);
    }
    //return the stored number with 15 decimal points precision
    std::string reconstruct() const override{
        std::stringstream ss;
        ss << std::scientific << std::setprecision(15) << num_;
        return ss.str();

        //return std::to_string(num_);
    }

    double num_;
};

struct ValStrNode : public CNode{
    ValStrNode(const std::string& str) : str_(str) {}
    CValue evaluate([[maybe_unused]]std::map<CPos, CNode*>& cells) const override{
        return str_;
    }
    CNode* clone() const override{
        return new ValStrNode(*this);
    }
    //double the quotes that were undoubled by the parser and wrap the string in quotes, so that it will be correctly
    //parsed later again
    std::string reconstruct() const override{
        std::string res = "\"";
        for(char c : str_){
            if(c != '\"')
                res.push_back(c);
            else
                res += "\"\"";
        }
        return res + "\"";
    }

    std::string str_;
};

struct ValRefNode : public CNode{
    ValRefNode(const std::string& str){
        size_t idx = 0;
        char c = 'a';
        if(str[0] == '$'){
            col_abs_ = true;
            ++idx;
        }
        std::string col;
        for(; idx < str.size(); ++idx){
            c = str[idx];
            if(c < 'A' || c > 'Z')
                break;
            col.push_back(c);
        }
        if(c == '$'){
            row_abs_ = true;
            ++idx;
        }
        else if(idx == 0 || c < '0' || c > '9' || idx == str.size() || (c == '0' && idx != str.size() - 1))
            throw std::invalid_argument("");
        for(size_t i = idx + 1; i < str.size(); ++i)
            if(str[i] < '0' || str[i] > '9')
                throw std::invalid_argument("");
        col_ = getInt(col);
        row_ = std::atoi(&(str[idx]));
    }
    void shift_references(int w, int h) override{
        if(!col_abs_)
            col_ += w;
        if(!row_abs_)
            row_ += h;
    }
    CValue evaluate(std::map<CPos, CNode*>& cells) const override{
        CPos pos(col_, row_);
        auto it = cells.find(pos);
        if(it == cells.end() || it->second == nullptr)
            return CValue();
        return it->second->evaluate(cells);
    }
    CNode* clone() const override{
        return new ValRefNode(*this);
    }
    std::string reconstruct() const override{
        std::string res;
        if(col_abs_)
            res.push_back('$');
        res += getString(col_);
        if(row_abs_)
            res.push_back('$');
        res += std::to_string(row_);
        return res;
    }
    virtual bool hasCycle(std::set<CNode*>& visited, std::set<CNode*> rec_stack, const std::map<CPos, CNode*>& cells) const override{
        if(!rec_stack.insert((CNode*)this).second)
            return true;
        if(!visited.insert((CNode*)this).second)
            return false;
        auto it = cells.find(CPos(col_, row_));
        if(it == cells.end() || !it->second)
            return false;
        return it->second->hasCycle(visited, rec_stack, cells);
    }

    int col_ = 0;
    int row_ = 0;
    bool col_abs_ = false;
    bool row_abs_ = false;
};






//class used for building abstract syntax trees from the parsed expressions
class CASTBuilder : public CExprBuilder{
public:
    CASTBuilder() = default;
    ~CASTBuilder(){
        for(CNode* n : stack_)
            delete n;
    }
    void opAdd() override{
        CNode* right = stack_.front();
        stack_.pop_front();
        CNode* left = stack_.front();
        stack_.pop_front();
        CNode* node = new AddNode(left, right);
        stack_.push_front(node);
    }
    void opSub() override{
        CNode* right = stack_.front();
        stack_.pop_front();
        CNode* left = stack_.front();
        stack_.pop_front();
        CNode* node = new SubNode(left, right);
        stack_.push_front(node);
    }
    void opMul() override{
        CNode* right = stack_.front();
        stack_.pop_front();
        CNode* left = stack_.front();
        stack_.pop_front();
        CNode* node = new MulNode(left, right);
        stack_.push_front(node);
    }
    void opDiv() override{
        CNode* right = stack_.front();
        stack_.pop_front();
        CNode* left = stack_.front();
        stack_.pop_front();
        CNode* node = new DivNode(left, right);
        stack_.push_front(node);
    }
    void opPow() override{
        CNode* right = stack_.front();
        stack_.pop_front();
        CNode* left = stack_.front();
        stack_.pop_front();
        CNode* node = new PowNode(left, right);
        stack_.push_front(node);
    }
    void opNeg() override{
        CNode* node = new NegNode(stack_.front());
        stack_.pop_front();
        stack_.push_front(node);
    }
    void opEq() override{
        CNode* right = stack_.front();
        stack_.pop_front();
        CNode* left = stack_.front();
        stack_.pop_front();
        CNode* node = new EqNode(left, right);
        stack_.push_front(node);
    }
    void opNe() override{
        CNode* right = stack_.front();
        stack_.pop_front();
        CNode* left = stack_.front();
        stack_.pop_front();
        CNode* node = new NeNode(left, right);
        stack_.push_front(node);
    }
    void opLt() override{
        CNode* right = stack_.front();
        stack_.pop_front();
        CNode* left = stack_.front();
        stack_.pop_front();
        CNode* node = new LtNode(left, right);
        stack_.push_front(node);
    }
    void opLe() override{
        CNode* right = stack_.front();
        stack_.pop_front();
        CNode* left = stack_.front();
        stack_.pop_front();
        CNode* node = new LeNode(left, right);
        stack_.push_front(node);
    }
    void opGt() override{
        CNode* right = stack_.front();
        stack_.pop_front();
        CNode* left = stack_.front();
        stack_.pop_front();
        CNode* node = new GtNode(left, right);
        stack_.push_front(node);
    }
    void opGe() override{
        CNode* right = stack_.front();
        stack_.pop_front();
        CNode* left = stack_.front();
        stack_.pop_front();
        CNode* node = new GeNode(left, right);
        stack_.push_front(node);
    }
    void valNumber(double val) override{
        stack_.push_front(new ValNrNode(val));
    }
    void valString(std::string val) override{
        stack_.push_front(new ValStrNode(val));
    }
    void valReference(std::string val) override{
        stack_.push_front(new ValRefNode(val));
    }
    void valRange([[maybe_unused]]std::string val) override{}

    void funcCall([[maybe_unused]]std::string fnName, [[maybe_unused]]int paramCount) override{}


    //return root of the last built AST
    //this function is only called after an expression was successfully parsed and an AST has been built
    CNode* getAST() {
        CNode* root = stack_.front();
        stack_.pop_front();
        return root;
    }
private:
    std::deque<CNode*> stack_;
};







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

    std::map<CPos, CNode*>& cells() { return cells_; }

private:
    std::map<CPos, CNode*> cells_;
    CASTBuilder builder_;
};




CSpreadsheet::CSpreadsheet() = default;


//load stored cells from istream until it is empty
//assume only correctly parsed expressions were saved, return false if any error is encountered
//check whether the hash of a string of concatenated loaded expressions with their coordinates equals the saved hash
//if not return false - some cells were not loaded properly
bool CSpreadsheet::load(std::istream &is){
    CSpreadsheet x;
    int col = -1, row = -1;
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
           /* || col != 0 || row != 0*/)
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
#ifndef __PROGTEST__

//#include "expression.h"

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

    x0 = CSpreadsheet();

    assert(x0.save("savefile.txt"));
    assert(x0.load("savefile.txt"));

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

    assert (x0.setCell(CPos("VX5"), "1"));
    assert (x0.setCell(CPos("WZ5"), "2"));
    assert (x0.setCell(CPos("VY6"), "3"));
    assert (x0.setCell(CPos("WZ6"), "4"));
    assert (x0.setCell(CPos("VX55"), "1"));
    assert (x0.setCell(CPos("WZ577"), "2"));
    assert (x0.setCell(CPos("VY61"), "3"));
    assert (x0.setCell(CPos("WZ630"), "4"));


    assert (x0.setCell(CPos("A1"), "10/3"));
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





    return EXIT_SUCCESS;
}

#endif /* __PROGTEST__ */
