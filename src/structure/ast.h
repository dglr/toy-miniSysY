//
// Created by dglr on 2021/10/29.
//

#ifndef TOY_MINISYSY_AST_H
#define TOY_MINISYSY_AST_H

#pragma once

#include <cstdint>
#include <string_view>
#include <string>
#include <variant>
#include <vector>

#include "../util/common.h"

struct Func;

struct Expr {
    enum Tag {
        IntConst
    } tag;
    i32 result;  // 保存解析出来的结果，typeck阶段计算它，后续用它
};


struct IntConst : Expr {
    DEFINE_CLASSOF(Expr, p->tag == Tag::IntConst);
    i32 val;
    static IntConst ZERO;  // 值为0的IntConst，很多地方会用到，所以做个单例
};
//


struct Value;

struct Stmt {
    enum {
         Block,Return
    } tag;
};

struct Block : Stmt {
    DEFINE_CLASSOF(Stmt, p->tag == Stmt::Block);
    std::vector<Stmt *> stmts;
};
//

struct Return : Stmt {
    DEFINE_CLASSOF(Stmt, p->tag == Stmt::Return);
    Expr *val;  // nullable
};

struct IrFunc;

struct Func {
    // 返回类型只能是int/void，因此只记录是否是int
    bool is_int;
    std::string_view name;

    Block body;

    // ast->ir阶段赋值
    IrFunc *val;

};

struct Program {
    std::vector<Func> glob;
};


#endif //TOY_MINISYSY_AST_H
