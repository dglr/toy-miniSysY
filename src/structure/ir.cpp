//
// Created by dglr on 2021/10/30.
//
#include "ir.h"

#include <unordered_map>

#include "ast.h"



std::unordered_map<i32, ConstValue *> ConstValue::POOL;



// print value according to type
struct pv {
    IndexMapper<Value> &v_index;
    Value *v;

    pv(IndexMapper<Value> &v_index, Value *v) : v_index(v_index), v(v) {}

    friend std::ostream &operator<<(std::ostream &os, const pv &pv) {
        if (auto x = dyn_cast<ConstValue>(pv.v)) {
            os << x->imm;
        }  else {
            os << "%x" << pv.v_index.get(pv.v);
        }
        return os;
    }
};

// output IR
std::ostream &operator<<(std::ostream &os, const IrProgram &p) {
    using std::endl;


    for (auto f = p.func.head; f != nullptr; f = f->next) {
        const char *decl = "define";
        const char *ret = f->func->is_int ? "i32" : "void";
        os << decl << " " << ret << " @";
        os << f->func->name << "(";
        os << ")";

        os << " {" << endl;



        // bb的标号没有必要用IndexMapper，而且IndexMapper的编号是先到先得，这看着并不是很舒服
        std::map<BasicBlock *, u32> bb_index;
        IndexMapper<Value> v_index;
        for (auto bb = f->bb.head; bb; bb = bb->next) {
            u32 idx = bb_index.size();
            bb_index.insert({bb, idx});
        }
        for (auto bb = f->bb.head; bb; bb = bb->next) {
            u32 index = bb_index.find(bb)->second;
            os << "_" << index << ": ; preds = ";
            for (u32 i = 0; i < bb->pred.size(); ++i) {
                if (i != 0) os << ", ";
                os << "%_" << bb_index.find(bb->pred[i])->second;
            }
            // 这里原来会输出dom info的，现在不输出了，因为现在所有pass结束后dom info不是有效的
            os << endl;

            for (auto inst = bb->insts.head; inst != nullptr; inst = inst->next) {
                os << "\t";

                if (auto x = dyn_cast<ReturnInst>(inst)) {
                    if (x->ret.value) {
                        os << "ret i32 " << pv(v_index, x->ret.value) << endl;
                    } else {
                        os << "ret void" << endl;
                    }
                } else {
                    UNREACHABLE();
                }
            }
        }

        os << "}" << endl << endl;
    }

    return os;
}

