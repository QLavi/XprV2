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

void generate_opcodes_from_ast(AST_Node* node, uint8_t* stream);

void if_block_opcodes(AST_Node* node, uint8_t* stream) {
    generate_opcodes_from_ast(node->child_nodes[0], stream);
    stream[count++] = POP_JUMP_IF_FALSE;
    stream[count++] = 0xFF;
    stream[count++] = 0xFF;
    generate_opcodes_from_ast(node->child_nodes[1], stream);
}

void generate_opcodes_from_ast(AST_Node* node, uint8_t* stream) {
    if(node == NULL) return;

    switch(node->type) {
        case NODE_IF:
            if_block_opcodes(node, stream);
            return;
        default:
            break;
    }
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

AST_Node* if_block = NULL;
void find_jump_emitter(AST_Node* node) {
    if(node == NULL) return;

    switch(node->type) {
        case NODE_IF:
            if_block = node->child_nodes[1];
        default:
            break;
    }
    for(int x = 0; x < node->count; x++) {
        find_jump_emitter(node->child_nodes[x]);
    }
}

void patch_jump(uint8_t* stream) {
    uint8_t* ip = stream;
    for(;;) {
        switch(*ip++) {
            case POP_JUMP_IF_FALSE:
                uint16_t c = count;
                *ip++ = (c >> 8) & 0xFF;
                *ip++ = c & 0xFF;
                return;
        }
    }
}

void backpatch(AST_Node* node, uint8_t* stream) {
    find_jump_emitter(node);
    AST_Node* block_node = if_block;
    int tmp = count;
    count = 0;
    uint8_t* block_stream = ALLOC(uint8_t, 256);
    generate_opcodes_from_ast(block_node, block_stream);
    patch_jump(stream);
    count = tmp;
}

void print_opcodes(AST_Node* node) {
    if(node == NULL) return;
    uint8_t* stream = ALLOC(uint8_t, 256);
    generate_opcodes_from_ast(node, stream);
    backpatch(node, stream);

    uint8_t* ip = stream;

    int offset = 0;
    int jump_offset = -1;
    for(;offset < count;) {
        if(offset == jump_offset) {
            printf(">>  ");
        }
        switch(*ip++) {
            case POP_JUMP_IF_FALSE:
                jump_offset = *ip++ << 8 | *ip++;
                jump_offset = offset + jump_offset + 3;
                printf("\t%4i %-20s (to %i) \n", offset, "POP_JUMP_IF_FALSE", jump_offset);
                break;
            case BINARY_OP:
                printf("\t%4i %-20s (%s)\n", offset, "BINARY_OP", op_to_str[*ip++]);
                break;
            case UNARY_OP:
                printf("\t%4i %-20s (%s)\n", offset, "UNARY_OP", op_to_str[*ip++]);
                break;
            case LOAD_NAME:
                printf("\t%4i %-20s (%c)\n", offset, "LOAD_NAME", *ip++);
                break;
            case STORE_NAME:
                printf("\t%4i %-20s (%c)\n", offset, "STORE_NAME", *ip++);
                break;
            case LOAD_CONST:
                printf("\t%4i %-20s (0) '%i'\n", offset, "LOAD_CONST", *ip++);
                break;
            case POP_TOP:
                printf("\t%4i %-20s\n", offset, "POP_TOP");
                break;
            case RETURN_VALUE:
                printf("\t%4i %-20s\n", offset, "RETURN_VALUE");
                break;
        }
        offset = ip - stream;
    }

    FREE(uint8_t, stream);
}
