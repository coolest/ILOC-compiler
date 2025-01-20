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

    {"loadI",   TokenCategory::TC_LOADI},

    {"nop",     TokenCategory::TC_NOP},

    {"output",  TokenCategory::TC_OUTPUT},

    {"=>",      TokenCategory::TC_INTO},
};

const State Scanner::t[NUM_STATE][NUM_CHARACTER_CATEGORY] = {
    { State::S_DIGIT, State::S_LETTER, State::S_REGISTER_START, State::S_OP, State::S_ERROR, State::S_COMMENT_START }, // INIT
    { State::S_ERROR, State::S_LETTER, State::S_LETTER, State::S_ERROR, State::S_ERROR, State::S_ERROR }, // LETTER
    { State::S_DIGIT, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR }, // DIGIT
    { State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_OP, State::S_ERROR, State::S_ERROR }, // OP
    { State::S_REGISTER_BODY, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR }, // REGISTER_START
    { State::S_REGISTER_BODY, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR }, // REGISTER_BODY
    { State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR }, // EOF
    { State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR }, // EOL
    { State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR }, // ERROR
    { State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_ERROR, State::S_COMMENT }, // COMMENT START
    { State::S_COMMENT, State::S_COMMENT, State::S_COMMENT, State::S_COMMENT, State::S_COMMENT, State::S_COMMENT }, // COMMENT
};

CharacterState Scanner::char_states[128];

bool Scanner::init = false;

Scanner::Scanner(const std::string &f) : current_read{0}, i{0}, eof{false}, fstream{f} {
    if (fstream.fail()){
        std::cerr << "File " << f << " does not exist or cannot be opened!\n";
    }

    if (!init){
        for (int i = 0; i < 128; i++) char_states[(size_t) i] = CharacterState::CS_OTHER;
    
        char_states['>'] = CharacterState::CS_OP;
        char_states['='] = CharacterState::CS_OP;

        for (char c = '0'; c <= '9'; c++) char_states[(size_t) c] = CharacterState::CS_DIGIT;

        for (char c = 'a'; c <= 'z'; c++) char_states[(size_t) c] = CharacterState::CS_LETTER;
        for (char c = 'A'; c <= 'Z'; c++) char_states[(size_t) c] = CharacterState::CS_LETTER;

        char_states['/'] = CharacterState::CS_BACKSLASH;

        char_states['r'] = CharacterState::CS_REGISTER_START;

        init = true;
    }
}

Token Scanner::scan(){
    if (fstream.eof() || eof){
        return Token(TokenCategory::TC_EOF_TOKEN, "EOF");
    } else if (fstream.fail()){
        return Token(TokenCategory::TC_EOF_TOKEN, "File does not exist or cannot be opened!");
    }

    std::string lexeme;

    char c;

    State current_state = State::S_INIT;
    State history = current_state; // Normally a stack, but not necessary for ILOC language

    /*
        Only ignore whitespace at the beginning of a token, SINCE we error at the end/middle of a token
            Why? because 'lo ad' is not valid.

        Anytime we see a comma and INIT state we immediately return the token-
            Comma is not a character state

        If we see newline have to manually handle
            - If INIT state, we return newline token
            - If comment, we manually handle since comment accepts everything
            - Otherwise, the other category will error keyword, etc and it will rollback

        If we are unable to read into buffer, then we do checks for EOF (similar to newline)
    */

    while (current_state != State::S_ERROR){
        // End of buffer
        if (i == current_read){
            fstream.read(buf, sizeof(buf));

            current_read = fstream.gcount();
            if (current_read == 0) {
                if (current_state == State::S_INIT){
                    return Token(TokenCategory::TC_EOF_TOKEN, "EOF");
                } else if (current_state == State::S_COMMENT){
                    eof = true;

                    return Token(TokenCategory::TC_COMMENT, lexeme);
                }
            }

            i = 0;
        }

        c = buf[i++];

        if ((c == ' ' || c == '\r' || c == '\t') && current_state == State::S_INIT){
            continue;
        } else if (c == ',' && current_state == State::S_INIT){
            return Token(TokenCategory::TC_COMMA, ",");
        } else if (c == '\n'){
            if (current_state == State::S_INIT){
                return Token(TokenCategory::TC_EOL, "EOL");
            } else if (current_state == State::S_COMMENT){
                i--;

                return Token(TokenCategory::TC_COMMENT, lexeme);
            }
        }

        CharacterState char_state = char_states[(size_t) c];

        history = current_state;
        current_state = t[current_state][char_state];

        lexeme += c;
    }

    // Don't pop off its a single character ... we would infinite loop nothing -> x -> error -> rollback -> nothing
    if (history != State::S_INIT){
        if (!fstream.eof()) i--;
        lexeme.pop_back();
    }

    current_state = history;
    if (current_state == State::S_REGISTER_BODY){
        return Token(TokenCategory::TC_REGISTER, lexeme);
    } else if (current_state == State::S_DIGIT){
        return Token(TokenCategory::TC_CONSTANT, lexeme);
    } else {
        if (!keyword_category.count(lexeme)){
            return Token(TokenCategory::TC_ERROR, "Invalid Keyword: " + lexeme);
        }

        return Token(keyword_category.at(lexeme), lexeme);
    }
}