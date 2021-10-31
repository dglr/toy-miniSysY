//
// Created by dglr on 2021/10/30.
//

#include "typecheck.h"

#include <cstdlib>
#include <string>


#define ERR(...) ERR_EXIT(TYPE_CHECK_ERR, __VA_ARGS__)


struct Env {
    // stacks of local decls
    Func *cur_func = nullptr;

    void ck_func(Func *f) {
        cur_func = f;
        for (Stmt *s : f->body.stmts) {
            ck_stmt(s);
        }
    }



    void ck_stmt(Stmt *s) {
         if (auto x = dyn_cast<Return>(s)) {
            auto t = x->val ? ck_expr(x->val) : std::pair<Expr **, Expr **>{};
            if (!((cur_func->is_int && is_int(t)) || (!cur_func->is_int && !t.first))) {
                ERR("return type mismatch");
            }
        } else {
            UNREACHABLE();
        }
    }



    // 配合ck_expr使用
    static bool is_int(std::pair<Expr **, Expr **> t) { return t.first && t.first == t.second; }

    // 返回Option<&[Expr *]>：
    // 类型是int时返回空slice，类型是void时返回None，类型是int[]...时返回对应维度的slice
    std::pair<Expr **, Expr **> ck_expr(Expr *e) {
        const std::pair<Expr **, Expr **> empty{reinterpret_cast<Expr **>(8), reinterpret_cast<Expr **>(8)};
        if (auto x = dyn_cast<IntConst>(e)) {
            e->result = x->val;
            return empty;
        } else {
            UNREACHABLE();
        }
    }


};

void type_check(Program &p) {
    Env env;
    for (auto &f : p.glob) {
            env.ck_func(&f);

    }
}

