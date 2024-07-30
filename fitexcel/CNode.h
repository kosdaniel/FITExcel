
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
#include "CPos.h"

using CValue = std::variant<std::monostate, double, std::string>;

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
    //parsed again later
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

