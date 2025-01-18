#include "token.hpp"
#include <iostream>

Token::Token(TokenCategory c, const std::string &lex) : category{c}, lexeme{lex} {
    
}

void printToken(const Token &t){
    switch (t.category){
        case TokenCategory::TC_ARITHOP:
            std::cout << "<ARITHOP, '" << t.lexeme << "'> ";
            return;

        case TokenCategory::TC_COMMA:
            std::cout << "<COMMA, ','> ";
            return;

        case TokenCategory::TC_CONSTANT:
            std::cout << "<CONSTANT, '" << t.lexeme << "'> ";
            return;

        case TokenCategory::TC_EOF_TOKEN:
            std::cout << "<EOF, '" << t.lexeme << "'> ";
            return;

        case TokenCategory::TC_EOL:
            std::cout << "<EOL, '" << t.lexeme << "'>\n";
            return;

        case TokenCategory::TC_ERROR:
            std::cout << "<ERROR, '" << t.lexeme << "'> ";
            return;

        case TokenCategory::TC_INTO:
            std::cout << "<INTO, '" << t.lexeme << "'> ";
            return;

        case TokenCategory::TC_LOADI:
            std::cout << "<LOADI, '" << t.lexeme << "'> ";
            return;

        case TokenCategory::TC_MEMOP:
            std::cout << "<MEMOP, '" << t.lexeme << "'> ";
            return;

        case TokenCategory::TC_NOP:
            std::cout << "<NOP, '" << t.lexeme << "'> ";
            return;

        case TokenCategory::TC_OUTPUT:
            std::cout << "<OUTPUT, '" << t.lexeme << "'> ";
            return;

        case TokenCategory::TC_REGISTER:
            std::cout << "<REGISTER, '" << t.lexeme << "'> ";
            return;

        case TokenCategory::TC_COMMENT:
            std::cout << "<COMMENT, '" << t.lexeme << "'> ";
            return;
    }
}