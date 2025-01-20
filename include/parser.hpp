#ifndef PARSER_HPP
#define PARSER_HPP

#include "scanner.hpp"
#include "ir.hpp"
#include "enums.hpp"
#include <memory>

struct ParserStatistics {
    int errors = 0;
};

struct Parser {
    Scanner scanner;
    ParserStatistics stats;

    std::unique_ptr<IR_Node> parse();

    Parser(const std::string &filename);
};

#endif