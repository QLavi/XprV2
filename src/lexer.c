#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "lexer.h"

typedef struct {
    char* start;
    char* current;
} Lexer;

Lexer lexer;
#define CURR_CHAR() (*lexer.current)
#define ADVANCE() (lexer.current += 1)

void init_lexer(char* src) {
    lexer.start = lexer.current = src;
}

Token make_token(Token_Type type) {
    Token token;
    token.type = type;
    token.chars = lexer.start;
    token.length = (int)(lexer.current - lexer.start);
    return token;
}

Token error_token(char* error_descr) {
    Token token;
    token.type = TOKEN_ERROR;
    token.chars = error_descr;
    token.length = (int)(strlen(error_descr));
    return token;
}

bool match(char c) {
    if(CURR_CHAR() == c) {
        ADVANCE();
        return true;
    }
    return false;
}

void ignore_whitespace(void) {
    for(;;) {
        switch(CURR_CHAR()) {
            case '\t':
            case '\n':
            case ' ':
                ADVANCE();
                break;
            case '#':
                while(CURR_CHAR() != '\n') ADVANCE();
                break;
            default:
                return;
        }
    }
}

bool check_keyword(int start, int end, char* str) {
    return lexer.current - lexer.start == start + end && memcmp(lexer.start + start, str, end) == 0;
}

Token identifier_or_keyword(void) {
    while(isalpha(CURR_CHAR()) || isdigit(CURR_CHAR()) || CURR_CHAR() == '_') ADVANCE();

    switch(*lexer.start) {
        case 'i':
            if(check_keyword(1, 1, "f")) return make_token(TOKEN_IF);
        case 'w':
            if(check_keyword(1, 4, "hile")) return make_token(TOKEN_WHILE);
        case 'l':
            if(check_keyword(1, 3, "oop")) return make_token(TOKEN_LOOP);
        case 'p':
            if(check_keyword(1, 4, "rint")) return make_token(TOKEN_PRINT);
    }
    return make_token(TOKEN_IDENTIFIER);
}

Token number_token(void) {
    while(isdigit(CURR_CHAR())) ADVANCE();
    if(CURR_CHAR() == '.') ADVANCE();
    while(isdigit(CURR_CHAR())) ADVANCE();
    return make_token(TOKEN_NUMBER);
}

Token next_token(void) {
    ignore_whitespace();

    if(CURR_CHAR() == '\0') {
        return make_token(TOKEN_EOF);
    }
    lexer.start = lexer.current;

    char c = CURR_CHAR();
    ADVANCE();

    if(isalpha(c)) {
        return identifier_or_keyword();
    }
    if(isdigit(c)) {
        return number_token();
    }

    switch(c) {
        case '(': return make_token(TOKEN_LEFT_PAREN);
        case ')': return make_token(TOKEN_RIGHT_PAREN);
        case '{': return make_token(TOKEN_LEFT_BRACE);
        case '}': return make_token(TOKEN_RIGHT_BRACE);
        case '+': return make_token(match('=') ? TOKEN_PLUS_EQUAL : TOKEN_PLUS);
        case '-': return make_token(match('=') ? TOKEN_MINUS_EQUAL: TOKEN_MINUS);
        case '*': return make_token(match('=') ? TOKEN_STAR_EQUAL : TOKEN_STAR);
        case '/': return make_token(match('=') ? TOKEN_SLASH_EQUAL: TOKEN_SLASH);
        case '<': return make_token(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>': return make_token(match('=') ? TOKEN_GREATER_EQUAL: TOKEN_GREATER);
        case '=': return make_token(match('=') ? TOKEN_EQUAL_EQUAL: TOKEN_EQUAL);
        case '!': return make_token(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case ';': return make_token(TOKEN_SEMICOLON);
        default:
            return error_token("UnRecognized Character Found");
    }
}
