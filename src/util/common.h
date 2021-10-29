//
// Created by dglr on 2021/10/29.
//

#ifndef TOY_MINISYSY_COMMON_H
#define TOY_MINISYSY_COMMON_H
#include "dbg.h"
#include <type_traits>

#define ERR_EXIT(code, ...) \
  do {                      \
    dbg(__VA_ARGS__);       \
    exit(code);             \
  } while (false)

#define UNREACHABLE() ERR_EXIT(SYSTEM_ERR, "control flow should never reach here")


enum{
    SYSTEM_ERR = 1,
    PARSER_ERR,
    TYPE_CHECK_ERR,
    UNREACHABLE_ERR
};

using i32 = int32_t;
using u32 = uint32_t;

#define DEFINE_CLASSOF(cls, cond) \
  static bool classof(const cls *p) { return cond; }

#define DEFINE_ILIST(cls) \
  cls *prev;              \
  cls *next;

template <class T>
struct IndexMapper {
    std::map<T *, u32> mapping;
    u32 index_max = 0;

    u32 alloc() { return index_max++; }

    u32 get(T *t) {
        auto [it, inserted] = mapping.insert({t, index_max});
        index_max += inserted;
        return it->second;
    }
};

template <class Node>
struct ilist {
    Node *head;
    Node *tail;

    ilist() { head = tail = nullptr; }

    // insert newNode before insertBefore
    void insertBefore(Node *newNode, Node *insertBefore) {
        newNode->prev = insertBefore->prev;
        newNode->next = insertBefore;
        if (insertBefore->prev) {
            insertBefore->prev->next = newNode;
        }
        insertBefore->prev = newNode;

        if (head == insertBefore) {
            head = newNode;
        }
    }

    // insert newNode after insertAfter
    void insertAfter(Node *newNode, Node *insertAfter) {
        newNode->prev = insertAfter;
        newNode->next = insertAfter->next;
        if (insertAfter->next) {
            insertAfter->next->prev = newNode;
        }
        insertAfter->next = newNode;

        if (tail == insertAfter) {
            tail = newNode;
        }
    }

    // insert newNode at the end of ilist
    void insertAtEnd(Node *newNode) {
        newNode->prev = tail;
        newNode->next = nullptr;

        if (tail == nullptr) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    }

    // insert newNode at the begin of ilist
    void insertAtBegin(Node *newNode) {
        newNode->prev = nullptr;
        newNode->next = head;

        if (head == nullptr) {
            head = tail = newNode;
        } else {
            head->prev = newNode;
            head = newNode;
        }
    }

    // remove node from ilist
    void remove(Node *node) {
        if (node->prev != nullptr) {
            node->prev->next = node->next;
        } else {
            head = node->next;
        }

        if (node->next != nullptr) {
            node->next->prev = node->prev;
        } else {
            tail = node->prev;
        }
    }
};

template <typename D, typename B>
bool isa(const B *b) {
    if constexpr (std::is_base_of_v<D, B>) {
        return true;
    } else {
        static_assert(std::is_base_of_v<B, D>);
        return D::classof(b);
    }
}

template <typename D, typename B>
const D *dyn_cast(const B *b) {
    return isa<D>(b) ? static_cast<const D *>(b) : nullptr;
}

template <typename D, typename B>
D *dyn_cast(B *b) {
    return isa<D>(b) ? static_cast<D *>(b) : nullptr;
}

template <typename D, typename B>
const D *dyn_cast_nullable(const B *b) {
    return b && isa<D>(b) ? static_cast<const D *>(b) : nullptr;
}

template <typename D, typename B>
D *dyn_cast_nullable(B *b) {
    return b && isa<D>(b) ? static_cast<D *>(b) : nullptr;
}

#endif //TOY_MINISYSY_COMMON_H
