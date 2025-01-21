#ifndef FLAG_DISPATCH_HPP
#define FLAG_DISPATCH_HPP

#include <string>

struct FlagDispatch{
    bool debug_mode = false;

    void help();
    void scan(const std::string &filename);
    void parse(const std::string &filename);
    void read(const std::string &filename);
};

#endif