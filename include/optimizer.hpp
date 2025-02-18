#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include <ir.hpp>
#include <memory>

struct Optimizer {
    // Takes ownership of the IR for optimization then gives back ownership
    static std::unique_ptr<IR_NodePool> rename(std::unique_ptr<IR_NodePool> ir);
};

#endif