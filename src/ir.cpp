#include "ir.hpp"

IR::IR(IR_OP_CODE op_code) : op_code{op_code} {
    //memset(arg1, 0, sizeof(arg1));
    //memset(arg2, 0, sizeof(arg2));
    //memset(arg3, 0, sizeof(arg3));
};

IR_Node::IR_Node(IR ir) : ir{ir} {

}