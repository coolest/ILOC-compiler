#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "graph.hpp"
#include <queue>

class Scheduler {
    private:
    DependenceGraph* g;
    int f0_busy_until;
    int f1_busy_until;
    std::unordered_map<Node*, int> descendants_cache;

    int count_descendants(Node* root);
    void compute_descendants();
    std::vector<Node*> topological_sort();
    void compute_priorities();
    bool needs_f1(Node* node);

    public:
    Scheduler(DependenceGraph* g);
    std::vector<std::vector<Node*>> schedule();
};

#endif