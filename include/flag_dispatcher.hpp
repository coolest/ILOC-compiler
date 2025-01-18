#ifndef FLAG_DISPATCH_HPP
#define FLAG_DISPATCH_HPP

#include <string>

struct FlagDispatch{
    void help();
    void scan(const std::string &filename);
    void parse(const std::string &filename);
    void read(const std::string &filename);
};

#endif