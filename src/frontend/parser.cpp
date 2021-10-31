//
// Created by dglr on 2021/10/29.
//


#include "parser.h"
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;


Token Lexer::next()
{
    const static Token::Kind ACC[] = {
            Token::_Err,
            Token::_Err,
            Token::_Eps,
            Token::LPar,
            Token::RPar,
            Token::_Err,
            Token::IntConst,
            Token::IntConst,
            Token::Semi,
            Token::Ident,
            Token::Ident,
            Token::Ident,
            Token::LBrc,
            Token::RBrc,
            Token::_Err,
            Token::_Eps,
            Token::_Err,
            Token::Ident,
            Token::Ident,
            Token::_Err,
            Token::IntConst,
            Token::Int,
            Token::Ident,
            Token::_Eps,
            Token::Ident,
            Token::Ident,
            Token::Return,
    };
    const static u8 EC[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 3, 4, 5, 0, 0, 0, 0, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0, 9, 0, 0, 0, 0, 0, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 0, 0, 0, 0, 11, 0, 10, 10, 10, 10, 12, 10, 11, 11, 13, 11, 11, 11, 11, 14, 11, 11, 11, 15, 11, 16, 17, 11, 11, 18, 11, 11, 19, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    const static u8 DFA_EDGE[][21] = {
            {1, 2, 2, 3, 4, 1, 5, 6, 7, 8, 9, 9, 9, 10, 9, 11, 9, 9, 9, 12, 13},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 9, 9, 0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 9, 9, 0, 9, 9, 9, 9, 17, 9, 9, 9, 9, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 9, 9, 0, 9, 9, 18, 9, 9, 9, 9, 9, 9, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {14, 14, 14, 14, 14, 19, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14},
            {15, 15, 0, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15},
            {0, 0, 0, 0, 0, 0, 0, 20, 20, 0, 20, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 9, 9, 0, 9, 9, 9, 9, 9, 9, 21, 9, 9, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 9, 9, 0, 9, 9, 9, 9, 9, 9, 22, 9, 9, 0, 0},
            {14, 14, 14, 14, 14, 14, 23, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14},
            {0, 0, 0, 0, 0, 0, 0, 20, 20, 0, 20, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 9, 9, 0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 9, 9, 0, 9, 9, 9, 9, 9, 9, 9, 24, 9, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 9, 9, 0, 9, 9, 9, 9, 9, 25, 9, 9, 9, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 9, 9, 0, 9, 9, 9, 9, 26, 9, 9, 9, 9, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 9, 9, 0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0, 0},
    };
    u32 l = line, c = col;
    Token::Kind last_acc = Token::_Err;
    u32 state = 0, i = 0;
    while (true)
    {
        u32 ch, nxt;
        if (i < string.size())
        {
            ch = string[i], nxt = DFA_EDGE[state][EC[ch]];
        }
        else
        {
            ch = nxt = 0;
        }
        Token::Kind acc = ACC[nxt];
        if (acc != Token::_Err)
        {
            last_acc = acc;
        }
        state = nxt;
        if (nxt == 0)
        { // dead, should not eat this char
            // we are not using substr here, because it does range check and may throws exception
            std::string_view piece(string.data(), i);
            string = std::string_view(string.data() + i, string.size() - i);
            if (i == 0 || last_acc != Token::_Eps)
            {

                return Token{i == 0 ? Token::_Eof : last_acc, piece, l, c};
            }
            else
            {
                l = line, c = col, last_acc = Token::_Err, state = 0, i = 0;
            }
        }
        else
        { // continue, eat this char
            if (ch == '\n')
            {
                ++line, col = 1;
            }
            else
            {
                ++col;
            }
            ++i;
        }
    }
}

std::variant<Program, Token> Parser::parse(Lexer &lexer)
{
    const static u8 PROD[] = {0, 0, 1, 2, 3, 4, 4, 5, 6};
    const static u8 ACTION[][12] = {
            {
                    3,
                    5,
                    3,
                    5,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
            },
            {
                    3,
                    2,
                    3,
                    8,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
            },
            {
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    16,
            },
            {
                    3,
                    1,
                    3,
                    1,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
            },
            {
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    20,
                    3,
                    3,
                    3,
                    3,
                    3,
            },
            {
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    24,
                    3,
                    3,
                    3,
                    3,
            },
            {
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    28,
                    3,
                    3,
                    3,
            },
            {
                    3,
                    3,
                    3,
                    3,
                    25,
                    3,
                    3,
                    3,
                    3,
                    25,
                    3,
                    3,
            },
            {
                    3,
                    9,
                    3,
                    9,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
            },
            {
                    3,
                    3,
                    3,
                    3,
                    40,
                    3,
                    3,
                    3,
                    3,
                    44,
                    3,
                    3,
            },
            {
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    52,
                    3,
            },
            {
                    3,
                    13,
                    3,
                    13,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
            },
            {
                    3,
                    3,
                    3,
                    3,
                    21,
                    3,
                    3,
                    3,
                    3,
                    21,
                    3,
                    3,
            },
            {
                    3,
                    3,
                    3,
                    3,
                    3,
                    29,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
            },
            {
                    3,
                    3,
                    3,
                    3,
                    3,
                    60,
                    3,
                    3,
                    3,
                    3,
                    3,
                    3,
            },
            {
                    3,
                    3,
                    3,
                    3,
                    17,
                    3,
                    3,
                    3,
                    3,
                    17,
                    3,
                    3,
            },
    };
    const static u8 GOTO[][7] = {
            {1, 0, 0, 0, 0, 0, 0},
            {0, 3, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 8, 0, 0, 0, 0},
            {0, 0, 0, 0, 9, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 12, 0, 0, 0},
            {0, 0, 0, 0, 0, 14, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
    };
    std::vector<std::pair<StackItem, u8>> stk;
    stk.emplace_back(StackItem{}, 0);
    u32 state = 0;
    Token token = lexer.next();
    while (true)
    {
        u32 act = ACTION[state][token.kind], act_val = act >> 2;
        switch (act & 3)
        {
            case 0:
            {
                stk.emplace_back(token, act_val);
                state = act_val;
                token = lexer.next();
                break;
            }
            case 1:
            {
                StackItem __;
                // we are confident that the value must be what we expect, so directly dereference the return value of `std::get_if`
                // we are not using `std::get`, because it performs runtime check, and throws exceptions when fails
                switch (act_val)
                {
                    case 0:
                    {
                        [[maybe_unused]] Func _2(std::move(*std::get_if<Func>(&stk.back().first)));
                        stk.pop_back();
                        [[maybe_unused]] Program _1(std::move(*std::get_if<Program>(&stk.back().first)));
                        stk.pop_back();
                        _1.glob.emplace_back(std::move(_2));
                        __ = std::move(_1);
                        break;
                    }
                    case 1:
                    {
                        __ = Program{};
                        break;
                    }
                    case 2:
                    {
                        [[maybe_unused]] Block _5(std::move(*std::get_if<Block>(&stk.back().first)));
                        stk.pop_back();
                        [[maybe_unused]] Token _4(std::move(*std::get_if<Token>(&stk.back().first)));
                        stk.pop_back();
                        [[maybe_unused]] Token _3(std::move(*std::get_if<Token>(&stk.back().first)));
                        stk.pop_back();
                        [[maybe_unused]] Token _2(std::move(*std::get_if<Token>(&stk.back().first)));
                        stk.pop_back();
                        [[maybe_unused]] Token _1(std::move(*std::get_if<Token>(&stk.back().first)));
                        stk.pop_back();
                        __ = Func{true, _2.piece,  std::move(_5)};
                        break;
                    }
                    case 3:
                    {
                        [[maybe_unused]] Token _3(std::move(*std::get_if<Token>(&stk.back().first)));
                        stk.pop_back();
                        [[maybe_unused]] std::vector<Stmt *> _2(std::move(*std::get_if<std::vector<Stmt *>>(&stk.back().first)));
                        stk.pop_back();
                        [[maybe_unused]] Token _1(std::move(*std::get_if<Token>(&stk.back().first)));
                        stk.pop_back();
                        __ = Block{Stmt::Block, std::move(_2)};
                        break;
                    }
                    case 4:
                    {
                        [[maybe_unused]] Token _3(std::move(*std::get_if<Token>(&stk.back().first)));
                        stk.pop_back();
                        [[maybe_unused]] Expr *_2(std::move(*std::get_if<Expr *>(&stk.back().first)));
                        stk.pop_back();
                        [[maybe_unused]] Token _1(std::move(*std::get_if<Token>(&stk.back().first)));
                        stk.pop_back();
                        __ = new Return{Stmt::Return, _2};
                        break;
                    }
                    case 5:
                    {
                        [[maybe_unused]] Stmt *_2(std::move(*std::get_if<Stmt *>(&stk.back().first)));
                        stk.pop_back();
                        [[maybe_unused]] std::vector<Stmt *> _1(std::move(*std::get_if<std::vector<Stmt *>>(&stk.back().first)));
                        stk.pop_back();
                        _1.push_back(_2);
                        __ = std::move(_1);
                        break;
                    }
                    case 6:
                    {
                        __ = std::vector<Stmt *>();
                        break;
                    }
                    case 7:
                    {
                        [[maybe_unused]] Token _1(std::move(*std::get_if<Token>(&stk.back().first)));
                        stk.pop_back();
                        const char *data = _1.piece.data();
                        int base = data[1] == 'x' ? 16 : data[0] == '0' ? 8
                                                                        : 10;
                        __ = new IntConst{Expr::IntConst, 0, (int)strtol(data, nullptr, base)};
                        break;
                    }
                    case 8:
                    {
                        [[maybe_unused]] Program _1(std::move(*std::get_if<Program>(&stk.back().first)));
                        stk.pop_back();
                        __ = _1;
                        break;
                    }

                    default:
                        __builtin_unreachable();
                }
                u8 nxt = GOTO[stk.back().second][PROD[act_val]];
                stk.emplace_back(std::move(__), nxt);
                state = nxt;
                break;
            }
            case 2:
                return std::move(*std::get_if<Program>(&stk.back().first));
            case 3:
                return token;
            default:
                __builtin_unreachable();
        }
    }
}
