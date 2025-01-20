#include "parser.hpp"

IR expect_tokens(TokenCategory categories[], int n){
    return IR(IR_OP_CODE::IR_NOP);
};

Parser::Parser(const std::string &filename) : scanner{filename} {

}

IR_Node* Parser::parse(){
    return nullptr;
}