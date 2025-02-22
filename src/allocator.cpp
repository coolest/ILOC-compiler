#include "allocator.hpp"

Allocator::Allocator() {

}

void populate_behavior(bool is_def[], bool is_use[], IR& node){
    switch (node.op_code) {
        case IR_OP_CODE::IR_LOAD:
            is_use[0] = true;   // r1 use
            is_def[1] = true;   // r2 def

            break;

        case IR_OP_CODE::IR_STORE:
            is_use[0] = true;   // r1 use
            is_use[1] = true;   // r2 use

            break;

        case IR_OP_CODE::IR_LOADI:
            is_def[1] = true;   // r2 def

            break;

        case IR_OP_CODE::IR_ARITHOP:
            is_use[0] = true;   // r1 use
            is_use[1] = true;   // r2 use
            is_def[2] = true;   // r3 def

            break;

        default:
            break;
    }
}

constexpr int INVALID = -1;
constexpr int NEVER_USED = -1;
std::unique_ptr<IR_NodePool> Allocator::rename(std::unique_ptr<IR_NodePool> ir_head) {
    // Start from the back
    int index = 0; // Global index for last_use
    IR_NodePool* ir = ir_head.get();
    while (1) {
        index += ir->i;
        if (!ir->next){
            break;
        }

        ir = ir->next;
    }

    std::unordered_map<int, int> sr_to_vr;
    std::unordered_map<int, int> last_use;
    int vr_name = 0;
    
    size_t max_active = 0;
    size_t active = 0;

    // Loop through NodePools right to left
    while (ir) {
        // Loop through the node pool
        for (int i = ir->i-1; i >= 0; i--, index--){
            IR &node = ir->pool[i].ir;

            bool is_def[3] = {false, false, false};
            bool is_use[2] = {false, false}; // Never use r3

            populate_behavior(is_def, is_use, node);

            // As per the algorithm:

            // Defines
            for (int arg_num = 0; arg_num < 3; arg_num++) {
                if (!is_def[arg_num]) {
                    continue;
                }

                int sr = node.args[arg_num][IR_FIELD::SR];
                if (!sr_to_vr.count(sr) || sr_to_vr[sr] == INVALID) {
                    sr_to_vr[sr] = vr_name++;
                } else {
                    active--;
                }

                node.args[arg_num][IR_FIELD::VR] = sr_to_vr[sr];
                node.args[arg_num][IR_FIELD::NE] = last_use[sr];

                last_use[sr] = NEVER_USED;
                sr_to_vr[sr] = INVALID;
            }

            // Uses
            for (int arg_num = 0; arg_num < 2; arg_num++) {
                if (!is_use[arg_num]) {
                    continue;
                }

                int sr = node.args[arg_num][IR_FIELD::SR];
                if (!sr_to_vr.count(sr) || sr_to_vr[sr] == INVALID) {
                    sr_to_vr[sr] = vr_name++;

                    active++;
                }

                node.args[arg_num][IR_FIELD::VR] = sr_to_vr[sr];
                node.args[arg_num][IR_FIELD::NE] = last_use[sr];
            }

            // Last Uses
            for (int arg_num = 0; arg_num < 2; arg_num++) {
                if (!is_use[arg_num]) {
                    continue;
                }

                int sr = node.args[arg_num][IR_FIELD::SR];
                last_use[sr] = index; // global index here
            }

            max_active = std::max(max_active, active);
        }

        ir = ir->prev;
    }

    // Update live range (or max of live range)
    live_range = max_active;

    return std::move(ir_head);
}

// #include <iostream>
std::unique_ptr<IR_NodePool> Allocator::allocate(std::unique_ptr<IR_NodePool> ir_head, int k) {
    IR_NodePool* ir = ir_head.get();

    //std::cout << live_range << std::endl;
    // Keep k-1 register for spilling, if needed
    const int allocate_regs = k >= live_range ? k : k-1;
    //std::cout << allocate_regs << std::endl;
    std::stack<int> free_prs;
    std::vector<int> pr_to_vr(allocate_regs, -1);
    std::vector<int> pr_nu(allocate_regs, 0);
    // vr can be large (maybe vector still better...)
    std::unordered_map<int, int> vr_to_pr;
    std::unordered_map<int, int> vr_to_spill;

    // initialize stack
    for (int i = allocate_regs-1; i >= 0; i--){
        free_prs.push(i);
    }

    int next_spill_location = 65536; // default spill

    // spill & restore code, and adds the instructions in IR_Extra pointer (so no need to edit array)
    // note:
    //  allocates k registers if spilling is not required, otherwise k-1, and k-1 is never used in pr_to_vr
    //  or pr_nu/etc since it is desginated as a reversed register for spilling.
    auto spill = [&](IR_Node &node, int pr, int mem_loc) {
        if (!node.extra) {
            node.extra = std::make_unique<IR_Extra>();
        }
        
        // Load the next spill memory location into reserved register...
        IR addr_load(IR_OP_CODE::IR_LOADI);
        addr_load.args[0][IR_FIELD::SR] = mem_loc;
        addr_load.args[1][IR_FIELD::PR] = k-1;
        
        // Store the PR into the address stored in the reserved register...
        IR store(IR_OP_CODE::IR_STORE);
        store.args[0][IR_FIELD::PR] = pr;
        store.args[1][IR_FIELD::PR] = k-1;

        // Now, should be able to overwrite PR, since we spilled PR into memory...

        node.extra->before.push_back(addr_load);
        node.extra->before.push_back(store);
    };

    auto restore = [&](IR_Node &node, int pr, int mem_loc) {
        if (!node.extra) {
            node.extra = std::make_unique<IR_Extra>();
        }

        // Load the spilled address into the reserved register...
        IR addr_load(IR_OP_CODE::IR_LOADI);
        addr_load.args[0][IR_FIELD::SR] = mem_loc;
        addr_load.args[1][IR_FIELD::PR] = k-1;

        // Load the value of address in reserved regitser INTO the PR given... (unspills/restores)
        IR load(IR_OP_CODE::IR_LOAD);
        load.args[0][IR_FIELD::PR] = k-1;
        load.args[1][IR_FIELD::PR] = pr;

        // Now, should be able to use the value since it is given a PR...
        
        node.extra->before.push_back(addr_load);
        node.extra->before.push_back(load);
    };

    int idx = 0; // global idx (line count)
    while (ir) {
        for (int i = 0; i < ir->i; i++, idx++){
            IR& node = ir->pool[i].ir;

            bool is_def[3] = {false, false, false};
            bool is_use[2] = {false, false}; // Never use r3

            populate_behavior(is_def, is_use, node);

            // As per the algorithm:

            // Not in psuedocode but needed ??
            // O(K)... free all non-used registers...
            for (int pr = 0; pr < allocate_regs; pr++) {
                int vr = pr_to_vr[pr];
                if (vr < 0) {
                    continue; // no VR in this PR
                }
                
                int nuse = pr_nu[pr];
                if (nuse < idx) {
                    pr_to_vr[pr] = -1;
                    pr_nu[pr] = 0;
                    vr_to_pr[vr] = -1;
                    free_prs.push(pr);
                }
            }

            // Uses..
            for (int arg_num = 0; arg_num < 2; arg_num++){
                int vr = node.args[arg_num][IR_FIELD::VR];

                // if we ARENT using it, OR there exists a valid physical register already...
                if (!is_use[arg_num]) {
                    continue;
                }

                if (vr_to_pr.count(vr) && vr_to_pr[vr] >= 0) {
                    // VR already has a PR mapping, use it
                    node.args[arg_num][IR_FIELD::PR] = vr_to_pr[vr]; // this costed me like 4 hours...

                    continue;
                }

                // Already assigned a physical register?
                if (vr_to_pr.count(vr) && vr_to_pr[vr] >= 0) {
                    int pr = vr_to_pr[vr];
                    node.args[arg_num][IR_FIELD::PR] = pr;

                    // Update pr_nu if the IR says a new next use
                    pr_nu[pr] = node.args[arg_num][IR_FIELD::NE];

                    continue;
                } 

                // If we have FREE physical registers..
                if (!free_prs.empty()){
                    // assign physical register...
                    int pr = free_prs.top();
                    free_prs.pop();

                    //std::cout << "pop " << pr << " for use" << std::endl;

                    // might be spilled, but we have free register so unspill...
                    if (vr_to_spill.count(vr) && vr_to_spill[vr] >= 0) {
                        restore(ir->pool[i], pr, vr_to_spill[vr]);

                        vr_to_spill[vr] = -1;
                    }

                    // necessary mappings/information for setting the VR to PR
                    vr_to_pr[vr] = pr;
                    pr_to_vr[pr] = vr;

                    node.args[arg_num][IR_FIELD::PR] = pr;

                    // specify when this register is NEXT USED...
                    pr_nu[pr] = node.args[arg_num][IR_FIELD::NE];
                } else {
                    // otherwise... we need to SPILL an existing register...
                    int max_val = pr_nu[0];
                    int pr = 0;

                    for (int i = 1; i < allocate_regs; i++){
                        int val = pr_nu[i];
                        int val_for_compare = (val == -1) ? INT_MAX : val;
                        if (val_for_compare <= max_val){
                            continue;
                        }

                        pr = i;
                        max_val = val_for_compare;
                    }

                    // spill the node decided on...
                    //std::cout << "spill " << pr << " for use" << std::endl;
                    spill(ir->pool[i], pr, next_spill_location);

                    // unassign the mappings to that node...
                    int victim_vr = pr_to_vr[pr];
                    vr_to_spill[victim_vr] = next_spill_location;
                    next_spill_location += 4;

                    vr_to_pr[victim_vr] = -1;
                    vr_to_pr[vr] = pr;
                    pr_to_vr[pr] = vr;

                    node.args[arg_num][IR_FIELD::PR] = pr;
                    pr_nu[pr] = node.args[arg_num][IR_FIELD::NE];
                }
            }

            // Defs..
            for (int arg_num = 0; arg_num < 3; arg_num++){
                if (!is_def[arg_num] || node.args[arg_num][IR_FIELD::NE] <= idx){ // If we are defining it... and it is used somewhere..
                    continue;
                }

                int vr = node.args[arg_num][IR_FIELD::VR];
                if (!free_prs.empty()){ // If we have FREE registers...
                    // Pop and give them to the VR we are defining..
                    int pr = free_prs.top();
                    free_prs.pop();
                    
                    //std::cout << "pop " << pr << " for def" << std::endl;

                    // might be spilled, but we have free register so unspill...
                    if (vr_to_spill.count(vr) && vr_to_spill[vr] >= 0) {
                        restore(ir->pool[i], pr, vr_to_spill[vr]);

                        vr_to_spill[vr] = -1;
                    }

                    vr_to_pr[vr] = pr;
                    pr_to_vr[pr] = vr;

                    node.args[arg_num][IR_FIELD::PR] = pr;

                    pr_nu[pr] = node.args[arg_num][IR_FIELD::NE];
                } else {
                    // No register... :(
                    // We have to spill one not used...

                    // like old algorithm... (copy and paste basically... could abstract into a singular function... but would make it more complex IMO)
                    int max_val = pr_nu[0];
                    int pr = 0;

                    for (int i = 1; i < allocate_regs; i++){
                        int val = pr_nu[i];
                        int val_for_compare = (val == -1) ? INT_MAX : val;
                        if (val_for_compare <= max_val){
                            continue;
                        }

                        pr = i;
                        max_val = val_for_compare;
                    }

                    //std::cout << "spill " << pr << " for def" << std::endl;
                    spill(ir->pool[i], pr, next_spill_location);

                    int victim_vr = pr_to_vr[pr];
                    vr_to_spill[victim_vr] = next_spill_location;
                    next_spill_location += 4;

                    vr_to_pr[victim_vr] = -1;
                    vr_to_pr[vr] = pr;
                    pr_to_vr[pr] = vr;

                    node.args[arg_num][IR_FIELD::PR] = pr;
                    pr_nu[pr] = node.args[arg_num][IR_FIELD::NE];
                }
            }
        }

        ir = ir->next;
    }

    // Give back ownership
    return std::move(ir_head);
}