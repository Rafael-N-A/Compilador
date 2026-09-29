#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    FUNCTION,
    MAIN,
    LET,
    INT,
    FLOAT,
    CHAR,
    IF,
    ELSE,
    WHILE,
    PRINTLN,
    RETURN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,
    ARROW,
    COLON,
    SEMICOLON,
    COMMA,
    ASSIGN,
    EQ,
    NE,
    GT,
    GE,
    LT,
    LE,
    PLUS,
    MINUS,
    MULT,
    DIV,
    ID,
    INT_CONST,
    FLOAT_CONST,
    CHAR_LITERAL,
    FMT_STRING
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[128];
    int line;
} Token;

static int line = 1;

TokenType identificadorToken(const char* str) {
    if (strcmp(str, "function") == 0) return FUNCTION;
    if (strcmp(str, "main") == 0) return MAIN;
    if (strcmp(str, "let") == 0) return LET;
    if (strcmp(str, "int") == 0) return INT;
    if (strcmp(str, "float") == 0) return FLOAT;
    if (strcmp(str, "char") == 0) return CHAR;
    if (strcmp(str, "if") == 0) return IF;
    if (strcmp(str, "else") == 0) return ELSE;
    if (strcmp(str, "while") == 0) return WHILE;
    if (strcmp(str, "println") == 0) return PRINTLN;
    if (strcmp(str, "return") == 0) return RETURN;

    // Se não for uma palavra reservada, retorna ID
    return ID;
}

Token makeToken(TokenType type, const char* lexeme, int line) {
    Token token;
    
    token.type = type;
    // Garantir que o lexema não exceda o tamanho do buffer 128
    strncpy(token.lexeme, lexeme, sizeof(token.lexeme) - 1);
    // Garantir terminação nula
    token.lexeme[sizeof(token.lexeme) - 1] = '\0';
    token.line = line;
    return token;
}

Token lerIdenficador(FILE *file, int first, int line) {
    char lexema[128];
    int c;
    int i = 0;

    lexema[i++] = (char)first;

    while ((c = fgetc(file)) != EOF && (isalnum(c) || c == '_')) {
        if (i < sizeof(lexema) - 1) { // Evitar estouro de buffer
            lexema[i++] = (char)c;
        }
    }

    // Lookahed
    if(c  != EOF) {
        ungetc(c, file);
    }

    TokenType type = identificadorToken(lexema);
    return makeToken(type, lexema, line);
}