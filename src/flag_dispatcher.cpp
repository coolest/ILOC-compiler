
#include "flag_dispatcher.hpp"
#include "scanner.hpp"
#include "parser.hpp"
#include "token.hpp"
#include "allocator.hpp"
#include <string>
#include <iostream>
#include <chrono>
#include <cmath>

std::string calculate_throughput(int bytes, double seconds){
    std::string suffix = " B/s";
    double data_per_second = (double) bytes / seconds;
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

    return std::to_string(data_per_second) + suffix;
}

void FlagDispatch::help(){
    std::cout 
        << "434alloc -h:\n\t* Prints interactive flags for 434fe\n"
        << "434alloc -s <name>:\n\t* Prints, to the standard output stream, a list of the tokens that the scanner found.\n"
        << "434alloc -p <name> [DEFAULT FLAG]:\n\t* Scans it and parses it, builds the intermediate representation, and reports either success or all the errors that it finds in the input file.\n"
        << "434alloc -r <name>:\n\t* Scans it, parses it, builds the intermediate representation, and prints out the information in the intermediate representation (in an appropriately human readable format).\n"
        << "434alloc -x <name>:\n\t* Scans it, parses it, builds IR, then performs a renaming pass and builds the IR with the renaming algorithm performed in std::out. \n\n"
        << "434alloc <flag> IS MUTAUALLY EXCLUSIVE... if more than one flag is passed, you are taken here.\n";
};

void FlagDispatch::scan(const std::string &filename){
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
    // Debug mode doesn't exist yet, will add functionality with -d flag, but too hard to deal with all of flags etc.
    if (1){
        perform(true);
        std::cout << std::endl;
    }

    // Performance:
    auto start = std::chrono::high_resolution_clock::now();

    int bytes_read = perform(false);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    double seconds = (double) duration.count() / (double) 1000000;
    std::cout << std::endl;
    std::cout << "Takes: " << seconds  << " s" << std::endl;

    // Pretty print data/s
    const std::string throughput = calculate_throughput(bytes_read, seconds);
    std::cout << "Approx: " << throughput << std::endl;
};

void FlagDispatch::parse(const std::string &filename){
    // Performance:
    auto start = std::chrono::high_resolution_clock::now();

    Parser parser(filename);
    std::unique_ptr<IR_NodePool> ir_head = parser.parse(); // Maybe do something with this .. ?

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Print parse statistics and performance
    // Parser prints errors as it parses (i.e Line X: Expected A but got B)
    std::cout << parser.stats.errors << " errors found." << std::endl << std::endl;

    double seconds = (double) duration.count() / (double) 1000000;
    std::cout << "Takes: " << seconds << "s" << std::endl;
    
    const std::string throughput = calculate_throughput(parser.scanner.bytes_read, seconds);
    std::cout << "Approx: " << throughput << std::endl;
};

const std::string ir_op_code_strings[NUM_OP_CODES] = 
    { "IR_ERROR", "IR_LOAD", "IR_STORE", "IR_ARITHOP", "IR_LOADI", "IR_NOP", "IR_OUTPUT" };
const std::string ir_field_strings[NUM_IR_FIELD] = 
    { "SR", "VR", "PR", "NE" };

void print_ir(const IR &ir, const int &line){
    std::cout << ir_op_code_strings[ir.op_code] << " [line " << line << "] ";
    if (ir.op_code == IR_OP_CODE::IR_ERROR){
        std::cout << "( " << ir.error_lexeme << " )" << std::endl;
    } else {
        for (int i = 0; i < 3; i++){
            std::cout << std::endl << "arg" << i+1 << ": [" 
                << ir.args[i][0] << ", "
                << ir.args[i][1] << ", "
                << ir.args[i][2] << ", "
                << ir.args[i][3] << " ]";
        }

        std::cout << std::endl << std::endl;
    }
}

void print_all_ir(std::unique_ptr<IR_NodePool> ir) {
    IR_NodePool* ir_head = ir.get();

    while (ir_head){
        for (int i = 0; i < IR_NodePool::POOL_SIZE; i++){
            IR_Node node = ir_head->pool[i];
            if (node.line == 0){
                break; // End
            }

            print_ir(node.ir, node.line);
        }

        ir_head = ir_head->next;
    }
}

void FlagDispatch::read(const std::string &filename){
    // Mainly for printing out the IR, not going to profile

    Parser parser(filename);
    std::unique_ptr<IR_NodePool> ir = parser.parse();

    print_all_ir(std::move(ir));
};

// LAB 2

const std::string arith_op_strings[] = {"add", "sub", "mult", "lshift", "rshift"};
std::string format_register(int reg) {
    return "r" + std::to_string(reg);
}

void FlagDispatch::rename(const std::string &filename) {
    Parser parser(filename);

    std::unique_ptr<IR_NodePool> ir_nodepool =
        Allocator::rename( // Perform renaming pass
            parser.parse() // Parse into an IR.
        );

    IR_NodePool* ir_raw_nodepool = ir_nodepool.get();
    while (ir_raw_nodepool) {
        for (int i = 0; i < ir_raw_nodepool->i; i++){
            IR &ir = ir_raw_nodepool->pool[i].ir;

            switch(ir.op_code) {
                case IR_OP_CODE::IR_ARITHOP:
                    std::cout << arith_op_strings[ir.arith_op] << " "
                            << format_register(ir.args[0][IR_FIELD::SR]) << ", "
                            << format_register(ir.args[1][IR_FIELD::SR]) << " => "
                            << format_register(ir.args[2][IR_FIELD::SR])
                            << " // vr" << ir.args[0][IR_FIELD::VR] << ", vr"
                            << ir.args[1][IR_FIELD::VR] << " => vr"
                            << ir.args[2][IR_FIELD::VR];

                    break;
                
                case IR_OP_CODE::IR_LOAD:
                    std::cout << "load " << format_register(ir.args[0][IR_FIELD::SR])
                            << " => " << format_register(ir.args[1][IR_FIELD::SR])
                            << " // vr" << ir.args[0][IR_FIELD::VR]
                            << " => vr" << ir.args[1][IR_FIELD::VR];
                            
                    break;
                    
                case IR_OP_CODE::IR_STORE:
                    std::cout << "store " << format_register(ir.args[0][IR_FIELD::SR])
                            << " => " << format_register(ir.args[1][IR_FIELD::SR]);

                    if (ir.args[1][IR_FIELD::VR] == 0) {
                        std::cout << " // vr" << ir.args[0][IR_FIELD::VR]
                                << " => Mem[vr" << ir.args[1][IR_FIELD::SR] << "]";
                    } else {
                        std::cout << " // vr" << ir.args[0][IR_FIELD::VR]
                                << " => vr" << ir.args[1][IR_FIELD::VR];
                    }

                    break;
                    
                case IR_OP_CODE::IR_LOADI:
                    std::cout << "loadI " << ir.args[0][IR_FIELD::SR]
                            << " => " << format_register(ir.args[1][IR_FIELD::SR]);

                    if (ir.args[1][IR_FIELD::PR] != 0) {
                        std::cout << " // rematerialize vr" << ir.args[1][IR_FIELD::VR]
                                << " => pr" << ir.args[1][IR_FIELD::PR];
                    } else {
                        std::cout << " // " << ir.args[0][IR_FIELD::SR]
                                << " => vr" << ir.args[1][IR_FIELD::VR];
                    }

                    break;
                    
                case IR_OP_CODE::IR_OUTPUT:
                    std::cout << "output " << ir.args[0][IR_FIELD::SR]
                            << " // as in the input";

                    break;
                    
                default:
                    break;

            }

            std::cout << std::endl;
        }

        ir_raw_nodepool = ir_raw_nodepool->next;
    }
}