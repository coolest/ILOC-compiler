#include "token.hpp"

Token::Token(TokenCategory c, const std::string &lex) : category{c}, lexeme{lex} {
    
}