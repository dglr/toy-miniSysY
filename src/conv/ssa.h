//
// Created by dglr on 2021/10/30.
//

#ifndef TOY_MINISYSY_SSA_H
#define TOY_MINISYSY_SSA_H

#include "../structure/ast.hpp"
#include "../structure/ir.hpp"

IrProgram *convert_ssa(Program &p);

#endif //TOY_MINISYSY_SSA_H
