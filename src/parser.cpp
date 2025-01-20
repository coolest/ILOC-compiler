#include "parser.hpp"
#include <memory>

Parser::Parser(const std::string &filename) : scanner{filename}, stats{} {

}

// Helper function for ::parse()
IR expect_tokens(
    Scanner &scanner, 
    const IR_OP_CODE op_code,
    const TokenCategory categories[], 
    const uint8_t n
){
    IR block(op_code);
    int arg_num = 0;

    for (int i = 0; i < n; i++){
        Token token = scanner.scan();
        if (token.category != categories[i]){
            block.op_code = IR_OP_CODE::IR_ERROR; // sentence level error
            block.args[0][0] = categories[i]; // expected
            block.args[0][1] = token.category; // received

            break; // do not continue parsing sentence
        } else if (token.category == TokenCategory::TC_CONSTANT || token.category == TokenCategory::TC_REGISTER){
            block.args[arg_num][IR_FIELD::SR] = 
                token.category == TokenCategory::TC_CONSTANT ? stoi(token.lexeme) : stoi(token.lexeme.substr(1));
        }
    }

    return block;
};

// Preset the sentences we look for, so we do not create new arrays every iteration
const uint8_t arithop_n = 5;
const TokenCategory arithop_categories[] = 
    { TokenCategory::TC_REGISTER, TokenCategory::TC_COMMA, TokenCategory::TC_REGISTER, TokenCategory::TC_INTO, TokenCategory::TC_REGISTER };
const uint8_t output_n = 1;
const TokenCategory output_categories[] = 
    { TokenCategory::TC_CONSTANT };
const uint8_t memop_n = 3;
const TokenCategory memop_categories[] = 
    { TokenCategory::TC_REGISTER, TokenCategory::TC_INTO, TokenCategory::TC_REGISTER };
const uint8_t loadi_n = 3;
const TokenCategory loadi_categories[] = 
    { TokenCategory::TC_CONSTANT, TokenCategory::TC_INTO, TokenCategory::TC_REGISTER };

std::unique_ptr<IR_Node> Parser::parse(){
    Token start = scanner.scan();

    std::unique_ptr<IR_Node> head; // Once head is deleted it recursively deletes node->next
    IR_Node* tail = head.get();

    while (start.category != TokenCategory::TC_EOF_TOKEN){
        IR block(IR_OP_CODE::IR_NOP); // Makes switches easier, automatically handles NOP case.

        switch(start.category){
            case TokenCategory::TC_ARITHOP:
                block = expect_tokens(scanner, IR_OP_CODE::IR_ARITHOP, arithop_categories, arithop_n);
                break;

            case TokenCategory::TC_OUTPUT:
                block = expect_tokens(scanner, IR_OP_CODE::IR_OUTPUT, output_categories, output_n);
                break;

            case TokenCategory::TC_MEMOP:
                block = expect_tokens(scanner, IR_OP_CODE::IR_MEMOP, memop_categories, memop_n);
                break;

            case TokenCategory::TC_LOADI:
                block = expect_tokens(scanner, IR_OP_CODE::IR_LOADI, loadi_categories, loadi_n);
                break;

            case TokenCategory::TC_NOP:
                break;

            default:
                block.op_code = IR_ERROR;
                block.args[1][0] = start.category; // OP level error
        }

        start = scanner.scan();
        if (head == nullptr){
            head = std::make_unique<IR_Node>(block);
            tail = head.get();
        } else {
            tail->next = new IR_Node(block);
            tail->next->prev = tail;

            tail = tail->next;
        }

        if (block.op_code == IR_OP_CODE::IR_ERROR){
            stats.errors++;
        }

        // Ignore for now
        /*
        if (block.op_code == IR_OP_CODE::IR_ERROR){
            if (block.args[1][0] > 0){ // Indicates a OP level error, start.category > 0 always.

            } else { // Sentence level error, read args[0][0], args[0][1]

            }
        }
        */
    }

    return head;
}