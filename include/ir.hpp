#ifndef IR_HPP
#define IR_HPP

#include <stddef.h>
#include <array>
#include <string>
#include <cstdint>
#include <memory>

enum IR_FIELD {
    SR, VR, PR, NE
};

enum IR_OP_CODE {
    IR_ERROR,
    IR_MEMOP, 
    IR_ARITHOP,
    IR_LOADI,
    IR_NOP,
    IR_OUTPUT
};

const size_t NUM_IR_FIELD = IR_FIELD::NE + 1;
const size_t NUM_OP_CODES = IR_OP_CODE::IR_OUTPUT + 1;

struct IR {
    // If error happens we put lexeme here, otherwise nullptr
    std::string error_lexeme;

    IR_OP_CODE op_code;
    int args[3][NUM_IR_FIELD];

    IR(const IR_OP_CODE op_code);
    IR();
};

struct IR_Node {
    IR ir;
    uint32_t line;

    IR_Node(const IR &ir);
    IR_Node();
};

struct IR_NodePool {
    static const u_int16_t POOL_SIZE = 1024;
    u_int16_t i = 0;

    IR_Node pool[POOL_SIZE];

    IR_NodePool* next = nullptr;
    IR_NodePool* prev = nullptr;

    IR_NodePool();
    ~IR_NodePool();
};

#endif