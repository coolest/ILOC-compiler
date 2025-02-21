#ifndef IR_HPP
#define IR_HPP

#include <stddef.h>
#include <array>
#include <string>
#include <cstdint>
#include <memory>
#include <cstring>
#include <vector>

enum IR_FIELD {
    SR, VR, PR, NE
};

enum IR_OP_CODE {
    IR_ERROR,
    IR_LOAD,
    IR_STORE,
    IR_ARITHOP,
    IR_LOADI,
    IR_NOP,
    IR_OUTPUT
};

enum IR_ARITH_OP {
    IR_ADD,
    IR_SUB,
    IR_MULT,
    IR_LSHIFT,
    IR_RSHIFT
};

const size_t NUM_IR_FIELD = IR_FIELD::NE + 1;
const size_t NUM_OP_CODES = IR_OP_CODE::IR_OUTPUT + 1;

struct IR {
    // If error happens we put lexeme here, otherwise nullptr
    std::string error_lexeme;

    IR_OP_CODE op_code;
    IR_ARITH_OP arith_op;
    
    int args[3][NUM_IR_FIELD];

    IR(const IR_OP_CODE op_code);
    IR();
};

struct IR_Extra {
    std::vector<IR> before;
    std::vector<IR> after;
};

struct IR_Node {
    IR ir;
    uint32_t line;
    std::unique_ptr<IR_Extra> extra;

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