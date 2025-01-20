#ifndef PARSER_HPP
#define PARSER_HPP

#include "scanner.hpp"
#include "ir.hpp"
#include "enums.hpp"

struct ParserStatistics {

};

struct Parser {
    Scanner scanner;
    ParserStatistics stats;
    
    std::unique_ptr<IR_Node> parse();

    Parser(const std::string &filename);
};

#endif