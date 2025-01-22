#ifndef SCANNER_HPP
#define SCANNER_HPP

#include "enums.hpp"
#include "token.hpp"
#include <string>
#include <fstream>
#include <stack>

/*
    Only a fixed set of characters will be put in this trie, due to the limited grammar of ILOC.
    This TrieNode will act as a cache as the lexeme builds, the 'cache' will change due to traversing the trie.
        * Once lexeme is fully built, it looks at cache before doing hashmap lookup for keywords which saves us another O(n) for the hash
        * Initially it will cost more, but over time the trie becomes fully built and it is actively completely like a cache.
*/
struct TrieNode {
    bool terminal = false;

    TrieNode* parent = nullptr;
    std::unique_ptr<TrieNode> next[128];

    TokenCategory category;
    State state;

    TrieNode();
};

struct Scanner {
    static const State t[NUM_STATE][NUM_CHARACTER_CATEGORY];
    static CharacterState char_states[128];
    static bool init;

    int bytes_read;
    
    char buf[4096 * 2];
    int current_read;
    int i;

    bool eof;
    std::ifstream fstream;
    std::unique_ptr<TrieNode> cache;

    Scanner(const std::string &f);
    Token scan();
};

#endif