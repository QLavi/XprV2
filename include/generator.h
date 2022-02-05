#pragma once

#include <stdint.h>

enum {
    LOAD_NAME,
    STORE_NAME,
    LOAD_CONST,
    BINARY_OP,
    UNARY_OP,
    POP_TOP,
    RETURN_VALUE,
};

typedef struct {
    uint8_t* code;
    int count;
} Stream;

void print_opcodes(AST_Node* node);
