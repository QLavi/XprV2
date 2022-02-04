#include "io_and_mem.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char** argv) {
    if(argc != 2) {
        fprintf(stderr, "Usage: ./xpr <file>\n");
        exit(1);
    }
    char* src = load_file(argv[1]);
    AST_Node* node = statement_list();
    free_ast(node);

    FREE(char, src);
    return 0;
}
