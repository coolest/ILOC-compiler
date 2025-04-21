#include "graph.hpp"

DependenceGraph::DependenceGraph(){

}

int DependenceGraph::get_op_delay(IR_OP_CODE &code, IR_ARITH_OP &op){
    switch (code){
        case IR_LOAD:
        case IR_STORE:
            return 5;
        case IR_ARITHOP:
            if (op == IR_MULT){
                return 3;
            }
        default:
            return 1;
    }
}

bool DependenceGraph::is_mem_op(IR_OP_CODE &code){
    return code == IR_OUTPUT || code == IR_STORE || code == IR_LOAD;
}

void DependenceGraph::add_edge(Node* source, Node* target, EdgeType type, int regNumber, int delay){
    Edge edge;
    edge.source = source;
    edge.target = target;
    edge.type = type;
    edge.regNumber = regNumber; // -1 = not a register dependency, but a conflict/serialization edge
    edge.latency = delay;
    
    source->successors.push_back(edge);
    source->remaining_dependencies++; // one more dependency

    target->predecessors.push_back(edge);
}

void DependenceGraph::add_node(IR &ir){
    Node* node = new Node();
    node->op = &ir;
    node->delay = get_op_delay(ir.op_code, ir.arith_op);
    nodes.push_back(node);
    
    if (is_mem_op(ir.op_code)) {
        memory_ops.push_back(node);
    }
}

std::unique_ptr<IR_NodePool> DependenceGraph::build_graph(std::unique_ptr<IR_NodePool> ir_head){
    /*
        Going over the details, so I don't get them incorrect when programming it-
        
        First, ADD nodes to our graph by traversing the IR_NodePool
            1. Get a IR_Node, check IR_Extra before then add those first, add existing node, add IR_Extra after nodes.

        (Hard part):
        Second, ADD edges to our graph for dependencies:
            1. If OP defines VRi, set register_map[VRi] = NODEi (where NODEi is the corresponding Node created in step 1).
            2. If VRj is used in NODEiOP, add an edge from NODEi to register_map[VRj].
                2a. add_edge(NODE_WHERE_ARE_AT, register_map[VRj], etc, etc)
        
        (Super hard part):
        Context: We track the LAST store/output, because we are building the dependencies ONE LAYER AT A TIME.
        Context2: BE CONSERVATIVE, Assume all addresses are same when adding these edges!
        Third, MEMOP edges.
            1. load/output BEFORE store-> assume same address, 
                serialize edge BECAUSE: we want to read before we write that address
            2. store BEFORE store-> assume same address
                serialize edge BECAUSE: we need to ensure after both stores execute address has expected value, and in between these two edges, etc.
            3. store BEFORE load/output -> assume same address,
                conflict/serialzie edge BECAUSE: we need to ensure we write the correct value before reading it, otherwise we may output/store a wrong value
    */

    // ADD NODES HERE
    IR_NodePool* temp = ir_head.get();
    while (temp != nullptr){
        int end = temp->i;
        for (int i = 0; i < end; i++){
            IR_Node &node = temp->pool[i];

            if (node.extra != nullptr){
                for (IR &ir : node.extra->before){
                    add_node(ir);
                }
            }

            add_node(node.ir);

            if (node.extra != nullptr){
                for (IR &ir : node.extra->after){
                    add_node(ir);
                }
            }
        }

        temp = temp->next;
    }

    // ADD EDGES HERE
    for (Node* node : nodes) {
        IR* op = node->op;
        if (op == nullptr){
            continue;
        }

        bool is_def[3] = {false, false, false};
        bool is_use[2] = {false, false}; // Never use r3
        populate_behavior(is_def, is_use, *op);

        // Defines
        for (int i = 0; i < 3; i++){
            if (!is_def[i]){
                continue;
            }
            
            int vr = op->args[i][IR_FIELD::VR];
            register_map[vr] = node;
        }

        // Uses
        for (int i = 0; i < 2; i++){
            if (!is_use[i]){
                continue;
            }
            
            int vr = op->args[i][IR_FIELD::VR];
            if (register_map.find(vr) == register_map.end()){
                continue; // USE BEFORE DEFINE (???)
            }

            add_edge(node, register_map[vr], EdgeTypeData, vr, register_map[vr]->delay);
        }
    }

    /*
        Context 2:
        LOADS & STORES need to be in order- so if a LOAD comes around, we need to get the last store and make an edge
        STORES & LOADS need to be in order- so if a STORE comes around, we need to get all of the previous loads that happened until the last store and connect them.
        ^^ The reason we have an array is because LOADS & LOADS can be out of order, but not STORES & STORES, so we always update the stores, but not the loads.
        STORES & STORES need to be in order- so if a STORE comes around, we need to connect the edges between the two stores to ensure proper order
        OUTPUTS - if an OUTPUT comes we need to connect output with the LAST store&output AND THEN push the output in the array of loads (loads/outputs are the same in this sense as they are a READ)
    */

    // MEMORY EDGES
    Node* last_store = nullptr;
    Node* last_output = nullptr;
    std::vector<Node*> loads_since_last_store;
    
    for (Node* memop : memory_ops) {
        IR_OP_CODE op_code = memop->op->op_code;
        
        switch (op_code){
            case IR_LOAD: {
                if (last_store != nullptr) {
                    add_edge(memop, last_store, EdgeTypeConflict, -1, last_store->delay);
                }
                
                loads_since_last_store.push_back(memop);
                break;
            }

            case IR_STORE: {
                if (last_store != nullptr) {
                    add_edge(memop, last_store, EdgeTypeSerial, -1, 1);
                }
                
                for (Node* load : loads_since_last_store) {
                    add_edge(memop, load, EdgeTypeSerial, -1, 1);
                }
                
                last_store = memop;
                loads_since_last_store.clear();
                break;
            }

            case IR_OUTPUT: {
                if (last_store != nullptr) {
                    add_edge(memop, last_store, EdgeTypeConflict, -1, last_store->delay);
                }
                
                if (last_output != nullptr) {
                    add_edge(memop, last_output, EdgeTypeSerial, -1, 1);
                }
                
                loads_since_last_store.push_back(memop);
                last_output = memop;
                break;
            }

            default:
        }
    }

    // give back ownership
    return ir_head;
}