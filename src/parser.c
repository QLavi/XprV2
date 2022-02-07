#include <stdbool.h>

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

void consume(Token_Type type, char* error_descr) {
    if(parser.curr_token.type == type) {
        parser.curr_token = next_token();
    }
    else {
        fprintf(stderr, "%s\n", error_descr);
    }
}

bool match_token(Token_Type type) {
    return parser.curr_token.type == type;
}

AST_Node* make_ast_node(Node_Type type, Value value, char* chars, Op_Type op_type) {
    AST_Node* node = ALLOC(AST_Node, 1);
    node->type = type;

    if(type == NODE_VALUE) {
        node->value = value;
    }
    else if(type == NODE_LOAD || type == NODE_STORE) {
        node->id = chars;
    }
    else if(type == NODE_BINARY_OP || type == NODE_UNARY_OP) {
        node->op_type = op_type;
    }
    else if(type == NODE_IF) {
        node->jump_id = op_type;
    }
    else {
        node->id = 0; // Zero out the union members
    }
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

    for(int x = 0; x < node->count; x++) {
        free_ast(node->child_nodes[x]);
    }
    FREE(AST_Node*, node->child_nodes);
    FREE(AST_Node, node);
}

typedef enum {
    PREC_NONE,
    PREC_ASSIGN,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
} Precedence;

AST_Node* group(void);
AST_Node* binary(AST_Node*);
AST_Node* unary(void);
AST_Node* identi(void);
AST_Node* number(void);

typedef AST_Node* (*Prefix_Fn)(void);
typedef AST_Node* (*Infix_Fn)(AST_Node*);
typedef struct {
    Prefix_Fn prefix_rule;
    Infix_Fn infix_rule;
    Precedence precedence;
} Parse_Rule;

Parse_Rule rules[] = {
    [TOKEN_LEFT_BRACE]      = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE]     = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_PAREN]      = {group, NULL, PREC_NONE},
    [TOKEN_RIGHT_PAREN]     = {NULL, NULL, PREC_NONE},
    [TOKEN_PLUS]            = {NULL, binary, PREC_TERM},
    [TOKEN_MINUS]           = {unary, binary, PREC_TERM},
    [TOKEN_STAR]            = {NULL, binary, PREC_FACTOR},
    [TOKEN_SLASH]           = {NULL, binary, PREC_FACTOR},
    [TOKEN_LESS]            = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL]      = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER]         = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL]   = {NULL, binary, PREC_COMPARISON},
    [TOKEN_BANG]            = {unary, NULL, PREC_NONE},
    [TOKEN_BANG_EQUAL]      = {NULL, binary, PREC_EQUALITY},
    [TOKEN_EQUAL]           = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL]     = {NULL, binary, PREC_EQUALITY},
    [TOKEN_NUMBER]          = {number, NULL, PREC_NONE},
    [TOKEN_IDENTIFIER]      = {identi, NULL, PREC_NONE},
    [TOKEN_IF]              = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE]            = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE]           = {NULL, NULL, PREC_NONE},
    [TOKEN_LOOP]            = {NULL, NULL, PREC_NONE},
    [TOKEN_SEMICOLON]       = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR]           = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF]             = {NULL, NULL, PREC_NONE},
};

AST_Node* expression(Precedence precedence) {
    parser.prev_token = parser.curr_token;
    parser.curr_token = next_token();
    AST_Node* node = rules[parser.prev_token.type].prefix_rule();

    while(precedence < rules[parser.curr_token.type].precedence) {
        parser.prev_token = parser.curr_token;
        parser.curr_token = next_token();
        node = rules[parser.prev_token.type].infix_rule(node);
    }
    return node;
}

AST_Node* binary(AST_Node* node) {
    AST_Node* parent = make_ast_node(NODE_BINARY_OP, 0, NULL, NO_OP);
    switch(parser.prev_token.type) {
        case TOKEN_PLUS:
            parent->op_type = ADD_OP;
            add_child_node(parent, node);
            add_child_node(parent, expression(PREC_TERM));
            break;
        case TOKEN_MINUS:
            parent->op_type = SUB_OP;
            add_child_node(parent, node);
            add_child_node(parent, expression(PREC_TERM));
            break;
        case TOKEN_STAR:
            parent->op_type = MUL_OP;
            add_child_node(parent, node);
            add_child_node(parent, expression(PREC_FACTOR));
            break;
        case TOKEN_SLASH:
            parent->op_type = DIV_OP;
            add_child_node(parent, node);
            add_child_node(parent, expression(PREC_FACTOR));
            break;
        case TOKEN_LESS:
            parent->op_type = LESS_OP;
            add_child_node(parent, node);
            add_child_node(parent, expression(PREC_COMPARISON));
            break;
        case TOKEN_LESS_EQUAL:
            parent->op_type = LESS_EQUAL_OP;
            add_child_node(parent, node);
            add_child_node(parent, expression(PREC_COMPARISON));
            break;
        case TOKEN_GREATER:
            parent->op_type = GREATER_OP,
            add_child_node(parent, node);
            add_child_node(parent, expression(PREC_COMPARISON));
            break;
        case TOKEN_GREATER_EQUAL:
            parent->op_type = GREATER_EQUAL_OP;
            add_child_node(parent, node);
            add_child_node(parent, expression(PREC_COMPARISON));
            break;
        case TOKEN_EQUAL_EQUAL:
            parent->op_type = EQUAL_OP;
            add_child_node(parent, node);
            add_child_node(parent, expression(PREC_EQUALITY));
            break;
        case TOKEN_BANG_EQUAL:
            parent->op_type = NOT_EQUAL_OP;
            add_child_node(parent, node);
            add_child_node(parent, expression(PREC_EQUALITY));
            break;
        default: return NULL;
    }
    return parent;
}

AST_Node* unary(void) {
    AST_Node* parent = make_ast_node(NODE_UNARY_OP, 0, NULL, NO_OP);
    switch(parser.prev_token.type) {
        case TOKEN_BANG:
            parent->op_type = NOT_OP;
            add_child_node(parent, expression(PREC_UNARY));
            break;
        case TOKEN_MINUS:
            parent->op_type = NEGATE_OP;
            add_child_node(parent, expression(PREC_UNARY));
            break;
        default: return NULL;
    }
}

AST_Node* group(void) {
    AST_Node* node = expression(PREC_NONE);
    consume(TOKEN_RIGHT_PAREN, "Incomplete Set of Parentheses");
    return node;
}

AST_Node* number(void) {
    Value value = strtod(parser.prev_token.chars, NULL);
    return make_ast_node(NODE_VALUE, value, NULL, NO_OP);
}

AST_Node* identi(void) {
    char* id = parser.prev_token.chars;
    return make_ast_node(NODE_LOAD, 0, id, NO_OP);
}

AST_Node* print_statement(void) {
    AST_Node* node = make_ast_node(NODE_PRINT, 0, NULL, NO_OP);
    consume(TOKEN_PRINT, "");
    add_child_node(node, expression(PREC_NONE));
    consume(TOKEN_SEMICOLON, "missing `;` after expression");
    return node;
}

AST_Node* assign_statement(void) {
    char* id = parser.curr_token.chars;
    AST_Node* node = make_ast_node(NODE_STORE, 0, id, NO_OP);
    consume(TOKEN_IDENTIFIER, "");
    consume(TOKEN_EQUAL, "missing `=` after var-id");

    add_child_node(node, expression(PREC_NONE));
    consume(TOKEN_SEMICOLON, "missing `;` after expression");
    return node;
}

AST_Node* if_statement(void);

AST_Node* block(void) {
    AST_Node* list = make_ast_node(NODE_STATEMENT_LIST, 0, NULL, NO_OP);
    while(!match_token(TOKEN_RIGHT_BRACE)) {

        if(match_token(TOKEN_IDENTIFIER)) {
            AST_Node* node = assign_statement();
            add_child_node(list, assign_statement());
        }
        else if(match_token(TOKEN_IF)) {
            add_child_node(list, if_statement());
        }
        else if(match_token(TOKEN_PRINT)) {
            add_child_node(list, print_statement());
        }
    }
    return list;
}

int em_id = 0;
AST_Node* if_statement(void) {
    AST_Node* parent = make_ast_node(NODE_IF, 0, NULL, em_id++);
    consume(TOKEN_IF, "");

    consume(TOKEN_LEFT_PAREN, "Missing `(` after `if` token");
    add_child_node(parent, expression(PREC_NONE));
    consume(TOKEN_RIGHT_PAREN, "Missing `)` after condition expression");

    consume(TOKEN_LEFT_BRACE, "`{` is mandatory after if statement");
    add_child_node(parent, block());
    consume(TOKEN_RIGHT_BRACE, "`}` is missing after if block");

    return parent;
}

AST_Node* statement(void) {
    AST_Node* parent = make_ast_node(NODE_STATEMENT_LIST, 0, NULL, NO_OP);

    while(!match_token(TOKEN_EOF)) {
        if(match_token(TOKEN_IF)) {
            add_child_node(parent, if_statement());
        }
        else if(match_token(TOKEN_IDENTIFIER)) {
            add_child_node(parent, assign_statement());
        }
        else if(match_token(TOKEN_PRINT)) {
            add_child_node(parent, print_statement());
        }
    }
    return parent;
}
