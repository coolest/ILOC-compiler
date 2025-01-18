
#include "flag_dispatcher.hpp"
#include <string>
#include <iostream>

using std::cout;

void FlagDispatch::help(){
    cout 
        << "434fe -h:\n\t* Prints interactive flags for 434fe\n"
        << "434fe -s <name>:\n\t* Prints, to the standard output stream, a list of the tokens that the scanner found.\n"
        << "434fe -p <name> [DEFAULT FLAG]:\n\t* Scans it and parses it, builds the intermediate representation, and reports either success or all the errors that it finds in the input file.\n"
        << "434fe -s <name>:\n\t* Scans it, parses it, builds the intermediate representation, and prints out the information in the intermediate representation (in an appropriately human readable format).\n";
};

// verify proper file

void FlagDispatch::scan(const std::string &filename){

};

void FlagDispatch::parse(const std::string &filename){

};

void FlagDispatch::read(const std::string &filename){

};