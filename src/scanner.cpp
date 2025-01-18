#include <unordered_map>
#include <string>

#include "scanner.hpp"
#include "token.hpp"
#include <fstream>
#include <iostream>
#include <stack>

const std::unordered_map<std::string, TokenCategory> keyword_category = {
    {"\n",      TokenCategory::TC_EOL},
    {"\r\n",    TokenCategory::TC_EOL},

    {"store",   TokenCategory::TC_MEMOP},
    {"load",    TokenCategory::TC_MEMOP},

    {"sub",     TokenCategory::TC_ARITHOP},
    {"add",     TokenCategory::TC_ARITHOP},
    {"lshift",  TokenCategory::TC_ARITHOP},
    {"rshift",  TokenCategory::TC_ARITHOP},
    {"mult",    TokenCategory::TC_ARITHOP},

    {"loadi",   TokenCategory::TC_LOADI},

    {"nop",     TokenCategory::TC_NOP},

    {"output",  TokenCategory::TC_OUTPUT},

    {"=>",      TokenCategory::TC_INTO},
};

const State Scanner::t[NUM_STATE][NUM_CHARACTER_CATEGORY] = {
    { State::S_ERROR, State::S_LETTER, State::S_REGISTER_START, State::S_OP, State::S_ERROR }, // INIT
    { State::S_ERROR, State::S_LETTER, State::S_ERROR, State::S_ERROR, State::S_ERROR }, // LETTER
    { State::S_ERROR, State::S_LETTER, State::S_REGISTER_BODY, State::S_ERROR, State::S_ERROR }, // DIGIT
    { State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_OP, State::S_ERROR }, // OP
    { State::S_REGISTER_BODY, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR }, // REGISTER_START
    { State::S_REGISTER_BODY, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR }, // REGISTER_BODY
    { State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR }, // EOF
    { State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR }, // EOL
    { State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR }, // ERROR
};

CharacterState Scanner::char_states[128];

bool Scanner::init = false;

Scanner::Scanner(const std::string &f) : fstream{f}, state_history{} {
    if (fstream.fail()){
        std::cerr << "File " << f << " does not exist or cannot be opened!\n";
    }

    if (!init){
        for (int i = 0; i < 128; i++) char_states[(size_t) i] = CharacterState::CS_OTHER;
    
        char_states['<'] = CharacterState::CS_OP;
        char_states['='] = CharacterState::CS_OP;

        for (char c = '0'; c <= '9'; c++) char_states[(size_t) c] = CharacterState::CS_DIGIT;

        for (char c = 'a'; c <= 'z'; c++) char_states[(size_t) c] = CharacterState::CS_LETTER;
        for (char c = 'A'; c <= 'Z'; c++) char_states[(size_t) c] = CharacterState::CS_LETTER;

        init = true;
    }
}

Token Scanner::Scan(){
    if (fstream.fail()){
        return Token(TokenCategory::TC_ERROR, "File does not exist or cannot be opened!");
    } else if (fstream.eof()){
        return Token(TokenCategory::TC_EOF_TOKEN, "EOF");
    }

    std::string lexeme;
    char c;

    State current_state = State::S_INIT;

    while (!state_history.empty()) state_history.pop();
    state_history.push(current_state);

    while (current_state != State::S_ERROR){
        fstream.get(c);

        CharacterState char_state = char_states[(size_t) c];
        current_state = t[current_state][char_state];

        state_history.push(current_state);
        lexeme += c;
    }

    while (current_state == State::S_ERROR){
        current_state = state_history.top();
        state_history.pop();

        fstream.putback(lexeme.back());
        lexeme.pop_back();
    }

    if (current_state == State::S_REGISTER_BODY){
        return Token(TokenCategory::TC_REGISTER, lexeme);
    } else {
        if (!keyword_category.count(lexeme)){
            return Token(TokenCategory::TC_ERROR, lexeme);
        }

        return Token(keyword_category.at(lexeme), lexeme);
    }
}