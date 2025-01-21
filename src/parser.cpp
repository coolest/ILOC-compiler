#include "parser.hpp"
#include <iostream>

Parser::Parser(const std::string &filename) : scanner{filename}, stats{} {

}

// Helper function for ::parse()
std::string string_of_token_category_enum(int code){
    // Whenever error happens in parsing we have EXPECT [token code] RECEIVED [token code]
    // We will pretty print this to std::out

    std::string result = "ERROR";
    switch (code){
        case TokenCategory::TC_MEMOP: 
            result = "MEMOP"; 
            break;

        case TokenCategory::TC_LOADI: 
            result = "LOADI"; 
            break;
        
        case TokenCategory::TC_ARITHOP: 
            result = "ARITHOP"; 
            break;

        case TokenCategory::TC_OUTPUT: 
            result = "OUTPUT"; 
            break;

        case TokenCategory::TC_NOP: 
            result = "NOP"; 
            break;

        case TokenCategory::TC_CONSTANT: 
            result = "CONSTANT"; 
            break;

        case TokenCategory::TC_REGISTER: 
            result = "REGISTER"; 
            break;

        case TokenCategory::TC_COMMA: 
            result = "COMMA"; 
            break;

        case TokenCategory::TC_INTO: 
            result = "=>"; 
            break;

        case TokenCategory::TC_EOL: 
            result = "EOL"; 
            break;

        case TokenCategory::TC_COMMENT: 
            result = "COMMENT"; 
            break;
    }

    return result;
}

// Only handles rXXX or XXX, where XXX is a positive integer.
int fast_stoi(const std::string& s, uint8_t i){
    int num = 0;

    uint8_t n = s.size();
    for (; i < n; i++){
        num *= 10;
        num += s[i] - '0'; // Doesn't check if valid string, can assume valid rXXX or XXX
    }

    return num;
}

IR expect_tokens(
    Scanner &scanner,
    int &line,

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

            if (token.category == TokenCategory::TC_EOL){
                line ++;
            }

            break; // do not continue parsing sentence
        } else if (token.category == TokenCategory::TC_CONSTANT || token.category == TokenCategory::TC_REGISTER){
            block.args[arg_num][IR_FIELD::SR] = fast_stoi(token.lexeme, token.category == TokenCategory::TC_REGISTER);
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

    int line = 1;
    while (start.category != TokenCategory::TC_EOF_TOKEN){
        // Typically lines will be after a sentence, error if they are in between sentences as 'load r1 \n => r2' shouldn't pass
        if (start.category == TokenCategory::TC_EOL){
            start = scanner.scan();
            line ++;

            continue;
        }
        
        IR block(IR_OP_CODE::IR_NOP); // Makes switches easier, automatically handles NOP case.

        switch(start.category){
            case TokenCategory::TC_ARITHOP:
                block = expect_tokens(scanner, line, IR_OP_CODE::IR_ARITHOP, arithop_categories, arithop_n);
                break;

            case TokenCategory::TC_OUTPUT:
                block = expect_tokens(scanner, line, IR_OP_CODE::IR_OUTPUT, output_categories, output_n);
                break;

            case TokenCategory::TC_MEMOP:
                block = expect_tokens(scanner, line, IR_OP_CODE::IR_MEMOP, memop_categories, memop_n);
                break;

            case TokenCategory::TC_LOADI:
                block = expect_tokens(scanner, line, IR_OP_CODE::IR_LOADI, loadi_categories, loadi_n);
                break;

            case TokenCategory::TC_NOP:
                break;

            // Treat comments as NOP, in middle pass, we can filter out these NOPs
            case TokenCategory::TC_COMMENT:
                break;

            default:
                block.op_code = IR_ERROR;
                block.args[1][0] = start.category; // OP level error
        }

        start = scanner.scan();
        if (head == nullptr){
            head = std::make_unique<IR_Node>(block);
            head->line = line;

            tail = head.get();
        } else {
            tail->next = new IR_Node(block);
            tail->next->prev = tail;

            tail = tail->next;
            tail->line = line;
        }

        if (block.op_code == IR_OP_CODE::IR_ERROR){
            stats.errors++;

            if (block.args[1][0] > 0){ // Indicates a OP level error, start.category > 0 always.
                std::cerr 
                    << "Line " << line 
                    << ": Expected start of sentence [ARITHOP, OUTPUT, MEMOP, LOADI, NOP], got: " 
                    << string_of_token_category_enum(block.args[1][0]) 
                    << std::endl;
            } else { // Sentence level error, read args[0][0], args[0][1]
                std::cerr 
                    << "Line " << line 
                    << ": Expected " << string_of_token_category_enum(block.args[0][0]) 
                    << ", got: " << string_of_token_category_enum(block.args[0][1]) 
                    << std::endl;
            }
        }
    }

    return head;
}