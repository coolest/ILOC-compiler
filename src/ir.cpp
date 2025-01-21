#include "ir.hpp"

IR::IR(const IR_OP_CODE op_code) : op_code{op_code} {
    memset(args, 0, sizeof(args));
};

IR::IR(const IR &other) : op_code{other.op_code} {
    memcpy(args, other.args, sizeof(args));

    if (other.error_lexeme) {
        error_lexeme = std::make_unique<std::string>(*other.error_lexeme);
    }
}

IR& IR::operator=(const IR& other){
    op_code = other.op_code;
    memcpy(args, other.args, sizeof(args));

    if (other.error_lexeme) {
        error_lexeme = std::make_unique<std::string>(*other.error_lexeme);
    } else {
        error_lexeme = nullptr;
    }

    return *this;
}

IR::IR() : op_code{IR_OP_CODE::IR_ERROR} {

};

IR_Node::IR_Node(const IR& ir) : ir{ir} {

};

IR_Node::~IR_Node(){
    if (next){
        delete next;
    }
}