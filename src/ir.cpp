#include "ir.hpp"

IR::IR(const IR_OP_CODE op_code) : op_code{op_code} {
    memset(args, 0, sizeof(args));
};

IR::IR() : op_code{IR_OP_CODE::IR_ERROR} {

};

IR_Node::IR_Node(const IR& ir) : ir{ir} {

};

IR_Node::IR_Node() {
    
}

IR_NodePool::IR_NodePool() : pool() {

};

IR_NodePool::~IR_NodePool() {
    if (next){
        delete next;
    }
}