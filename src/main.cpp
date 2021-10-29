#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fstream>
#include "util/common.h"
#include "frontend/parser.h"
#include "conv/typecheck.h"
#include "conv/ssa.h"

using namespace std;

// 程序的主函数
int main(int argc, char *argv[]) {
    char *src = nullptr, *output = nullptr;
    for (int ch;(ch = getopt(argc, argv, "Sdpl:o:O:h")) != -1;) {
        switch (ch) {
            case 'o':
                output = strdup(optarg);
                break;
            default:
                break;
        }
    }
    if(optind<=argc){
        src=strdup(argv[optind]);
    }
    if(src==nullptr){
        fprintf(stderr, "Usage: %s [-o output_file] input_file\n", argv[0]);
        return SYSTEM_ERR;
    }
    if(output== nullptr){
        output="output.ll";
    }

    // open source file
    int fd = open(src, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "failed to open %s\n", src);
        return SYSTEM_ERR;
    }
    struct stat st {};
    fstat(fd, &st);
    char *input = (char *)mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    // run lexer
    Lexer l(std::string_view(input, st.st_size));
    auto result = Parser{}.parse(l);

    // run parser
    if (Program *p = std::get_if<0>(&result)) {
        dbg("parsing success");
        type_check(*p);  // 失败时直接就exit(1)了
        dbg("type_check success");
        auto *ir = convert_ssa(*p);
        std::ofstream(output) << *ir;
    } else if (Token *t = std::get_if<1>(&result)) {
        ERR_EXIT(PARSER_ERR, "parsing error", t->kind, t->line, t->col, t->piece);
    }

    return 0;
}