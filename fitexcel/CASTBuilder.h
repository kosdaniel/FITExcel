


#include <string>
#include <deque>
#include "CNode.h"
#include "expression.h"


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
