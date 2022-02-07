#include "io_and_mem.h"
#include "lexer.h"
#include "parser.h"
#include "generator.h"
#include "vm.h"

int main(int argc, char** argv) {
    if(argc != 2) {
        fprintf(stderr, "Usage: ./xpr <file>\n");
        exit(1);
    }
    char* src = load_file(argv[1]);
    init_vm();

    init_parser(src);
    AST_Node* node = statement();

    int count;
    uint8_t* stream = generate_opcodes(node, &count);
    eval(stream, count);

    free_ast(node);
    free_vm();
    FREE(uint8_t, stream);
    FREE(char, src);
    return 0;
}
