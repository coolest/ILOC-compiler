#include "optimizer.hpp"

std::unique_ptr<IR_NodePool> Optimizer::rename(std::unique_ptr<IR_NodePool> ir) {
    return std::move(ir);
}