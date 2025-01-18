#include <unordered_map>
#include <string>

#include "scanner.hpp"
#include "token.hpp"
#include <fstream>
#include <iostream>
#include <stack>

const std::unordered_map<std::string, TokenCategory> keyword_category = {
    {"\n",      TokenCategory::EOL},
    {"\r\n",    TokenCategory::EOL},

    {"store",   TokenCategory::MEMOP},
    {"load",    TokenCategory::MEMOP},

    {"sub",     TokenCategory::ARITHOP},
    {"add",     TokenCategory::ARITHOP},
    {"lshift",  TokenCategory::ARITHOP},
    {"rshift",  TokenCategory::ARITHOP},
    {"mult",    TokenCategory::ARITHOP},

    {"loadi",   TokenCategory::LOADI},

    {"nop",     TokenCategory::NOP},

    {"output",  TokenCategory::OUTPUT},

    {"=>",      TokenCategory::INTO},
};

void Scanner::initialize(){
    const State t[NUM_STATE][NUM_CHARACTER_CATEGORY] = {
        { State::ERROR, State::LETTER, State::REGISTER_START, ::OP, State::ERROR }, // INIT
        { State::ERROR, State::LETTER, State::ERROR, State::ERROR, State::ERROR }, // LETTER
        { State::ERROR, State::LETTER, State::REGISTER_BODY, State::ERROR, State::ERROR }, // DIGIT
        { State::ERROR, State::ERROR, State::ERROR, State::OP, State::ERROR }, // OP
        { State::REGISTER_BODY, State::ERROR, State::ERROR, State::ERROR, State::ERROR }, // REGISTER_START
        { State::REGISTER_BODY, State::ERROR, State::ERROR, State::ERROR, State::ERROR }, // REGISTER_BODY
        { State::ERROR, State::ERROR, State::ERROR, State::ERROR, State::ERROR }, // EOF
        { State::ERROR, State::ERROR, State::ERROR, State::ERROR, State::ERROR }, // EOL
        { State::ERROR, State::ERROR, State::ERROR, State::ERROR, State::ERROR }, // ERROR
    };

    for (int i = 0; i < 128; i++) char_states[i] = CharacterState::OTHER;
    
    char_states['<'] = CharacterState::OP;
    char_states['='] = CharacterState::OP;

    for (char c = '0'; c <= '9'; c++) char_states[c] = CharacterState::DIGIT;

    for (char c = 'a'; c <= 'z'; c++) char_states[c] = CharacterState::LETTER;
    for (char c = 'A'; c <= 'Z'; c++) char_states[c] = CharacterState::LETTER;
}

// comma will be manual, and rDIGITS will also be manual (?)

Scanner::Scanner(const std::string &f) : fstream{f}, state_history{} {
    if (fstream.fail()){
        std::cerr << "File " << f << " does not exist or cannot be opened!\n";
    }
}

Token Scanner::Scan(){
    if (fstream.fail()){
        return Token(TokenCategory::ERROR, "File does not exist or cannot be opened!");
    } else if (fstream.eof()){
        return Token(TokenCategory::EOF_TOKEN, "EOF");
    }

    std::string lexeme;
    char c;

    State current_state = State::INIT;

    while (!state_history.empty()) state_history.pop();
    state_history.push(current_state);

    while (current_state != State::ERROR){
        fstream.get(c);

        CharacterState char_state = char_states[c];
        current_state = t[current_state][char_state];

        state_history.push(current_state);
        lexeme += c;
    }

    while (current_state == State::ERROR){
        current_state = state_history.top();
        state_history.pop();

        fstream.putback(lexeme.back());
        lexeme.pop_back();
    }

    if (current_state == State::REGISTER_BODY){
        return Token(TokenCategory::REGISTER, lexeme);
    } else {
        if (!keyword_category.count(lexeme)){
            return Token(TokenCategory::ERROR, lexeme);
        }

        return Token(keyword_category.at(lexeme), lexeme);
    }
}