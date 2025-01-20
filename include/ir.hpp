#ifndef IR_HPP
#define IR_HPP

#include <stddef.h>
#include <array>

enum IR_FIELD {
    SR, VR, PR, NE
};

enum IR_OP_CODE {
    IR_MEMOP, 
    IR_ARITHOP,
    IR_LOADI,
    IR_NOP,
    IR_OUTPUT
};

const size_t NUM_IR_FIELD = IR_FIELD::NE + 1;
const size_t NUM_OP_CODES = IR_OP_CODE::IR_OUTPUT + 1;

struct IR {
    IR_OP_CODE op_code;

    int arg1[NUM_IR_FIELD];
    int arg2[NUM_IR_FIELD];
    int arg3[NUM_IR_FIELD];

    IR(IR_OP_CODE op_code);
};

struct IR_Node {
    IR ir;

    IR_Node* prev;
    IR_Node* next;

    IR_Node(IR ir);
};

#endif