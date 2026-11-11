#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*
##########################################################
    ANALISADOR LÉXICO
##########################################################
*/

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
    FMT_STRING,
    End_Of_File,
    ERROR
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
    if (strcmp(str, "fn") == 0) return FUNCTION;
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

    lexema[i] = '\0';

    // Lookahed
    if(c != EOF) {
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

    lexema[i] = '\0';

    if(isalpha(c)) {
            do {
                c = fgetc(file);
            } while (c == ' ' || c == '\t' || c == '\r');
            ungetc(c, file);
            return makeToken(ERROR, "Número seguido de letra");
        }


    if(lexema[i - 1] == '.') {
        ungetc(c, file);
        return makeToken(ERROR, "Número com ponto decimal no final");
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

    while ((c = fgetc(file)) != EOF && c != '"') {

        if(c == '\n') {
            line++;
            return makeToken(ERROR, "String não fechada");
        }

        if (i < sizeof(lexema) - 1) {
            lexema[i++] = (char)c;
        }
    }

    lexema[i] = '\0';

    c = fgetc(file);

    if(c != EOF) {
        ungetc(c, file);
    }

    return makeToken(FMT_STRING, lexema);
}

Token lerChar(FILE *file){
    char lexema[128];
    int c;
    int i = 0;

    c = fgetc(file);
    if(c == '\n' || c == EOF) {
        line++;
        return makeToken(ERROR, "Char não fechado");
    }

    if (c == '\'') {
        fgetc(file); // Consumir o '
        return makeToken(CHAR_LITERAL, "");
    }

    lexema[i++] = (char)c;

    // Consumir o '
    if((c = fgetc(file)) != '\'') {
        if(c == '\n') {
            line++;
            return makeToken(ERROR, "Char não fechado");
        }
        ungetc(c, file);
        return makeToken(ERROR, "Char não fechado");
    }

    c = fgetc(file);
    if(c != EOF) {
        ungetc(c, file);
    }

    lexema[i] = '\0';
    return makeToken(CHAR_LITERAL, lexema);
}

Token errorToken(char lexema) {
    fprintf(stderr, "Erro léxico na linha %d: token inválido '%c'\n", line, lexema);
    return makeToken(ERROR, &lexema);
}

Token nextToken(FILE *file) {
    int c;
    int next = 0;

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

        if (c == '\''){
            return lerChar(file);
        }

        switch (c){
            case '{':
                return makeToken(LBRACE, "{");
            case '}':
                return makeToken(RBRACE, "}");
            case '(':
                return makeToken(LBRACKET, "(");
            case ')':
                return makeToken(RBRACKET, ")");
            case '-':
                next = fgetc(file);
                if (next == '>') {
                    return makeToken(ARROW, "->");
                } else {
                    if(next != EOF) {
                        ungetc(next, file);
                    }
                    return makeToken(MINUS, "-");
                }
            case ':':
                return makeToken(COLON, ":");
            case ';':
                return makeToken(SEMICOLON, ";");
            case ',':
                return makeToken(COMMA, ",");
            case '=':
                next = fgetc(file);
                if (next == '=') {
                    return makeToken(EQ, "==");
                } else {
                    if(next != EOF) {
                        ungetc(next, file);
                    }
                    return makeToken(ASSIGN, "=");
                }
            case '!':
                next = fgetc(file);
                if (next == '=') {
                    return makeToken(NE, "!=");
                } else {
                    if(next != EOF) {
                        ungetc(next, file);
                    }
                    return errorToken('!');
                }
            case '>':
                next = fgetc(file);
                if (next == '=') {
                    return makeToken(GE, ">=");
                } else {
                    if(next != EOF) {
                        ungetc(next, file);
                    }
                    return makeToken(GT, ">");
                }
            case '<':
                next = fgetc(file);
                if (next == '=') {
                    return makeToken(LE, "<=");
                } else {
                    if(next != EOF) {
                        ungetc(next, file);
                    }
                    return makeToken(LT, "<");
                }
            case '+':
                return makeToken(PLUS, "+");
            case '*':
                return makeToken(MULT, "*");
            case '/':
                return makeToken(DIV, "/");
            default:
                return errorToken((char)c);
        }
        
    }

    return makeToken(End_Of_File, "");
}

const char* nomeToken(TokenType t) {
    switch (t) {
        case FUNCTION: return "FUNCTION";
        case MAIN: return "MAIN";
        case LET: return "LET";
        case INT: return "INT";
        case FLOAT: return "FLOAT";
        case CHAR: return "CHAR";
        case IF: return "IF";
        case ELSE: return "ELSE";
        case WHILE: return "WHILE";
        case PRINTLN: return "PRINTLN";
        case RETURN: return "RETURN";
        case LBRACKET: return "LBRACKET";
        case RBRACKET: return "RBRACKET";
        case LBRACE: return "LBRACE";
        case RBRACE: return "RBRACE";
        case ARROW: return "ARROW";
        case COLON: return "COLON";
        case SEMICOLON: return "SEMICOLON";
        case COMMA: return "COMMA";
        case ASSIGN: return "ASSIGN";
        case EQ: return "EQ";
        case NE: return "NE";
        case GT: return "GT";
        case GE: return "GE";
        case LT: return "LT";
        case LE: return "LE";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case MULT: return "MULT";
        case DIV: return "DIV";
        case ID: return "ID";
        case INT_CONST: return "INT_CONST";
        case FLOAT_CONST: return "FLOAT_CONST";
        case CHAR_LITERAL: return "CHAR_LITERAL";
        case FMT_STRING: return "FMT_STRING";
        case ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

/*
##########################################################
    ANALISADOR SINTÁTICO
##########################################################
*/

static Token *tokens = NULL;      // vetor de tokens
static int totalTokens = 0;       // quantidade de tokens no vetor
static int atual = 0;             // token atual
 
static FILE *saidaErros = NULL;   // arquivo onde o erro sintático é gravado
 

void iniciarParser(Token *vetor, int total, FILE *arquivoErros) {
    tokens = vetor;
    totalTokens = total;
    atual = 0;
    saidaErros = arquivoErros;
}
 

Token *tokenAtual(void) {
    return &tokens[atual];
}
 
Token *proximoToken(void) {
    if (tokens[atual].type != End_Of_File && atual < totalTokens - 1) {
        atual++;
    }
    return &tokens[atual];
}
 

void erroSintatico(const char *esperado) {
    Token *t = tokenAtual();
    char mensagem[512];
 
    if (t->type == End_Of_File) {
        snprintf(mensagem, sizeof(mensagem),
                 "Erro sintático na linha %d: esperado %s, mas o arquivo terminou",
                 t->line, esperado);
    } else {
        snprintf(mensagem, sizeof(mensagem),
                 "Erro sintático na linha %d: esperado %s, mas encontrado %s '%s'",
                 t->line, esperado, nomeToken(t->type), t->lexeme);
    }
 
    if (saidaErros) {
        fprintf(saidaErros, "%s\n", mensagem);
        fclose(saidaErros);
    }
    fprintf(stderr, "%s\nCompilação interrompida.\n", mensagem);
    exit(1);
}

Token *casar(TokenType esperado) {
    Token *t = tokenAtual();
 
    if (t->type != esperado) {
        erroSintatico(nomeToken(esperado));
    }
 
    proximoToken();
    return t;
}

// Descida Recursiva

void programa(void){
    if(tokenAtual()->type == FUNCTION) {
        funcao();
        funcaoseq();
    }else{
        erroSintatico("FUNCTION");
    }
}

void funcao(void){
    casar(FUNCTION);
    if(tokenAtual()->type == ID || tokenAtual()->type == MAIN){
        nomefuncao();
    }
    casar(LBRACKET);
    if(tokenAtual()->type == ID) {
        listaparams();
    }
    casar(RBRACKET);
    if(tokenAtual()->type == ARROW) {
        TipoRetornoFuncao();
    }
    if(tokenAtual()->type == LBRACE) {
        Bloco();
    }else{
        erroSintatico("LBRACE");
    }
}

void listaparams(void){
    casar(ID);
    casar(COLON);
    if(tokenAtual()->type == INT || tokenAtual()->type == FLOAT || tokenAtual()->type == CHAR) {
        type();
    }else{
        erroSintatico("INT, FLOAT ou CHAR");
    }
    if(tokenAtual()->type == COMMA) {
        listaparams2();
    }
}

void listaparams2(void){
    casar(COMMA);
    casar(ID);
    casar(COLON);
    if(tokenAtual()->type == INT || tokenAtual()->type == FLOAT || tokenAtual()->type == CHAR) {
        type();
    }else{
        erroSintatico("INT, FLOAT ou CHAR");
    }
    if(tokenAtual()->type == COMMA) {
        listaparams2();
    }
}

void type(void){
    if(tokenAtual()->type == INT || tokenAtual()->type == FLOAT || tokenAtual()->type == CHAR) {
        casar(tokenAtual()->type);
    }else{
        erroSintatico("INT, FLOAT ou CHAR");
    }
}

void nomefuncao(void){
    if(tokenAtual()->type == ID) {
        casar(ID);
    }else if(tokenAtual()->type == MAIN) {
        casar(MAIN);
    }else{
        erroSintatico("ID ou MAIN");
    }
}

void funcaoseq(void){
    if(tokenAtual()->type == FUNCTION) {
        funcao();
        funcaoseq();
    }
}

/*
##########################################################
    MAIN
##########################################################
*/

#define MAX_TOKENS 1000

int main(){
    FILE *file = fopen("teste.txt", "r");
    Token token;
 
    Token vetorTokens[MAX_TOKENS];
    int index = 0;
 
    while((token = nextToken(file)).type != End_Of_File && index < MAX_TOKENS - 1) {
        vetorTokens[index++] = token;
        printf("Token: %s, Lexema: %s, Linha: %d\n",
        nomeToken(token.type),
        token.lexeme,
        token.line);
    }
 
    printf("Total de tokens: %d\n", index);
 
    if (token.type != End_Of_File) {
        fprintf(stderr, "Aviso: limite de %d tokens atingido, o restante do arquivo foi ignorado\n", MAX_TOKENS - 1);
    }
    vetorTokens[index++] = makeToken(End_Of_File, "");

    fclose(file);

    int numErrosLexicos = 0;
    for (int i = 0; i < index; i++) {
        if (vetorTokens[i].type == ERROR) {
            numErrosLexicos++;
        }
    }
    if (numErrosLexicos > 0) {
        fprintf(stderr, "Foram encontrados %d erro(s) léxico(s). Compilação interrompida.\n", numErrosLexicos);
        return 1;
    }
 
    // Análise sintática
    FILE *arquivoErros = fopen("saida_erros_sintaticos.txt", "w");
    iniciarParser(vetorTokens, index, arquivoErros);

 
    if (arquivoErros) {
        fprintf(arquivoErros, "Nenhum erro sintático encontrado.\n");
        fclose(arquivoErros);
    }
 
    return 0;
}
