#include "ir.hpp"

IR::IR(const IR_OP_CODE op_code) : op_code{op_code} {
    //memset(arg1, 0, sizeof(arg1));
    //memset(arg2, 0, sizeof(arg2));
    //memset(arg3, 0, sizeof(arg3));
};

IR::IR() : op_code{IR_OP_CODE::IR_ERROR} {

};

IR_Node::IR_Node(const IR& ir) : ir{ir} {

};

IR_Node::~IR_Node(){
    if (next){
        delete next;
    }
}