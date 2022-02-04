#include "io_and_mem.h"
#include "lexer.h"
#include "parser.h"

typedef struct {
    Token prev_token;
    Token curr_token;
} Parser;

Parser parser;

void init_parser(char* src) {
    init_lexer(src);
    parser.curr_token = next_token();
}

AST_Node* make_ast_node(Node_Type type, Value value, char* id, Op_Type op_type) {
    AST_Node* node = ALLOC(AST_Node, 1);
    node->type = type;
    node->value = value;
    node->id = id;
    node->op_type = op_type;
    node->child_nodes = NULL;
    node->count = 0;
    node->capacity = 0;
    return node;
}

void add_child_node(AST_Node* parent, AST_Node* child) {
    if(parent->capacity -1 < parent->count) {
        int cap = parent->capacity;
        cap = cap < 8 ? cap < 2 ? 2 : 8 : cap * 2;
        parent->capacity = cap;
        parent->child_nodes = REALLOC(AST_Node*, parent->child_nodes, cap);
    }
    parent->child_nodes[parent->count++] = child;
}

void free_ast(AST_Node* node) {
    if(node == NULL) return;

    for(int x = 0; x < node->capacity; x++) {
        free_ast(node->child_nodes[x]);
    }
    FREE(AST_Node*, node->child_nodes);
    FREE(AST_Node, node);
}

AST_Node* statement_list(void) {
    AST_Node* node = make_ast_node(NODE_BINARY_OP, 0, NULL, ADD_OP);
    add_child_node(node, make_ast_node(NODE_VALUE, 10, NULL, NO_OP));
    add_child_node(node, make_ast_node(NODE_VALUE, 20, NULL, NO_OP));
    return node;
}
