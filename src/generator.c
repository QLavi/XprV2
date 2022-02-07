#include "io_and_mem.h"
#include "parser.h"
#include "generator.h"
#include "vm.h"

#define DEBUG_OPCODE_STREAM

char* op_to_str[] = {
    [ADD_OP] = "+",
    [SUB_OP] = "-",
    [MUL_OP] = "*",
    [DIV_OP] = "/",
    [MOD_OP] = "%",
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
int emitter_ids[256];
int em_count = 0;

void generate_opcodes_from_ast(AST_Node* node, uint8_t* stream);

void if_block_opcodes(AST_Node* node, uint8_t* stream) {
    emitter_ids[em_count++] = node->jump_id;
    generate_opcodes_from_ast(node->child_nodes[0], stream);
    stream[count++] = POP_JUMP_IF_FALSE;
    stream[count++] = node->jump_id;

    // NOTE: special code to tell to jump over the JUMP_BLOCK instruction

    stream[count++] = node->count == 3 ? 0xCD : 0xFF;
    generate_opcodes_from_ast(node->child_nodes[1], stream);

    if(node->count == 3) {
        emitter_ids[em_count++] = node->child_nodes[2]->jump_id;
        stream[count++] = JUMP_BLOCK;
        stream[count++] = node->child_nodes[2]->jump_id;
        stream[count++] = 0xFF;
        generate_opcodes_from_ast(node->child_nodes[2]->child_nodes[0], stream);
    }
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
                case MOD_OP:
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
            stream[count++] = load_const(node->value);
            break;
        case NODE_PRINT:
            stream[count++] = PRINT_VALUE;
            (void)stream[count++]; // Note: nothing to add as argument to print
            break;
        default:
            return;
    }
}

AST_Node* jmp_block = NULL;
void find_jump_emitter(AST_Node* node) {
    if(node == NULL) return;

    switch(node->type) {
        case NODE_IF:
            if(emitter_ids[em_count] == node->jump_id) {
                jmp_block = node->child_nodes[1];
            } break;
        case NODE_ELSE:
            if(emitter_ids[em_count] == node->jump_id) {
                jmp_block = node->child_nodes[0];
            } break;
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
            {
                uint8_t id = *ip;
                if(emitter_ids[em_count] == id) {
                    uint16_t c = count;
                    if(ip[1] == 0xCD) {
                        c += 3;
                    }
                    *ip++ = (c >> 8) & 0xFF;
                    *ip++ = c & 0xFF;
                    return;
                }
                ip += 2;
            } break;
            case JUMP_BLOCK:
            {
                uint8_t id = *ip;
                if(emitter_ids[em_count] == id) {
                    uint16_t c = count;
                    *ip++ = (c >> 8) & 0xFF;
                    *ip++ = c & 0xFF;
                    return;
                }
                ip += 2;
            } break;
            case ZERO_I:
                break;
            case LOAD_NAME:
                ip += 1;
                break;
            case STORE_NAME:
                ip += 1;
                break;
            case LOAD_CONST:
                ip += 1;
                break;
            case BINARY_OP:
                ip += 1;
                break;
            case UNARY_OP:
                ip += 1;
                break;
            case POP_TOP:
                break;
            case PRINT_VALUE:
                ip += 1;
                break;
            case RETURN_VALUE:
                return;
        }
    }
}

void backpatch(AST_Node* node, uint8_t* stream) {
    int tmp = count;
    uint8_t* block_stream = ALLOC(uint8_t, 256);
    for(;em_count > 0;) {
        find_jump_emitter(node);
        AST_Node* block_node = jmp_block;
        count = 0;
        generate_opcodes_from_ast(block_node, block_stream);
        patch_jump(stream);
        em_count--;
    }
    count = tmp;
    FREE(uint8_t, block_stream);
}

uint8_t* generate_opcodes(AST_Node* node, int* c) {
    if(node == NULL) return NULL;
    uint8_t* stream = ALLOC(uint8_t, 256);
    generate_opcodes_from_ast(node, stream);
    stream[count++] = RETURN_VALUE;
    count -= 2;
    backpatch(node, stream);

    *c = count;
#ifdef DEBUG_OPCODE_STREAM
    uint8_t* ip = stream;
    int offset = 0;
    int jmp0 = -1;
    for(;offset < count;) {
        switch(*ip++) {
            case POP_JUMP_IF_FALSE:
            {
                jmp0= ip[0] << 8 | ip[1];
                jmp0= offset + jmp0 + 3;
                printf("\t%4i %-20s (to %i) \n", offset, "POP_JUMP_IF_FALSE", jmp0);
                ip += 2;
            } break;
            case JUMP_BLOCK:
            {
                jmp0= ip[0] << 8 | ip[1];
                jmp0= offset + jmp0 + 3;
                printf("\t%4i %-20s (to %i) \n", offset, "JUMP_BLOCK", jmp0);
                ip += 2;
            } break;
            case BINARY_OP:
            {
                printf("\t%4i %-20s (%s)\n", offset, "BINARY_OP", op_to_str[*ip++]);
            } break;
            case UNARY_OP:
            {
                printf("\t%4i %-20s (%s)\n", offset, "UNARY_OP", op_to_str[*ip++]);
            } break;
            case LOAD_NAME:
            {
                printf("\t%4i %-20s (%c)\n", offset, "LOAD_NAME", *ip++);
            } break;
            case STORE_NAME:
            {
                printf("\t%4i %-20s (%c)\n", offset, "STORE_NAME", *ip++);
            } break;
            case LOAD_CONST:
            {
                int index = *ip++;
                printf("\t%4i %-20s (%i) '%g'\n", offset, "LOAD_CONST", index, get_const(index));
            } break;
            case POP_TOP:
            {
                printf("\t%4i %-20s\n", offset, "POP_TOP");
            } break;
            case PRINT_VALUE:
            {
                printf("\t%4i %-20s\n", offset, "PRINT_VALUE");
                ip += 1;
            } break;
            case RETURN_VALUE:
                break;
            case ZERO_I:
            {
                printf("\t%4i %-20s\n", offset, "ZERO_I");
            } break;
        }
        offset = ip - stream;
    }
#endif
    return stream;
}
