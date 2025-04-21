#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP

#include <ir.hpp>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <stack>

struct Allocator {
    int live_range;

    // Takes ownership of the IR for optimization then gives back ownership
    std::unique_ptr<IR_NodePool> rename(std::unique_ptr<IR_NodePool> ir);
    std::unique_ptr<IR_NodePool> allocate(std::unique_ptr<IR_NodePool> ir, int k);

    Allocator();
};

void populate_behavior(bool is_def[], bool is_use[], IR& node);

#endif