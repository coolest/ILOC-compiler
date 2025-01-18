#ifndef SCANNER_HPP
#define SCANNER_HPP

#include "enums.hpp"
#include "token.hpp"
#include <string>
#include <fstream>
#include <stack>

struct Scanner {
    static const State t[NUM_STATE][NUM_CHARACTER_CATEGORY];
    static CharacterState char_states[128];
    static bool init;

    std::ifstream fstream;
    std::stack<State> state_history;

    Scanner(const std::string &f);
    Token Scan();
};

#endif