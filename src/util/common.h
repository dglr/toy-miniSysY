//
// Created by dglr on 2021/10/29.
//

#ifndef TOY_MINISYSY_COMMON_H
#define TOY_MINISYSY_COMMON_H
#include "dbg.h"

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

#endif //TOY_MINISYSY_COMMON_H
