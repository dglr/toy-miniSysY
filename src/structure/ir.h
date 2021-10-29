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
struct Decl;

struct Inst;
struct IrFunc;
struct Use;
struct MemPhiInst;

namespace op {
// 我希望Op和整数能够互相转化，所以不用enum class
// 同时我希望Op的成员不要暴露在全局作用域中，所以用一个namespace包起来
    enum Op {
        Add,
        Sub,
        Rsb,
        Mul,
        Div,
        Mod,
        Lt,
        Le,
        Ge,
        Gt,
        Eq,
        Ne,
        And,
        Or,
    };

    inline i32 eval(Op op, i32 l, i32 r) {
        switch (op) {
            case Add:
                return l + r;
            case Sub:
                return l - r;
            case Rsb:
                return r - l;
            case Mul:
                return l * r;
                // 除0就随它去吧，反正我们对于错误都是直接退出的
            case Div:
                return l / r;
            case Mod:
                return l % r;
            case Lt:
                return l < r;
            case Le:
                return l <= r;
            case Ge:
                return l >= r;
            case Gt:
                return l > r;
            case Eq:
                return l == r;
            case Ne:
                return l != r;
            case And:
                return l && r;
            case Or:
                return l || r;
            default:
                UNREACHABLE();
        }
    }

// 是否是两个相反的比较运算符，即 l <a> r 是否等于 r <b> l
    inline bool isrev(Op a, Op b) {
        return (a == Lt && b == Gt) || (a == Gt && b == Lt) || (a == Le && b == Ge) || (a == Ge && b == Le);
    }
}  // namespace op

struct Value {
    // value is used by ...
    ilist<Use> uses;
    // tag
    enum class Tag {
        Add,
        Sub,
        Rsb,
        Mul,
        Div,
        Mod,
        Lt,
        Le,
        Ge,
        Gt,
        Eq,
        Ne,
        And,
        Or,
        Branch,
        Jump,
        Return,  // Control flow
        GetElementPtr,
        Load,
        Store,  // Memory
        Call,
        Alloca,
        Phi,
        MemOp,
        MemPhi,  // 虚拟的MemPhi指令，保证不出现在指令序列中，只出现在BasicBlock::mem_phis中
        Const,
        Global,
        Param,
        Undef,  // Const ~ Undef: Reference
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

void Value::replaceAllUseWith(Value *v) {
    // head->set会将head从链表中移除
    while (uses.head) uses.head->set(v);
}

struct IrProgram {
    ilist<IrFunc> func;
    std::vector<Decl *> glob_decl;

    friend std::ostream &operator<<(std::ostream &os, const IrProgram &dt);
};

std::ostream &operator<<(std::ostream &os, const IrProgram &dt);

struct BasicBlock {
    DEFINE_ILIST(BasicBlock)
    std::vector<BasicBlock *> pred;
    BasicBlock *idom;
    std::unordered_set<BasicBlock *> dom_by;  // 支配它的节点集
    std::vector<BasicBlock *> doms;           // 它支配的节点集
    u32 dom_level;                            // dom树中的深度，根深度为0
    bool vis;  // 各种算法中用到，标记是否访问过，算法开头应把所有vis置false(调用IrFunc::clear_all_vis)
    ilist<Inst> insts;
    ilist<Inst> mem_phis;  // 元素都是MemPhiInst

    inline std::array<BasicBlock *, 2> succ();
    inline std::array<BasicBlock **, 2> succ_ref();  // 想修改succ时使用
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
    bool builtin;
    bool load_global;
    // has_side_effect: 修改了全局变量/传入的数组参数，或者调用了has_side_effect的函数
    // no side effect函数的没有user的调用可以删除
    bool has_side_effect;
    bool can_inline;

    // pure函数的参数相同的调用可以删除
    bool pure() const { return !(load_global || has_side_effect); }

    // 将所有bb的vis置false
    void clear_all_vis() {
        for (BasicBlock *b = bb.head; b; b = b->next) b->vis = false;
    }
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

struct GlobalRef : Value {
    DEFINE_CLASSOF(Value, p->tag == Tag::Global);
    Decl *decl;

    GlobalRef(Decl *decl) : Value(Tag::Global), decl(decl) {}
};

struct ParamRef : Value {
    DEFINE_CLASSOF(Value, p->tag == Tag::Param);
    Decl *decl;

    ParamRef(Decl *decl) : Value(Tag::Param), decl(decl) {}
};

struct UndefValue : Value {
    DEFINE_CLASSOF(Value, p->tag == Tag::Undef);

    UndefValue() : Value(Tag::Undef) {}
    // 这是一个全局可变变量，不过反正也不涉及多线程，不会有冲突
    static UndefValue INSTANCE;
};

struct Inst : Value {
    DEFINE_CLASSOF(Value, Tag::Add <= p->tag && p->tag <= Tag::MemPhi);
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
    std::pair<Use *, Use *> operands();

    inline bool has_side_effect();
};

struct BinaryInst : Inst {
    DEFINE_CLASSOF(Value, Tag::Add <= p->tag && p->tag <= Tag::Or);
    // operands
    // loop unroll pass里用到了lhs和rhs的摆放顺序，不要随便修改
    Use lhs;
    Use rhs;

    BinaryInst(Tag tag, Value *lhs, Value *rhs, BasicBlock *insertAtEnd)
            : Inst(tag, insertAtEnd), lhs(lhs, this), rhs(rhs, this) {}

    BinaryInst(Tag tag, Value *lhs, Value *rhs, Inst *insertBefore)
            : Inst(tag, insertBefore), lhs(lhs, this), rhs(rhs, this) {}

    bool rhsCanBeImm() {
        // Add, Sub, Rsb, Mul, Div, Mod, Lt, Le, Ge, Gt, Eq, Ne, And, Or
        return (tag >= Tag::Add && tag <= Tag::Rsb) || (tag >= Tag::Lt && tag <= Tag::Or);
    }

    constexpr static const char *LLVM_OPS[14] = {
            /* Add = */ "add",
            /* Sub = */ "sub",
            /* Rsb = */ nullptr,
            /* Mul = */ "mul",
            /* Div = */ "sdiv",
            /* Mod = */ "srem",
            /* Lt = */ "icmp slt",
            /* Le = */ "icmp sle",
            /* Ge = */ "icmp sge",
            /* Gt = */ "icmp sgt",
            /* Eq = */ "icmp eq",
            /* Ne = */ "icmp ne",
            /* And = */ "and",
            /* Or = */ "or",
    };

    constexpr static std::pair<Tag, Tag> swapableOperators[11] = {
            {Tag::Add, Tag::Add}, {Tag::Sub, Tag::Rsb}, {Tag::Mul, Tag::Mul}, {Tag::Lt, Tag::Gt},
            {Tag::Le, Tag::Ge},   {Tag::Gt, Tag::Lt},   {Tag::Ge, Tag::Le},   {Tag::Eq, Tag::Eq},
            {Tag::Ne, Tag::Ne},   {Tag::And, Tag::And}, {Tag::Or, Tag::Or},
    };

    bool swapOperand() {
        for (auto [before, after] : swapableOperators) {
            if (tag == before) {
                // note:
                // Use是被pin在内存中的，不能直接swap它们。如果未来希望这样做，需要实现配套的设施，基本上就是把下面的逻辑在构造函数/拷贝运算符中实现
                tag = after;
                Value *l = lhs.value, *r = rhs.value;
                l->killUse(&lhs);
                r->killUse(&rhs);
                l->addUse(&rhs);
                r->addUse(&lhs);
                std::swap(lhs.value, rhs.value);
                return true;
            }
        }
        return false;
    }

    Value *optimizedValue() {
        // imm on rhs
        if (auto r = dyn_cast<ConstValue>(rhs.value)) {
            switch (tag) {
                case Tag::Add:
                case Tag::Sub:
                    return r->imm == 0 ? lhs.value : nullptr;  // ADD or SUB 0
                case Tag::Mul:
                    if (r->imm == 0) return ConstValue::get(0);  // MUL 0
                    [[fallthrough]];
                case Tag::Div:
                    if (r->imm == 1) return lhs.value;  // MUL or DIV 1
                case Tag::Mod:
                    return r->imm == 1 ? ConstValue::get(0) : nullptr;  // MOD 1
                case Tag::And:
                    if (r->imm == 0) return ConstValue::get(0);  // AND 0
                    return r->imm == 1 ? lhs.value : nullptr;    // AND 1
                case Tag::Or:
                    if (r->imm == 1) return ConstValue::get(1);  // OR 1
                    return r->imm == 0 ? lhs.value : nullptr;    // OR 0
                default:
                    return nullptr;
            }
        }
        return nullptr;
    }
};

struct BranchInst : Inst {
    DEFINE_CLASSOF(Value, p->tag == Tag::Branch);
    Use cond;
    // true
    BasicBlock *left;
    // false
    BasicBlock *right;

    BranchInst(Value *cond, BasicBlock *left, BasicBlock *right, BasicBlock *insertAtEnd)
            : Inst(Tag::Branch, insertAtEnd), cond(cond, this), left(left), right(right) {}
};

struct JumpInst : Inst {
    DEFINE_CLASSOF(Value, p->tag == Tag::Jump);
    BasicBlock *next;

    JumpInst(BasicBlock *next, BasicBlock *insertAtEnd) : Inst(Tag::Jump, insertAtEnd), next(next) {}
};

struct ReturnInst : Inst {
    DEFINE_CLASSOF(Value, p->tag == Tag::Return);
    Use ret;

    ReturnInst(Value *ret, BasicBlock *insertAtEnd) : Inst(Tag::Return, insertAtEnd), ret(ret, this) {}
};

struct AccessInst : Inst {
    DEFINE_CLASSOF(Value, p->tag == Tag::GetElementPtr || p->tag == Tag::Load || p->tag == Tag::Store);
    Decl *lhs_sym;
    Use arr;
    Use index;
    AccessInst(Inst::Tag tag, Decl *lhs_sym, Value *arr, Value *index, BasicBlock *insertAtEnd)
            : Inst(tag, insertAtEnd), lhs_sym(lhs_sym), arr(arr, this), index(index, this) {}
};

struct GetElementPtrInst : AccessInst {
    DEFINE_CLASSOF(Value, p->tag == Tag::GetElementPtr);
    int multiplier;
    GetElementPtrInst(Decl *lhs_sym, Value *arr, Value *index, int multiplier, BasicBlock *insertAtEnd)
            : AccessInst(Tag::GetElementPtr, lhs_sym, arr, index, insertAtEnd), multiplier(multiplier) {}
};

struct LoadInst : AccessInst {
    DEFINE_CLASSOF(Value, p->tag == Tag::Load);
    Use mem_token;  // 由memdep pass计算
    LoadInst(Decl *lhs_sym, Value *arr, Value *index, BasicBlock *insertAtEnd)
            : AccessInst(Tag::Load, lhs_sym, arr, index, insertAtEnd), mem_token(nullptr, this) {}
};

struct StoreInst : AccessInst {
    DEFINE_CLASSOF(Value, p->tag == Tag::Store);
    Use data;
    StoreInst(Decl *lhs_sym, Value *arr, Value *data, Value *index, BasicBlock *insertAtEnd)
            : AccessInst(Tag::Store, lhs_sym, arr, index, insertAtEnd), data(data, this) {}
};

struct CallInst : Inst {
    DEFINE_CLASSOF(Value, p->tag == Tag::Call);
    IrFunc *func;
    std::vector<Use> args;
    CallInst(IrFunc *func, BasicBlock *insertAtEnd) : Inst(Tag::Call, insertAtEnd), func(func) {}
};

struct AllocaInst : Inst {
    DEFINE_CLASSOF(Value, p->tag == Tag::Alloca);

    Decl *sym;
    AllocaInst(Decl *sym, BasicBlock *insertBefore) : Inst(Tag::Alloca, insertBefore), sym(sym) {}
};

struct PhiInst : Inst {
    DEFINE_CLASSOF(Value, p->tag == Tag::Phi);
    std::vector<Use> incoming_values;
    std::vector<BasicBlock *> &incoming_bbs() { return bb->pred; }

    explicit PhiInst(BasicBlock *insertAtFront) : Inst(Tag::Phi) {
        bb = insertAtFront;
        bb->insts.insertAtBegin(this);
        u32 n = incoming_bbs().size();
        incoming_values.reserve(n);
        for (u32 i = 0; i < n; ++i) {
            // 在new PhiInst的时候还不知道它用到的value是什么，先填nullptr，后面再用Use::set填上
            incoming_values.emplace_back(nullptr, this);
        }
    }

    explicit PhiInst(Inst *insertBefore) : Inst(Tag::Phi, insertBefore) {
        u32 n = incoming_bbs().size();
        incoming_values.reserve(n);
        for (u32 i = 0; i < n; ++i) {
            incoming_values.emplace_back(nullptr, this);
        }
    }
};

struct MemOpInst : Inst {
    DEFINE_CLASSOF(Value, p->tag == Tag::MemOp);
    Use mem_token;
    LoadInst *load;
    MemOpInst(LoadInst *load, Inst *insertBefore)
            : Inst(Tag::MemOp, insertBefore), mem_token(nullptr, this), load(load) {}
};

// 它的前几个字段和PhiInst是兼容的，所以可以当成PhiInst用(也许理论上有隐患，但是实际上应该没有问题)
// 我不希望让它继承PhiInst，这也许会影响以前的一些对PhiInst的使用
struct MemPhiInst : Inst {
    DEFINE_CLASSOF(Value, p->tag == Tag::MemPhi);
    std::vector<Use> incoming_values;
    std::vector<BasicBlock *> &incoming_bbs() { return bb->pred; }

    // load依赖store和store依赖load两种依赖用到的MemPhiInst不一样
    // 前者的load_or_arr来自于load的数组地址，类型是Decl *，后者的load_or_arr来自于LoadInst
    void *load_or_arr;

    explicit MemPhiInst(void *load_or_arr, BasicBlock *insertAtFront) : Inst(Tag::MemPhi), load_or_arr(load_or_arr) {
        bb = insertAtFront;
        bb->mem_phis.insertAtBegin(this);
        u32 n = incoming_bbs().size();
        incoming_values.reserve(n);
        for (u32 i = 0; i < n; ++i) {
            incoming_values.emplace_back(nullptr, this);
        }
    }
};

bool Inst::has_side_effect() {
    if (isa<BranchInst>(this) || isa<JumpInst>(this) || isa<ReturnInst>(this) || isa<StoreInst>(this)) return true;
    if (auto x = dyn_cast<CallInst>(this); x && x->func->has_side_effect) return true;
    return false;
}

std::array<BasicBlock *, 2> BasicBlock::succ() {
    Inst *end = insts.tail;  // 必须非空
    if (auto x = dyn_cast<BranchInst>(end))
        return {x->left, x->right};
    else if (auto x = dyn_cast<JumpInst>(end))
        return {x->next, nullptr};
    else if (isa<ReturnInst>(end))
        return {nullptr, nullptr};
    else
        UNREACHABLE();
}

std::array<BasicBlock **, 2> BasicBlock::succ_ref() {
    Inst *end = insts.tail;
    if (auto x = dyn_cast<BranchInst>(end))
        return {&x->left, &x->right};
    else if (auto x = dyn_cast<JumpInst>(end))
        return {&x->next, nullptr};
    else if (isa<ReturnInst>(end))
        return {nullptr, nullptr};
    else
        UNREACHABLE();
}

bool BasicBlock::valid() {
    Inst *end = insts.tail;
    return end && (isa<BranchInst>(end) || isa<JumpInst>(end) || isa<ReturnInst>(end));
}


#endif //TOY_MINISYSY_IR_H
