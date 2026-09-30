#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Definição dos tokens

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

// Token

typedef struct {
    TokenType type;
    char lexeme[128];
    int line;
} Token;

static int line = 1;

// Identificador de palavra reservada

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

    return ID;
}

// Função cria token

Token makeToken(TokenType type, const char* lexeme) {
    Token token;
    
    token.type = type;
    strncpy(token.lexeme, lexeme, sizeof(token.lexeme) - 1);
    token.lexeme[sizeof(token.lexeme) - 1] = '\0';
    token.line = line;
    return token;
}

Token lerIdenficador(FILE *file, int first) {
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
    return makeToken(type, lexema);
}

Token lerNumero(FILE *file, int first) {
    char lexema[128];
    int c;
    int i = 0;
    int pontoDecimal = 0;

    lexema[i++] = (char)first;

    while ((c = fgetc(file)) != EOF && (isdigit(c) || c == '.')) {
        if (c == '.') {
            if (pontoDecimal) {
                break;
            }
            pontoDecimal = 1;
        }
        if (i < sizeof(lexema) - 1) {
            lexema[i++] = (char)c;
        }
    }

    if(c != EOF) {
        ungetc(c, file);
    }

    TokenType type = pontoDecimal ? FLOAT_CONST : INT_CONST;
    return makeToken(type, lexema);
}

Token lerString(FILE *file){
    char lexema[128];
    int c;
    int i = 0;

    while ((c = fgetc(file)) != EOF && c != '"' && c != '\n') {
        if (i < sizeof(lexema) - 1) {
            lexema[i++] = (char)c;
        }
    }

    if(c != EOF) {
        ungetc(c, file);
    }

    lexema[i] = '\0';
    return makeToken(FMT_STRING, lexema);
}

Token lerChar(FILE *file){
    char lexema[128];
    int c;
    int i = 0;

    c = fgetc(file);
    if (c == EOF || c == '\n' || c == '\'') {
        return makeToken(CHAR_LITERAL, "");
    }

    lexema[i++] = (char)c;

    c = fgetc(file);
    if(c != EOF) {
        ungetc(c, file);
    }

    lexema[i] = '\0';
    return makeToken(CHAR_LITERAL, lexema);
}

Token nextToken(FILE *file) {
    int c;
    char lexema[128];
    int i = 0;

    while ((c = fgetc(file)) != EOF) {
        
        if(c == ' ' || c == '\t' || c == '\r') {
            continue;
        }
        
        if (c == '\n') {
            line++;
            continue;
        }

        if (isalpha(c)) {
            return lerIdenficador(file, c);
        }

        if (isdigit(c)) {
            return lerNumero(file, c);
        }

        if (c == '"') {
            return lerString(file);
        }

        if (c == "'"){
            return lerChar(file);
        }

        // Tratar operadores e símbolos
        // ... (resto da implementação)
    }

    return makeToken(EOF, "");
}