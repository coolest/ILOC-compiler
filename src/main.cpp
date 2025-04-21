#include "flag_dispatcher.hpp"
#include "scanner.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[]){
    FlagDispatch dispatch;

    // LAB 3, just a gaurd then do what we have been doing
    if (argc == 2){
        dispatch.schedule(argv[1]);

        return 1;
    }

    if (argc <= 2){ // either -h or <filename>
        if (std::string(argv[1]) == "-h"){
            dispatch.help(); // help flag passed
        }else{
            dispatch.parse(argv[1]); // default flag, passed in the file name
        }

        return 1;
    } else if (argc > 3){
        // we are allowed to handle flags mutually exclusive... if we do ./434fe -s <name> -p, i give it help flag
        dispatch.help(); // help flag passed

        return 1;
    } else if (argc == 3 && argv[1][0] != '-') {
        // assume it is like ./434alloc [k] [filename]

        dispatch.allocate(std::stoi(argv[1]), argv[2]);

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

        case 'x':
            dispatch.rename(argv[2]);
            break;

        default:
            std::cerr << "Improper flag used, please use '434fe -h'\n";
    }
}