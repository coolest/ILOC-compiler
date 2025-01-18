#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <stddef.h>

enum TokenCategory {
    MEMOP, LOADI, ARITHOP, OUTPUT, NOP, CONSTANT, REGISTER, COMMA, INTO, EOF_TOKEN, EOL, ERROR
};

enum CharacterState {
    DIGIT, LETTER, REGISTER_START, OP, OTHER
};

enum State {
    INIT, LETTER, DIGIT, OP, REGISTER_START, REGISTER_BODY, EOF_TOKEN, EOL, ERROR
};

const size_t NUM_TOKEN_CATEGORY = TokenCategory::ERROR + 1;
const size_t NUM_CHARACTER_CATEGORY = CharacterState::OTHER + 1;
const size_t NUM_STATE = State::ERROR + 1;

#endif