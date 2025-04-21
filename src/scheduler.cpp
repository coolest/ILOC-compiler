#include "scheduler.hpp"

Scheduler::Scheduler(DependenceGraph* g) : g{g}, f0_busy_until{0}, f1_busy_until{0} {

}

#include <iostream>
using namespace std;

std::vector<Node*> Scheduler::topological_sort() {
    std::vector<Node*> result;
    std::unordered_set<Node*> visited;
    std::unordered_set<Node*> temp_visited;
    
    std::function<void(Node*)> visit = [&](Node* node) {
        if (visited.find(node) != visited.end()) return;
        if (temp_visited.find(node) != temp_visited.end()) return;
        
        temp_visited.insert(node);
        
        for (Edge& e : node->successors) {
            visit(e.target);
        }
        
        temp_visited.erase(node);
        visited.insert(node);
        result.push_back(node);
    };
    
    for (Node* node : g->nodes) {
        if (visited.find(node) == visited.end()) {
            visit(node);
        }
    }
    
    std::reverse(result.begin(), result.end());
    return result;
}

void Scheduler::compute_descendants() {
    descendants_cache.clear();
    
    std::vector<Node*> topo_order = topological_sort();
    std::reverse(topo_order.begin(), topo_order.end());
    
    for (Node* node : topo_order) {
        int count = 1;

        for (Edge& e : node->successors) {
            count += descendants_cache[e.target];
        }
        
        descendants_cache[node] = count;
    }
}

int Scheduler::count_descendants(Node* node) {
    if (descendants_cache.empty()) {
        compute_descendants();
    }
    
    return descendants_cache[node] - 1;
}

bool only_waiting_for_control_deps(Node* node, int cycle) {
    for (Edge &e : node->successors){
        if (e.type == EdgeTypeData && (e.target->finish >= cycle)){
            return false;
        }
    }

    return true;
}

bool Scheduler::needs_f1(Node* node) {
    IR_OP_CODE op_code = node->op->op_code;
    
    if (op_code == IR_ARITHOP && node->op->arith_op == IR_MULT) {
        return true; // Needs f1
    }
    
    if (op_code == IR_LOAD || op_code == IR_STORE) {
        return false; // Needs f0
    }
    
    // Heuristic for choosing f0 or f1
    return f0_busy_until > f1_busy_until;
}

void Scheduler::compute_priorities(){
    bool changed = true;

    while (changed) {
        changed = false;

        for (Node* node : g->nodes){
            int max_path = 0;
            for (Edge &e : node->successors){
                Node* successor = e.target;

                int path_len = successor->priority + e.latency;
                max_path = std::max(path_len, max_path);
            }

            if (max_path > node->priority){
                node->priority = max_path;
                changed = true;
            }
        }
    }

    cout << "cccc" << endl;

    for (Node* node : g->nodes){
        node->priority = 10 * node->priority + count_descendants(node);
    }


    cout << "dddd" << endl;
}

std::vector<std::vector<Node*>> Scheduler::schedule(){
        cout << "bbbb" << endl;
    auto compare = [&](Node* a, Node* b){
        return a->priority > b->priority;
    };

    std::priority_queue<Node*, std::vector<Node*>, decltype(compare)> ready(compare);
    std::unordered_set<Node*> active;
    std::vector<std::vector<Node*>> schedule;

    compute_priorities();
    for (Node* node : g->nodes){
        if (node->successors.size() == 0){
            continue;
        }

        ready.push(node);
    }

    int cycle = 0;
    while (!ready.empty() || !active.empty()){
        schedule.push_back({nullptr, nullptr}); // [op0, op1]

        std::queue<Node*> q;

        cout << cycle << endl;
        int tries = 7; // don't try all readies...
        while (tries-- >= 0 && !ready.empty() && (f0_busy_until <= cycle || f1_busy_until <= cycle)){
            Node* node = ready.top();
            ready.pop();

            if (needs_f1(node)){
                if (f1_busy_until > cycle){
                    q.push(node); // cannot schedule
                } else {
                    schedule.back()[1] = node;

                    node->finish = cycle + node->delay;
                    f1_busy_until = node->finish;
                    active.insert(node);
                }
            } else {
                if (f0_busy_until > cycle){
                    q.push(node); // cannot schedule
                } else {
                    schedule.back()[0] = node;

                    node->finish = cycle + node->delay;
                    f0_busy_until = node->finish;
                    active.insert(node);
                }
            }
        }

        while (!q.empty()){
            ready.push(q.front());
            q.pop();
        }

        cycle++;        
        cout << cycle << endl;
        for (Node* node : active){
            if (node->finish >= cycle){
                continue;
            }

            // completed;
            active.erase(node);
            for (Edge &e : node->predecessors){
                Node* dependent = e.source;
                dependent->remaining_dependencies--;

                if (dependent->remaining_dependencies == 0){
                    ready.push(dependent);
                }
            }
        }

        for (Node* node : active){
            for (Edge &e : node->predecessors){
                Node* dependent = e.source;
                dependent->remaining_dependencies--;

                if (only_waiting_for_control_deps(dependent, cycle)){
                    ready.push(dependent);
                }
            }
        }
    }

    return schedule;
}