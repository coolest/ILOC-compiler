
#include "flag_dispatcher.hpp"
#include "scanner.hpp"
#include "parser.hpp"
#include "token.hpp"
#include <string>
#include <iostream>
#include <chrono>
#include <cmath>

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
    auto perform = [&](bool print) -> int{
        Scanner scanner(filename);

        Token token = Token(TokenCategory::TC_OUTPUT, "");
        while (token.category != TokenCategory::TC_EOF_TOKEN){
            token = scanner.scan();

            if (print) printToken(token);
        }

        return scanner.bytes_read;
    };

    // Printing tokens:
    perform(true);
    std::cout << std::endl;

    // Performance:
    auto start = std::chrono::high_resolution_clock::now();

    int bytes_read = perform(false);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    double seconds = (double) duration.count() / (double) 1000000;
    std::cout << std::endl;
    std::cout << "Takes: " << seconds  << " s" << std::endl;

    // Pretty print data/s
    std::string suffix = " B/s";
    double data_per_second = (double) bytes_read / seconds;
    if (data_per_second > std::pow(1024, 3)){
        data_per_second /= std::pow(1024, 3);
        suffix = " GB/s";
    } else if (data_per_second > std::pow(1024, 2)){
        data_per_second /= std::pow(1024, 2);
        suffix = " MB/s";
    } else if (data_per_second > 1024){
        data_per_second /= 1024;
        suffix = " KB/s";
    }

    std::cout << "Approx: " << data_per_second << suffix << std::endl;
};

void FlagDispatch::parse(const std::string &filename){
    std::cout << std::endl;

    // Performance:
    auto start = std::chrono::high_resolution_clock::now();

    Parser parser(filename);
    std::unique_ptr<IR_Node> ir_head = parser.parse(); // Maybe do something with this .. ?

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << parser.stats.errors << " errors found." << std::endl;
    std::cout << "Takes: " << (double) duration.count() / (double) 1000000 << "s" << std::endl;
};

void FlagDispatch::read(const std::string &filename){

};