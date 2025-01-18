#include "flag_dispatcher.hpp"
#include "scanner.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[]){
    FlagDispatch dispatch;

    if (argc <= 2){ // either -h or <filename>
        if (std::string(argv[1]) == "-h"){
            dispatch.help(); // help flag passed
        }else{
            dispatch.parse(argv[1]); // default flag, passed in the file name
        }

        return 1;
    }

    // assuming proper use
    char flag = argv[1][1];
    switch(flag){
        case 'p':
            dispatch.parse(argv[2]);
            break;

        case 's':
            dispatch.scan(argv[2]);
            break;

        case 'r':
            dispatch.read(argv[2]);
            break;

        default:
            std::cerr << "Improper flag used, please use '434fe -h'\n";
    }
}