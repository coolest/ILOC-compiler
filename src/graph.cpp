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
    
    target->remaining_dependencies++; // one more dependency
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

            add_edge(register_map[vr], node, EdgeTypeData, vr, register_map[vr]->delay);
        }
    }

    // MEMORY EDGES
    Node* last_store = nullptr;
    Node* last_output = nullptr;
    std::vector<Node*> loads_since_last_store;
    
    for (Node* memop : memory_ops) {
        IR_OP_CODE op_code = memop->op->op_code;
        
        switch (op_code){
            case IR_LOAD: {
                if (last_store != nullptr) {
                    add_edge(last_store, memop, EdgeTypeConflict, -1, last_store->delay);
                }
                
                loads_since_last_store.push_back(memop);
                break;
            }

            case IR_STORE: {
                if (last_store != nullptr) {
                    add_edge(last_store, memop, EdgeTypeSerial, -1, last_store->delay);
                }
                
                for (Node* load : loads_since_last_store) {
                    add_edge(load, memop, EdgeTypeSerial, -1, load->delay);
                }
                
                last_store = memop;
                loads_since_last_store.clear();
                break;
            }

            case IR_OUTPUT: {
                if (last_store != nullptr) {
                    add_edge(last_store, memop, EdgeTypeConflict, -1, last_store->delay);
                }
                
                if (last_output != nullptr) {
                    add_edge(last_output, memop, EdgeTypeSerial, -1, last_output->delay);
                }
                
                loads_since_last_store.push_back(memop);
                last_output = memop;
                break;
            }
        }
    }

    // give back ownership
    return ir_head;
}