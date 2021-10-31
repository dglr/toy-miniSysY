//
// Created by dglr on 2021/10/30.
//
#include "ssa.h"

#include "../structure/ast.h"

struct SsaContext {
    IrProgram *program;
    IrFunc *func;
    BasicBlock *bb;
    // bb stack for (continue, break)
    std::vector<std::pair<BasicBlock *, BasicBlock *>> loop_stk;
};

Value *convert_expr(SsaContext *ctx, Expr *expr) {
    if (auto x = dyn_cast<IntConst>(expr)) {
        return ConstValue::get(x->val);
    }
    return nullptr;
}

void convert_stmt(SsaContext *ctx, Stmt *stmt) {
  if (auto x = dyn_cast<Block>(stmt)) {
        for (auto &stmt : x->stmts) {
            convert_stmt(ctx, stmt);
            if ( isa<Return>(stmt)) {
                break;
            }
        }
    } else if (auto x = dyn_cast<Return>(stmt)) {
        if (x->val) {
            auto value = convert_expr(ctx, x->val);
            new ReturnInst(value, ctx->bb);
        } else {
            new ReturnInst(nullptr, ctx->bb);
        }
    }
}

IrProgram *convert_ssa(Program &p) {
    IrProgram *ret = new IrProgram;
    for (auto &f : p.glob) {
        IrFunc *func = new IrFunc;
        func->func = &f;
        f.val = func;
        ret->func.insertAtEnd(func);
    }
    for (auto &f : p.glob) {
            IrFunc *func = f.val;
            BasicBlock *entryBB = new BasicBlock;
            func->bb.insertAtEnd(entryBB);



            SsaContext ctx = {ret, func, entryBB};
            for (auto &stmt : f.body.stmts) {
                convert_stmt(&ctx, stmt);
            }

            // add extra return statement to avoid undefined behavior
            if (!ctx.bb->valid()) {
                if (func->func->is_int) {
                    new ReturnInst(ConstValue::get(0), ctx.bb);
                } else {
                    new ReturnInst(nullptr, ctx.bb);
                }
            }

            for (BasicBlock *bb = func->bb.head; bb; bb = bb->next) {
                bb->pred.clear();
            }
            for (BasicBlock *bb = func->bb.head; bb; bb = bb->next) {
                for (BasicBlock *x : bb->succ()) {
                    if (x) x->pred.push_back(bb);
                }
            }
    }
    return ret;
}

