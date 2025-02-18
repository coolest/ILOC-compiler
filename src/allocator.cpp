#include "allocator.hpp"

constexpr int INVALID = -1;
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

    // ir is now NULLPTR

    std::unordered_map<int, int> sr_to_vr;
    std::unordered_map<int, int> last_use;
    int vr_name = 0;

    // Loop through NodePools right to left
    while (ir) {
        // Loop through the node pool
        for (int i = ir->i; i >= 0; i--, index--){
            IR &node = ir->pool[i].ir;

            bool is_def[3] = {false, false, false};
            bool is_use[2] = {false, false}; // Never use r3

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

            // As per the algorithm:

            // Defines
            for (int arg_num = 0; arg_num < 3; arg_num++) {
                if (!is_def[arg_num]) {
                    continue;
                }

                int sr = node.args[arg_num][IR_FIELD::SR];
                if (!sr_to_vr.count(sr) || sr_to_vr[sr] == INVALID) {
                    sr_to_vr[sr] = vr_name++;
                }

                node.args[arg_num][IR_FIELD::VR] = sr_to_vr[sr];
                node.args[arg_num][IR_FIELD::NE] = last_use[sr];

                last_use[sr] = INT_MAX;
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
        }

        ir = ir->prev;
    }

    return std::move(ir_head);
}