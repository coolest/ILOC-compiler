#ifndef GRAPH
#define GRAPH

#include "ir.hpp"
#include "allocator.hpp"
#include <unordered_map>

class Scheduler;

enum EdgeType {
    EdgeTypeData, EdgeTypeSerial, EdgeTypeConflict
};

struct Edge;

struct Node {
    IR *op;
    int delay;
    int priority = 0;
    int remaining_dependencies = 0;
    int finish = INT_MAX;

    std::vector<Edge> successors; // Operations that I/ME DEPEND ON
    std::vector<Edge> predecessors; // Operations that DEPEND ON ME
};

struct Edge {
    Node *source;
    Node *target;
    EdgeType type; // Data, Serial, Conflict

    int regNumber;
    int latency;
};

class DependenceGraph {
    friend class Scheduler;

    private:
    std::vector<Node*> nodes;
    std::unordered_map<int, Node*> register_map;
    std::vector<Node*> memory_ops;

    public:
    DependenceGraph();
    std::unique_ptr<IR_NodePool> build_graph(std::unique_ptr<IR_NodePool> ir_head);
    void add_node(IR &ir);
    void add_edge(Node* source, Node* target, EdgeType type, int regNumber, int delay);
    bool is_mem_op(IR_OP_CODE &code);
    int get_op_delay(IR_OP_CODE &code, IR_ARITH_OP &op);
};

#endif