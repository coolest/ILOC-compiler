#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <stddef.h>

enum TokenCategory {
    TC_MEMOP, 
    TC_LOADI, 
    TC_ARITHOP, 
    TC_OUTPUT, 
    TC_NOP, 
    TC_CONSTANT, 
    TC_REGISTER, 
    TC_COMMA, 
    TC_INTO, 
    TC_EOF_TOKEN, 
    TC_EOL, 
    TC_ERROR,
    TC_COMMENT
};

enum CharacterState {
    CS_DIGIT, 
    CS_LETTER, 
    CS_REGISTER_START, 
    CS_OP, 
    CS_OTHER,
    CS_BACKSLASH,
};

enum State {
    S_INIT, 
    S_LETTER, 
    S_DIGIT, 
    S_OP, 
    S_REGISTER_START, 
    S_REGISTER_BODY, 
    S_EOF_TOKEN, 
    S_EOL, 
    S_ERROR,
    S_COMMENT_START,
    S_COMMENT,
};

const size_t NUM_TOKEN_CATEGORY = TokenCategory::TC_COMMENT + 1;
const size_t NUM_CHARACTER_CATEGORY = CharacterState::CS_BACKSLASH + 1;
const size_t NUM_STATE = State::S_COMMENT + 1;

#endif