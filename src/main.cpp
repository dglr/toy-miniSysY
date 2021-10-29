#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "util/common.h"

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

    return 0;
}