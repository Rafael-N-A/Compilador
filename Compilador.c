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

// Declaração das funções correspondentes a cada produção da gramática

void programa(void);
void funcaoseq(void);
void funcao(void);
void nomefuncao(void);
void listaparams(void);
void listaparams2(void);
void tiporetornofuncao(void);
void bloco(void);
void sequencia(void);
void declaracao(void);
void varlist(void);
void varlist2(void);
void type(void);
void comando(void);
void atribuicaoouchamada(void);
void comandose(void);
void comandosenao(void);
void expr(void);
void expropc(void);
void opigual(void);
void rel(void);
void relopc(void);
void oprel(void);
void adicao(void);
void adicaoopc(void);
void opadicao(void);
void termo(void);
void termoopc(void);
void opmult(void);
void fator(void);
void chamadafuncao(void);
void listaargs(void);
void listaargs2(void);
void arg(void);

// Mensagens de erro para reuso
#define ESPERADO_EXPR "ID, INT_CONST, FLOAT_CONST, CHAR_LITERAL ou LBRACKET"
#define ESPERADO_ARG  "ID, INT_CONST, FLOAT_CONST ou CHAR_LITERAL"

// Conjunto First(Comando) = { ID, IF, LBRACE, WHILE, PRINTLN, RETURN }
// Para reuso
int firstComando(void){
    TokenType t = tokenAtual()->type;
    return t == ID || t == IF || t == LBRACE || t == WHILE || t == PRINTLN || t == RETURN;
}

// Conjunto First(Expr) = First(Rel) = First(Adicao) = First(Termo) = First(Fator)
//             = { ID, INT_CONST, FLOAT_CONST, CHAR_LITERAL, LBRACKET }
// Para reuso
int firstExpr(void){
    TokenType t = tokenAtual()->type;
    return t == ID || t == INT_CONST || t == FLOAT_CONST || t == CHAR_LITERAL || t == LBRACKET;
}

// Conjunto First(Arg) = { ID, INT_CONST, FLOAT_CONST, CHAR_LITERAL }
// Para reuso
int firstArg(void){
    TokenType t = tokenAtual()->type;
    return t == ID || t == INT_CONST || t == FLOAT_CONST || t == CHAR_LITERAL;
}

// Programa → Funcao FuncaoSeq
void programa(void){
    if(tokenAtual()->type == FUNCTION) {
        funcao();
        funcaoseq();
    }else{
        erroSintatico("FUNCTION");
    }
    if(tokenAtual()->type != End_Of_File) {
        erroSintatico("FUNCTION ou fim do arquivo");
    }
}

// Funcao → fn NomeFuncao ( ListaParams ) TipoRetornoFuncao Bloco
void funcao(void){
    casar(FUNCTION);
    if(tokenAtual()->type == ID || tokenAtual()->type == MAIN){
        nomefuncao();
    }else{
        erroSintatico("ID ou MAIN");
    }
    casar(LBRACKET);
    if(tokenAtual()->type == ID) {
        listaparams();
    }
    casar(RBRACKET);
    if(tokenAtual()->type == ARROW) {
        tiporetornofuncao();
    }
    if(tokenAtual()->type == LBRACE) {
        bloco();
    }else{
        erroSintatico("LBRACE");
    }
}

// ListaParams → ID : Type ListaParams2 | ε   
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

// ListaParams2 → , ID : Type ListaParams2 | ε
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

// Type → int | float | char
void type(void){
    if(tokenAtual()->type == INT || tokenAtual()->type == FLOAT || tokenAtual()->type == CHAR) {
        casar(tokenAtual()->type);
    }else{
        erroSintatico("INT, FLOAT ou CHAR");
    }
}

// NomeFuncao → ID | MAIN
void nomefuncao(void){
    if(tokenAtual()->type == ID) {
        casar(ID);
    }else if(tokenAtual()->type == MAIN) {
        casar(MAIN);
    }else{
        erroSintatico("ID ou MAIN");
    }
}

// FuncaoSeq → Funcao FuncaoSeq | ε
void funcaoseq(void){
    if(tokenAtual()->type == FUNCTION) {
        funcao();
        funcaoseq();
    }
}

// TipoRetornoFuncao → -> Type | ε   
void tiporetornofuncao(void){
    casar(ARROW);
    if(tokenAtual()->type == INT || tokenAtual()->type == FLOAT || tokenAtual()->type == CHAR) {
        type();
    }else{
        erroSintatico("INT, FLOAT ou CHAR");
    }
}

// Bloco → { Sequencia }
void bloco(void){
    casar(LBRACE);
    sequencia();
    if(tokenAtual()->type == RBRACE) {
        casar(RBRACE);
    }else{
        erroSintatico("LET, ID, IF, LBRACE, WHILE, PRINTLN, RETURN ou RBRACE");
    }
}

// Sequencia → Declaracao Sequencia | Comando Sequencia | ε
void sequencia(void){
    if(tokenAtual()->type == LET) {
        declaracao();
        sequencia();
    }else if(firstComando()) {
        comando();
        sequencia();
    }
}

// Declaracao → let VarList : Type ;
void declaracao(void){
    casar(LET);
    if(tokenAtual()->type == ID) {
        varlist();
    }else{
        erroSintatico("ID");
    }
    casar(COLON);
    if(tokenAtual()->type == INT || tokenAtual()->type == FLOAT || tokenAtual()->type == CHAR) {
        type();
    }else{
        erroSintatico("INT, FLOAT ou CHAR");
    }
    casar(SEMICOLON);
}

// VarList → ID VarList2
void varlist(void){
    casar(ID);
    if(tokenAtual()->type == COMMA) {
        varlist2();
    }
}

// VarList2 → , ID VarList2 | ε
void varlist2(void){
    casar(COMMA);
    casar(ID);
    if(tokenAtual()->type == COMMA) {
        varlist2();
    }
}

// Comando → ID AtribuicaoOuChamada
//         | ComandoSe
//         | while Expr Bloco
//         | println ( FMT_STRING , ListaArgs ) ;
//         | return Expr ;
void comando(void){
    if(tokenAtual()->type == ID) {
        casar(ID);
        if(tokenAtual()->type == ASSIGN || tokenAtual()->type == LBRACKET) {
            atribuicaoouchamada();
        }else{
            erroSintatico("ASSIGN ou LBRACKET");
        }
    }else if(tokenAtual()->type == IF || tokenAtual()->type == LBRACE) {
        comandose();
    }else if(tokenAtual()->type == WHILE) {
        casar(WHILE);
        if(firstExpr()) {
            expr();
        }else{
            erroSintatico(ESPERADO_EXPR);
        }
        if(tokenAtual()->type == LBRACE) {
            bloco();
        }else{
            erroSintatico("LBRACE");
        }
    }else if(tokenAtual()->type == PRINTLN) {
        casar(PRINTLN);
        casar(LBRACKET);
        casar(FMT_STRING);
        casar(COMMA);
        if(firstArg()) {
            listaargs();
        }
        casar(RBRACKET);
        casar(SEMICOLON);
    }else if(tokenAtual()->type == RETURN) {
        casar(RETURN);
        if(firstExpr()) {
            expr();
        }else{
            erroSintatico(ESPERADO_EXPR);
        }
        casar(SEMICOLON);
    }else{
        erroSintatico("ID, IF, LBRACE, WHILE, PRINTLN ou RETURN");
    }
}

// AtribuicaoOuChamada → = Expr ; | ( ListaArgs ) ;
void atribuicaoouchamada(void){
    if(tokenAtual()->type == ASSIGN) {
        casar(ASSIGN);
        if(firstExpr()) {
            expr();
        }else{
            erroSintatico(ESPERADO_EXPR);
        }
        casar(SEMICOLON);
    }else if(tokenAtual()->type == LBRACKET) {
        casar(LBRACKET);
        if(firstArg()) {
            listaargs();
        }
        casar(RBRACKET);
        casar(SEMICOLON);
    }else{
        erroSintatico("ASSIGN ou LBRACKET");
    }
}

// ComandoSe → if Expr Bloco ComandoSenao | Bloco
void comandose(void){
    if(tokenAtual()->type == IF) {
        casar(IF);
        if(firstExpr()) {
            expr();
        }else{
            erroSintatico(ESPERADO_EXPR);
        }
        if(tokenAtual()->type == LBRACE) {
            bloco();
        }else{
            erroSintatico("LBRACE");
        }
        if(tokenAtual()->type == ELSE) {
            comandosenao();
        }
    }else if(tokenAtual()->type == LBRACE) {
        bloco();
    }else{
        erroSintatico("IF ou LBRACE");
    }
}

// ComandoSenao → else ComandoSe | ε
void comandosenao(void){
    casar(ELSE);
    if(tokenAtual()->type == IF || tokenAtual()->type == LBRACE) {
        comandose();
    }else{
        erroSintatico("IF ou LBRACE");
    }
}

// Expr → Rel ExprOpc
void expr(void){
    rel();
    if(tokenAtual()->type == EQ || tokenAtual()->type == NE) {
        expropc();
    }
}

// ExprOpc → OpIgual Rel ExprOpc | ε
void expropc(void){
    opigual();
    if(firstExpr()) {
        rel();
    }else{
        erroSintatico(ESPERADO_EXPR);
    }
    if(tokenAtual()->type == EQ || tokenAtual()->type == NE) {
        expropc();
    }
}

// OpIgual → == | !=
void opigual(void){
    if(tokenAtual()->type == EQ || tokenAtual()->type == NE) {
        casar(tokenAtual()->type);
    }else{
        erroSintatico("EQ ou NE");
    }
}

// Rel → Adicao RelOpc
void rel(void){
    adicao();
    if(tokenAtual()->type == LT || tokenAtual()->type == LE || tokenAtual()->type == GT || tokenAtual()->type == GE) {
        relopc();
    }
}

// RelOpc → OpRel Adicao RelOpc | ε
void relopc(void){
    oprel();
    if(firstExpr()) {
        adicao();
    }else{
        erroSintatico(ESPERADO_EXPR);
    }
    if(tokenAtual()->type == LT || tokenAtual()->type == LE || tokenAtual()->type == GT || tokenAtual()->type == GE) {
        relopc();
    }
}

// OpRel → < | <= | > | >=
void oprel(void){
    if(tokenAtual()->type == LT || tokenAtual()->type == LE || tokenAtual()->type == GT || tokenAtual()->type == GE) {
        casar(tokenAtual()->type);
    }else{
        erroSintatico("LT, LE, GT ou GE");
    }
}

// Adicao → Termo AdicaoOpc
void adicao(void){
    termo();
    if(tokenAtual()->type == PLUS || tokenAtual()->type == MINUS) {
        adicaoopc();
    }
}

// AdicaoOpc → OpAdicao Termo AdicaoOpc | ε
void adicaoopc(void){
    opadicao();
    if(firstExpr()) {
        termo();
    }else{
        erroSintatico(ESPERADO_EXPR);
    }
    if(tokenAtual()->type == PLUS || tokenAtual()->type == MINUS) {
        adicaoopc();
    }
}

// OpAdicao → + | -
void opadicao(void){
    if(tokenAtual()->type == PLUS || tokenAtual()->type == MINUS) {
        casar(tokenAtual()->type);
    }else{
        erroSintatico("PLUS ou MINUS");
    }
}

// Termo → Fator TermoOpc
void termo(void){
    fator();
    if(tokenAtual()->type == MULT || tokenAtual()->type == DIV) {
        termoopc();
    }
}

// TermoOpc → OpMult Fator TermoOpc | ε
void termoopc(void){
    opmult();
    if(firstExpr()) {
        fator();
    }else{
        erroSintatico(ESPERADO_EXPR);
    }
    if(tokenAtual()->type == MULT || tokenAtual()->type == DIV) {
        termoopc();
    }
}

// OpMult → * | /
void opmult(void){
    if(tokenAtual()->type == MULT || tokenAtual()->type == DIV) {
        casar(tokenAtual()->type);
    }else{
        erroSintatico("MULT ou DIV");
    }
}

// Fator → ID ChamadaFuncao | INT_CONST | FLOAT_CONST | CHAR_LITERAL | ( Expr )
void fator(void){
    if(tokenAtual()->type == ID) {
        casar(ID);
        if(tokenAtual()->type == LBRACKET) {
            chamadafuncao();
        }
    }else if(tokenAtual()->type == INT_CONST) {
        casar(INT_CONST);
    }else if(tokenAtual()->type == FLOAT_CONST) {
        casar(FLOAT_CONST);
    }else if(tokenAtual()->type == CHAR_LITERAL) {
        casar(CHAR_LITERAL);
    }else if(tokenAtual()->type == LBRACKET) {
        casar(LBRACKET);
        if(firstExpr()) {
            expr();
        }else{
            erroSintatico(ESPERADO_EXPR);
        }
        casar(RBRACKET);
    }else{
        erroSintatico(ESPERADO_EXPR);
    }
}

// ChamadaFuncao → ( ListaArgs ) | ε
void chamadafuncao(void){
    casar(LBRACKET);
    if(firstArg()) {
        listaargs();
    }
    casar(RBRACKET);
}

// ListaArgs → Arg ListaArgs2 | ε
void listaargs(void){
    arg();
    if(tokenAtual()->type == COMMA) {
        listaargs2();
    }
}

// ListaArgs2 → , Arg ListaArgs2 | ε
void listaargs2(void){
    casar(COMMA);
    if(firstArg()) {
        arg();
    }else{
        erroSintatico(ESPERADO_ARG);
    }
    if(tokenAtual()->type == COMMA) {
        listaargs2();
    }
}

// Arg → ID ChamadaFuncao | INT_CONST | FLOAT_CONST | CHAR_LITERAL
void arg(void){
    if(tokenAtual()->type == ID) {
        casar(ID);
        if(tokenAtual()->type == LBRACKET) {
            chamadafuncao();
        }
    }else if(tokenAtual()->type == INT_CONST) {
        casar(INT_CONST);
    }else if(tokenAtual()->type == FLOAT_CONST) {
        casar(FLOAT_CONST);
    }else if(tokenAtual()->type == CHAR_LITERAL) {
        casar(CHAR_LITERAL);
    }else{
        erroSintatico(ESPERADO_ARG);
    }
}

/*
##########################################################
    TABELA DE SÍMBOLOS
##########################################################
*/


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
    programa();   // se houver erro sintático, o erroSintatico() encerra o programa aqui dentro


    if (arquivoErros) {
        fprintf(arquivoErros, "Nenhum erro sintático encontrado.\n");
        fclose(arquivoErros);
    }
 
    return 0;
}