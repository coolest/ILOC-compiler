#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "enums.hpp"
#include <string>

struct Token {
    TokenCategory category;
    std::string lexeme;

    Token(TokenCategory c, const std::string &lex);
};

#endif