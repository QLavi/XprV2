#include "io_and_mem.h"
#include "parser.h"
#include "generator.h"

char* op_to_str[] = {
    [ADD_OP] = "+",
    [SUB_OP] = "-",
    [MUL_OP] = "*",
    [DIV_OP] = "/",
    [LESS_OP] = "<",
    [LESS_EQUAL_OP] = "<=",
    [GREATER_OP] = ">",
    [GREATER_EQUAL_OP] = ">=",
    [NOT_EQUAL_OP] = "!=",
    [EQUAL_OP] = "==",
    [NEGATE_OP] = "-",
    [NOT_OP] = "",
};

int count = 0;
void generate_opcodes_from_ast(AST_Node* node, uint8_t* stream) {
    if(node == NULL) return;

    for(int x = 0; x < node->count; x++) {
        generate_opcodes_from_ast(node->child_nodes[x], stream);
    }

    switch(node->type) {
        case NODE_BINARY_OP:
            switch(node->op_type) {
                case ADD_OP:
                case SUB_OP:
                case MUL_OP:
                case DIV_OP:
                case LESS_OP:
                case LESS_EQUAL_OP:
                case GREATER_OP:
                case GREATER_EQUAL_OP:
                case NOT_EQUAL_OP:
                case EQUAL_OP:
                    stream[count++] = BINARY_OP;
                    stream[count++] = node->op_type;
                    break;
                default:
                    return;
            } break;
        case NODE_UNARY_OP:
            switch(node->op_type) {
                case NEGATE_OP:
                case NOT_OP:
                    stream[count++] = UNARY_OP;
                    stream[count++] = node->op_type;
                    break;
                default:
                    return;
            } break;
        case NODE_LOAD:
            stream[count++] = LOAD_NAME;
            stream[count++] = node->id[0];
            break;
        case NODE_STORE:
            stream[count++] = STORE_NAME;
            stream[count++] = node->id[0];
            break;
        case NODE_VALUE:
            stream[count++] = LOAD_CONST;
            stream[count++] = node->value;
            break;
        default:
            return;
    }
}

void print_opcodes(AST_Node* node) {
    uint8_t* stream = ALLOC(uint8_t, 256);
    generate_opcodes_from_ast(node, stream);

    uint8_t* ip = stream;

    int offset = 0;
    for(;offset < count;) {
        switch(*ip++) {
            case BINARY_OP:
                printf("  %4i %-16s (%s)\n", offset, "BINARY_OP", op_to_str[*ip++]);
                break;
            case UNARY_OP:
                printf("  %4i %-16s (%s)\n", offset, "UNARY_OP", op_to_str[*ip++]);
                break;
            case LOAD_NAME:
                printf("  %4i %-16s (%c)\n", offset, "LOAD_NAME", *ip++);
                break;
            case STORE_NAME:
                printf("  %4i %-16s (%c)\n", offset, "STORE_NAME", *ip++);
                break;
            case LOAD_CONST:
                printf("  %4i %-16s (0) '%i'\n", offset, "LOAD_CONST", *ip++);
                break;
            case POP_TOP:
                printf("  %4i %-16s\n", offset, "POP_TOP");
                break;
            case RETURN_VALUE:
                printf("  %4i %-16s\n", offset, "RETURN_VALUE");
                break;
        }
        offset = ip - stream;
    }

    FREE(uint8_t, stream);
}
