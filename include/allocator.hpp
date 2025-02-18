#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP

#include <ir.hpp>
#include <memory>

struct Allocator {
    // Takes ownership of the IR for optimization then gives back ownership
    static std::unique_ptr<IR_NodePool> rename(std::unique_ptr<IR_NodePool> ir);
};

#endif