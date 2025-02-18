#include "allocator.hpp"
#include <vector>

std::unique_ptr<IR_NodePool> Allocator::rename(std::unique_ptr<IR_NodePool> ir_head) {
    IR_NodePool* ir = ir_head.get();
    while (ir->next){
        ir = ir->next;
    }

    std::vector<int> sr_to_vr;
    std::vector<int> last_use;

    // Loop through NodePools right to left
    while (ir) {
        // Loop through the node pool
        for (int i = ir->i; i >= 0; i--){
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

            // Defs:
            for (int j = 0; j < 3; j++) {
                if (is_def[j]) {
                    // def logic
                }
            }

            // Uses:
            for (int j = 0; j < 2; j++) {
                if (is_use[j]) {
                    // use logic
                }
            }

            // LastUse:
            for (int j = 0; j < 2; j++) {
                if (is_use[j]) {
                    // update last use
                }
            }
        }

        ir = ir->prev;
    }

    return std::move(ir_head);
}