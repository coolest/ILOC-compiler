#ifndef PARSER_HPP
#define PARSER_HPP

#include "scanner.hpp"
#include "ir.hpp"
#include "enums.hpp"

struct Parser {
    Scanner scanner;

    IR_Node* parse();

    Parser(const string &filename);
};

#endif