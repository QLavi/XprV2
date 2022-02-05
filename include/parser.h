#pragma once

#include <stdint.h>

typedef enum {
    ADD_OP,
    SUB_OP,
    MUL_OP,
    DIV_OP,
    NEGATE_OP,
    NOT_OP,
    LESS_OP,
    LESS_EQUAL_OP,
    GREATER_OP,
    GREATER_EQUAL_OP,
    NOT_EQUAL_OP,
    EQUAL_OP,
    NO_OP
} Op_Type;

typedef enum {
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_STATEMENT_LIST,
    NODE_LOAD,
    NODE_STORE,
    NODE_VALUE
} Node_Type;

typedef double Value;

typedef struct AST_Node AST_Node;
struct AST_Node {
    Node_Type type;
    union {
        Value value;
        char* id;
        Op_Type op_type;
    };
    AST_Node** child_nodes;
    int capacity;
    int count;
} ;

void init_parser(char*);
AST_Node* statement_list(void);
void free_ast(AST_Node*);
