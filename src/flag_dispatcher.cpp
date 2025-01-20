
#include "flag_dispatcher.hpp"
#include "scanner.hpp"
#include "token.hpp"
#include <string>
#include <iostream>
#include <chrono>

using std::cout;

void FlagDispatch::help(){
    cout 
        << "434fe -h:\n\t* Prints interactive flags for 434fe\n"
        << "434fe -s <name>:\n\t* Prints, to the standard output stream, a list of the tokens that the scanner found.\n"
        << "434fe -p <name> [DEFAULT FLAG]:\n\t* Scans it and parses it, builds the intermediate representation, and reports either success or all the errors that it finds in the input file.\n"
        << "434fe -s <name>:\n\t* Scans it, parses it, builds the intermediate representation, and prints out the information in the intermediate representation (in an appropriately human readable format).\n\n"
        << "434fe <flag> IS MUTAUALLY EXCLUSIVE... if more than one flag is passed, you are taken here.\n";
};

void FlagDispatch::scan(const std::string &filename){
    std::cout << std::endl;
    auto perform = [&](bool print){
        Scanner scanner(filename);

        Token token = Token(TokenCategory::TC_OUTPUT, "");
        while (token.category != TokenCategory::TC_EOF_TOKEN){
            token = scanner.scan();

            if (print) printToken(token);
        }

        std::cout << std::endl;
    };

    // Printing tokens:
    perform(true);

    // Performance:
    auto start = std::chrono::high_resolution_clock::now();

    perform(false);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Takes: " << (double) duration.count() / (double) 1000000 << "s" << std::endl;
};

void FlagDispatch::parse(const std::string &filename){

};

void FlagDispatch::read(const std::string &filename){

};