//
// Created by dglr on 2021/10/30.
//

#ifndef TOY_MINISYSY_IR_H
#define TOY_MINISYSY_IR_H

#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <map>
#include <set>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../util/common.h"

// 声明ast中用到的类型，从而让这里不需要include "ast.hpp"。真正需要访问字段的文件里自己include
struct Func;

struct Inst;
struct IrFunc;
struct Use;



struct Value {
    // value is used by ...
    ilist<Use> uses;
    // tag
    enum class Tag {
        Return,  // Control flow
        Const,

    } tag;

    Value(Tag tag) : tag(tag) {}

    void addUse(Use *u) { uses.insertAtEnd(u); }
    void killUse(Use *u) { uses.remove(u); }

    // 将对自身所有的使用替换成对v的使用
    inline void replaceAllUseWith(Value *v);
    // 调用deleteValue语义上相当于delete掉它，但是按照现在的实现不能直接delete它
    void deleteValue();
};

struct Use {
    DEFINE_ILIST(Use)

    Value *value;
    Inst *user;

    // 这个构造函数没有初始化prev和next，这没有关系
    // 因为prev和next永远不会从一个Use开始被主动使用，而是在遍历Use链表的时候用到
    // 而既然这个Use已经被加入了一个链表，它的prev和next也就已经被赋值了
    Use(Value *v, Inst *u) : value(v), user(u) {
        if (v) v->addUse(this);
    }

    // 没有必要定义移动构造函数/拷贝运算符，语义没有区别
    // 一般不会用到它们，只在类似vector内部构造临时变量又析构的场景中用到
    Use(const Use &rhs) : value(rhs.value), user(rhs.user) {
        if (value) value->addUse(this);
    }
    Use &operator=(const Use &rhs) {
        if (this != &rhs) {
            assert(user == rhs.user);
            set(rhs.value);
        }
        return *this;
    }

    // 注意不要写.value = xxx, 而是用.set(xxx), 因为需要记录被use的关系
    void set(Value *v) {
        if (value) value->killUse(this);
        value = v;
        if (v) v->addUse(this);
    }

    ~Use() {
        if (value) value->killUse(this);
    }
};


struct IrProgram {
    ilist<IrFunc> func;
    friend std::ostream &operator<<(std::ostream &os, const IrProgram &dt);
};

std::ostream &operator<<(std::ostream &os, const IrProgram &dt);

struct BasicBlock {
    DEFINE_ILIST(BasicBlock)
    std::vector<BasicBlock *> pred;

    ilist<Inst> insts;

    inline std::array<BasicBlock *, 2> succ();
    inline bool valid();
};

struct IrFunc {
    DEFINE_ILIST(IrFunc)
    Func *func;
    ilist<BasicBlock> bb;
    // functions called by this function
    std::set<IrFunc *> callee_func;
    // functions calling this function
    std::set<IrFunc *> caller_func;

};

struct ConstValue : Value {
    DEFINE_CLASSOF(Value, p->tag == Tag::Const);
    const i32 imm;

    static std::unordered_map<i32, ConstValue *> POOL;

    static ConstValue *get(i32 imm) {
        auto [it, inserted] = POOL.insert({imm, nullptr});
        if (inserted) it->second = new ConstValue(imm);
        return it->second;
    }

private:
    // use ConstValue::get instead
    explicit ConstValue(i32 imm) : Value(Tag::Const), imm(imm) {}
};


struct Inst : Value {
    // instruction linked list
    DEFINE_ILIST(Inst)
    // basic block
    BasicBlock *bb;

    // insert this inst before `insertBefore`
    Inst(Tag tag, Inst *insertBefore) : Value(tag), bb(insertBefore->bb) { bb->insts.insertBefore(this, insertBefore); }

    // insert this inst at the end of `insertAtEnd`
    Inst(Tag tag, BasicBlock *insertAtEnd) : Value(tag), bb(insertAtEnd) { bb->insts.insertAtEnd(this); }

    // 只初始化tag，没有加入到链表中，调用者手动加入
    Inst(Tag tag) : Value(tag) {}

    // 返回的指针对是一个左闭右开区间，表示这条指令的所有操作数，.value可能为空

};

struct ReturnInst : Inst {
    DEFINE_CLASSOF(Value, p->tag == Tag::Return);
    Use ret;

    ReturnInst(Value *ret, BasicBlock *insertAtEnd) : Inst(Tag::Return, insertAtEnd), ret(ret, this) {}
};


std::array<BasicBlock *, 2> BasicBlock::succ() {
    Inst *end = insts.tail;  // 必须非空
    if (isa<ReturnInst>(end))
        return {nullptr, nullptr};
    else
        UNREACHABLE();
}

bool BasicBlock::valid() {
    Inst *end = insts.tail;
    return end && ( isa<ReturnInst>(end));
}


#endif //TOY_MINISYSY_IR_H
