//
// Created by dglr on 2021/10/29.
//

#ifndef TOY_MINISYSY_PARSER_H
#define TOY_MINISYSY_PARSER_H

#include <cstdint>
#include <cstdlib>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>
#include "../structure/ast.h"
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;

struct Token
{
    enum Kind : u32
    {
        _Eps,
        _Eof,
        _Err,
        Int,
        Return,
        Semi,
        LPar,
        RPar,
        LBrc,
        RBrc,
        IntConst,
        Ident
    } kind;
    std::string_view piece;
    u32 line, col;
};

using StackItem = std::variant<Token, Program, Func, Block, Stmt *, std::vector<Stmt *>, Expr *>;

struct Lexer
{
    std::string_view string;
    u32 line, col;

    explicit Lexer(std::string_view string) : string(string), line(1), col(1) {}
    Token next();
};

struct Parser
{
    std::variant<Program, Token> parse(Lexer &lexer);
};

#endif //TOY_MINISYSY_PARSER_H
