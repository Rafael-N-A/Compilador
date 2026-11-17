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
    TABELA DE SÍMBOLOS
##########################################################
*/

// Definida na seção da análise semântica
void erroSemantico(Token *token, const char *mensagem);

typedef enum {
    DATA_ERROR = -1,
    DATA_INT,
    DATA_FLOAT,
    DATA_CHAR,
    DATA_VOID
}DataTypes;

DataTypes tipoDoToken(TokenType t){
    switch(t){
        case INT:
        case INT_CONST:
            return DATA_INT;
        case FLOAT:
        case FLOAT_CONST:
            return DATA_FLOAT;
        case CHAR:
        case CHAR_LITERAL:
            return DATA_CHAR;
        default:
            return DATA_ERROR;
    }
}
 
#define MAX_ARGS 32
 
typedef struct FunctionRegister {
    char name[128];                  // nome da função chamada
    int line;                        // linha da chamada (para as mensagens de erro)
    int num_args;
    char args[MAX_ARGS][128];        // lexema de cada argumento
    struct FunctionRegister *prox;   // próxima chamada da mesma tabela
} FunctionRegister;
 
typedef struct Simbolo {
    char name[128];                  // chave (lexema)
    DataTypes datatype;
    int is_param;
    int pos_param;                   // -1 para variável local
    FunctionRegister **call_refs;    // ponteiros para as chamadas desta função (NULL se não houver)
    int num_refs;
    struct Simbolo *prox;
} Simbolo;
 
typedef struct TabelaSimbolos {
    char nomeFuncao[128];
    DataTypes ret_type;              // DATA_VOID se não tiver "-> Type"
    Simbolo *simbolos;               // variáveis, parâmetros e funções chamadas
    FunctionRegister *chamadas;      // todas as chamadas, na ordem em que aparecem
    struct TabelaSimbolos *prox;     // tabela da próxima função
} TabelaSimbolos;

static TabelaSimbolos *listaTabelas = NULL;   // Primeira tabela da lista (uma por função, na ordem do código)
static TabelaSimbolos *tabelaAtual = NULL;    // Tabela Atual
 
// Cria a tabela de uma nova função e coloca na lista de tabelas.
TabelaSimbolos *abrirEscopo(const char *nomeFuncao){
    TabelaSimbolos *nova = malloc(sizeof(TabelaSimbolos));
    if(nova == NULL) {
        fprintf(stderr, "Erro: memória insuficiente para criar a tabela de símbolos\n");
        exit(1);
    }
 
    strncpy(nova->nomeFuncao, nomeFuncao, sizeof(nova->nomeFuncao) - 1);
    nova->nomeFuncao[sizeof(nova->nomeFuncao) - 1] = '\0';
    nova->ret_type = DATA_VOID;
    nova->simbolos = NULL;
    nova->chamadas = NULL;
    nova->prox = NULL;
 
    if(listaTabelas == NULL) {
        listaTabelas = nova;
    }else{
        TabelaSimbolos *t = listaTabelas;
        while(t->prox != NULL) {
            t = t->prox;
        }
        t->prox = nova;
    }
 
    tabelaAtual = nova;
    return nova;
}
 
// Fechar tabela
void fecharEscopo(void){
    tabelaAtual = NULL;
}

TabelaSimbolos *buscarTabela(const char *nomeFuncao){
    TabelaSimbolos *t = listaTabelas;
    while(t != NULL) {
        if(strcmp(t->nomeFuncao, nomeFuncao) == 0) {
            return t;
        }
        t = t->prox;
    }
    return NULL;
}
 
// Buscar simbolo na tabela atual
Simbolo *buscarSimbolo(TabelaSimbolos *tabela, const char *name){
    if(tabela == NULL) {
        return NULL;
    }
 
    Simbolo *s = tabela->simbolos;
    while(s != NULL) {
        if(strcmp(s->name, name) == 0) {
            return s;
        }
        s = s->prox;
    }
    return NULL;
}
 
// Inserir simbolo na tabela atual
Simbolo *inserir(TabelaSimbolos *tabela, const char *name, DataTypes datatype, int is_param, int pos_param){
    if(buscarSimbolo(tabela, name) != NULL) {
        return NULL;
    }
 
    Simbolo *novo = malloc(sizeof(Simbolo));
    if(novo == NULL) {
        fprintf(stderr, "Erro: memória insuficiente para inserir na tabela de símbolos\n");
        exit(1);
    }


    strncpy(novo->name, name, sizeof(novo->name) - 1);
    novo->name[sizeof(novo->name) - 1] = '\0';
    novo->datatype = datatype;
    novo->is_param = is_param;
    novo->pos_param = pos_param;
    novo->call_refs = NULL;
    novo->num_refs = 0;
    novo->prox = NULL;
 
    if(tabela->simbolos == NULL) {
        tabela->simbolos = novo;
    }else{
        Simbolo *s = tabela->simbolos;
        while(s->prox != NULL) {
            s = s->prox;
        }
        s->prox = novo;
    }
    return novo;
}

// Simplificar a inserção de simbolos
Simbolo *declararSimbolo(Token *id, DataTypes tipo, int is_param, int pos_param){
    Simbolo *s = inserir(tabelaAtual, id->lexeme, tipo, is_param, pos_param);
    if(s == NULL) {
        erroSemantico(id, "variável já declarada");
    }
    return s;
}

// FunctionRegister

FunctionRegister *registrarChamada(TabelaSimbolos *tabela, Token *nomeChamada){
    FunctionRegister *reg = malloc(sizeof(FunctionRegister));
    if(reg == NULL) {
        fprintf(stderr, "Erro: memória insuficiente para registrar a chamada\n");
        exit(1);
    }
    strncpy(reg->name, nomeChamada->lexeme, sizeof(reg->name) - 1);
    reg->name[sizeof(reg->name) - 1] = '\0';
    reg->line = nomeChamada->line;
    reg->num_args = 0;
    reg->prox = NULL;

    // Adiciona no final
    if(tabela->chamadas == NULL) {
        tabela->chamadas = reg;
    }else{
        FunctionRegister *r = tabela->chamadas;
        while(r->prox != NULL) {
            r = r->prox;
        }
        r->prox = reg;
    }

    // Adiciona chamada na tabela de símbolos da função atual
    Simbolo *s = buscarSimbolo(tabela, nomeChamada->lexeme);
    if(s == NULL) {
        TabelaSimbolos *chamada = buscarTabela(nomeChamada->lexeme);
        if(chamada == NULL) {
            erroSemantico(nomeChamada, "função chamada antes de ser definida");
        }
        s = inserir(tabela, nomeChamada->lexeme, chamada->ret_type, 0, -1);
    }else if(s->num_refs == 0) {
        erroSemantico(nomeChamada, "variável não pode ser chamada como função");
    }

    // call_refs aponta para o registro
    FunctionRegister **refs = realloc(s->call_refs, (s->num_refs + 1) * sizeof(FunctionRegister *));
    if(refs == NULL) {
        fprintf(stderr, "Erro: memória insuficiente para registrar a chamada\n");
        exit(1);
    }
    s->call_refs = refs;
    s->call_refs[s->num_refs] = reg;
    s->num_refs++;

    return reg;
}

void adicionarArgumento(FunctionRegister *reg, Token *arg){
    strncpy(reg->args[reg->num_args], arg->lexeme, sizeof(reg->args[0]) - 1);
    reg->args[reg->num_args][sizeof(reg->args[0]) - 1] = '\0';
    reg->num_args++;
}

void inserirPrint(void){
    if(buscarSimbolo(tabelaAtual, "println") == NULL) {
        inserir(tabelaAtual, "println", DATA_VOID, 0, -1);
    }
}

// Armazenar tabelas no arquivo de saída

const char *nomeDataType(DataTypes d){
    switch(d){
        case DATA_INT: return "int";
        case DATA_FLOAT: return "float";
        case DATA_CHAR: return "char";
        case DATA_VOID: return "void";
        default: return "error";
    }
}

int numeroDaChamada(TabelaSimbolos *tabela, FunctionRegister *reg){
    int n = 1;
    FunctionRegister *r = tabela->chamadas;
    while(r != NULL && r != reg) {
        r = r->prox;
        n++;
    }
    return n;
}

void imprimirTabelas(FILE *arquivo){
    TabelaSimbolos *t = listaTabelas;
    while(t != NULL) {
        fprintf(arquivo, "==================================================================\n");
        fprintf(arquivo, "Tabela de símbolos da função: %s\n", t->nomeFuncao);
        fprintf(arquivo, "ret_type(%s) = %s\n", t->nomeFuncao, nomeDataType(t->ret_type));
        fprintf(arquivo, "------------------------------------------------------------------\n");
        fprintf(arquivo, "%-12s %-12s %-9s %-9s %-10s %s\n",
                "chave", "name", "datatype", "is_param", "pos_param", "call_refs");

        Simbolo *s = t->simbolos;
        if(s == NULL) {
            fprintf(arquivo, "(tabela vazia)\n");
        }
        while(s != NULL) {
            fprintf(arquivo, "%-12s %-12s %-9s %-9s %-10d ",
                    s->name, s->name, nomeDataType(s->datatype),
                    s->is_param ? "true" : "false", s->pos_param);
            if(s->num_refs == 0) {
                fprintf(arquivo, "NULL\n");
            }else{
                // cada referência aponta para um FunctionRegister da lista abaixo
                for(int i = 0; i < s->num_refs; i++) {
                    fprintf(arquivo, "%sFunctionRegister #%d", i > 0 ? ", " : "", numeroDaChamada(t, s->call_refs[i]));
                }
                fprintf(arquivo, "\n");
            }
            s = s->prox;
        }

        fprintf(arquivo, "\nFunctionRegister das chamadas feitas em %s:\n", t->nomeFuncao);
        if(t->chamadas == NULL) {
            fprintf(arquivo, "  (nenhuma chamada)\n");
        }
        int n = 1;
        FunctionRegister *r = t->chamadas;
        while(r != NULL) {
            fprintf(arquivo, "  #%d  name = %s | num_args = %d | args = [", n, r->name, r->num_args);
            for(int i = 0; i < r->num_args; i++) {
                fprintf(arquivo, "%s%s", i > 0 ? ", " : "", r->args[i]);
            }
            fprintf(arquivo, "]\n");
            r = r->prox;
            n++;
        }
        fprintf(arquivo, "\n");
        t = t->prox;
    }
}

// Free em todas as tabelas
void liberarTabelas(void){
    TabelaSimbolos *t = listaTabelas;
    while(t != NULL) {
        Simbolo *s = t->simbolos;
        while(s != NULL) {
            Simbolo *proxSimbolo = s->prox;
            free(s->call_refs);   // só o vetor de ponteiros: os registros são liberados abaixo
            free(s);
            s = proxSimbolo;
        }

        FunctionRegister *r = t->chamadas;
        while(r != NULL) {
            FunctionRegister *proxRegistro = r->prox;
            free(r);
            r = proxRegistro;
        }

        TabelaSimbolos *proxTabela = t->prox;
        free(t);
        t = proxTabela;
    }
    listaTabelas = NULL;
    tabelaAtual = NULL;
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

// Registro da chamada 
static FunctionRegister *registroAtual = NULL;

// Declaração das funções produções

void programa(void);
void funcaoseq(void);
void funcao(void);
void nomefuncao(void);
void listaparams(void);
void listaparams2(int pos);
DataTypes tiporetornofuncao(void);
void bloco(void);
void sequencia(void);
void declaracao(void);
Simbolo *varlist(void);
void varlist2(void);
DataTypes type(void);
void comando(void);
void atribuicaoouchamada(Token *id);
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
void chamadafuncao(Token *nome);
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
        nomefuncao();   // abre o escopo (tabela de símbolos) 
    }else{
        erroSintatico("ID ou MAIN");
    }
    casar(LBRACKET);
    if(tokenAtual()->type == ID) {
        listaparams();
    }
    casar(RBRACKET);
    if(tokenAtual()->type == ARROW) {
        tabelaAtual->ret_type = tiporetornofuncao();  
    }
    if(tokenAtual()->type == LBRACE) {
        bloco();
    }else{
        erroSintatico("LBRACE");
    }
    // Fim do escopo (fecha tabela de símbolos)
    fecharEscopo();
}

// ListaParams → ID : Type ListaParams2 | ε   
void listaparams(void){
    Token *id = casar(ID);
    casar(COLON);
    DataTypes tipo = DATA_ERROR;
    if(tokenAtual()->type == INT || tokenAtual()->type == FLOAT || tokenAtual()->type == CHAR) {
        tipo = type();
    }else{
        erroSintatico("INT, FLOAT ou CHAR");
    }
    declararSimbolo(id, tipo, 1, 0);   // primeiro pos_param = 0
    if(tokenAtual()->type == COMMA) {
        listaparams2(1);
    }
}

// ListaParams2 → , ID : Type ListaParams2 | ε
void listaparams2(int pos){
    casar(COMMA);
    Token *id = casar(ID);
    casar(COLON);
    DataTypes tipo = DATA_ERROR;
    if(tokenAtual()->type == INT || tokenAtual()->type == FLOAT || tokenAtual()->type == CHAR) {
        tipo = type();
    }else{
        erroSintatico("INT, FLOAT ou CHAR");
    }
    // Atualizar a posição do parametro
    declararSimbolo(id, tipo, 1, pos);
    if(tokenAtual()->type == COMMA) {
        listaparams2(pos + 1);
    }
}

// Type → int | float | char
// Devolve o tipo do token (Para a tabela de símbolos)
DataTypes type(void){
    Token *t = casar(tokenAtual()->type);
    return tipoDoToken(t->type);   
}

// NomeFuncao → ID | MAIN
// Abre o escopo da função (cria a tabela)
void nomefuncao(void){
    Token *nome = NULL;
    if(tokenAtual()->type == ID) {
        nome = casar(ID);
    }else if(tokenAtual()->type == MAIN) {
        nome = casar(MAIN);
    }else{
        erroSintatico("ID ou MAIN");
    }
    if(buscarTabela(nome->lexeme) != NULL) {
        erroSemantico(nome, "função definida mais de uma vez");
    }
    abrirEscopo(nome->lexeme);
}

// FuncaoSeq → Funcao FuncaoSeq | ε
void funcaoseq(void){
    if(tokenAtual()->type == FUNCTION) {
        funcao();
        funcaoseq();
    }
}

// TipoRetornoFuncao → -> Type | ε   
// Devolve o tipo de retorno da função
DataTypes tiporetornofuncao(void){
    casar(ARROW);
    if(tokenAtual()->type == INT || tokenAtual()->type == FLOAT || tokenAtual()->type == CHAR) {
        return type();
    }else{
        erroSintatico("INT, FLOAT ou CHAR");
    }
    return DATA_ERROR;
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
    Simbolo *primeira = NULL;
    casar(LET);
    if(tokenAtual()->type == ID) {
        primeira = varlist();
    }else{
        erroSintatico("ID");
    }
    casar(COLON);
    DataTypes tipo = DATA_ERROR;
    if(tokenAtual()->type == INT || tokenAtual()->type == FLOAT || tokenAtual()->type == CHAR) {
        tipo = type();
    }else{
        erroSintatico("INT, FLOAT ou CHAR");
    }

    // Prencher o tipo nas variaveis da lista
    Simbolo *s = primeira;
    while(s != NULL) {
        s->datatype = tipo;
        s = s->prox;
    }

    casar(SEMICOLON);
}

// VarList → ID VarList2
// Devolve o símbolo da primeira variável da lista.
Simbolo *varlist(void){
    Token *id = casar(ID);
    Simbolo *primeira = declararSimbolo(id, DATA_ERROR, 0, -1);
    if(tokenAtual()->type == COMMA) {
        varlist2();
    }
    return primeira;
}

// VarList2 → , ID VarList2 | ε
void varlist2(void){
    casar(COMMA);
    Token *id = casar(ID);
    declararSimbolo(id, DATA_ERROR, 0, -1);
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
        Token *id = casar(ID);
        if(tokenAtual()->type == ASSIGN || tokenAtual()->type == LBRACKET) {
            atribuicaoouchamada(id);
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
        inserirPrint();   // entra na tabela 
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
void atribuicaoouchamada(Token *id){
    if(tokenAtual()->type == ASSIGN) {
        casar(ASSIGN);
        if(firstExpr()) {
            expr();
        }else{
            erroSintatico(ESPERADO_EXPR);
        }
        casar(SEMICOLON);
    }else if(tokenAtual()->type == LBRACKET) {
        // Chamada de função
        registroAtual = registrarChamada(tabelaAtual, id);
        casar(LBRACKET);
        if(firstArg()) {
            listaargs();
        }
        casar(RBRACKET);
        registroAtual = NULL;   // fim da lista de argumentos desta chamada
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
        Token *id = casar(ID);
        if(tokenAtual()->type == LBRACKET) {
            chamadafuncao(id);
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
void chamadafuncao(Token *nome){
    FunctionRegister *anterior = registroAtual;
    registroAtual = registrarChamada(tabelaAtual, nome);
    casar(LBRACKET);
    if(firstArg()) {
        listaargs();
    }
    casar(RBRACKET);
    registroAtual = anterior;   // terminou a lista de argumentos desta chamada
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
    Token *t = tokenAtual();   // Lexema vai pro FunctionRegister
    if(tokenAtual()->type == ID) {
        casar(ID);
        if(tokenAtual()->type == LBRACKET) {
            chamadafuncao(t);
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
    // Para o caso do println
    if(registroAtual != NULL) {
        adicionarArgumento(registroAtual, t);
    }
}

/*
##########################################################
    ANALISE SEMÂNTICA
##########################################################
*/

void erroSemantico(Token *token, const char *mensagem){
    char texto[512];
    snprintf(texto, sizeof(texto), "Erro semântico na linha %d: %s: '%s'",
             token->line, mensagem, token->lexeme);

    FILE *arquivo = fopen("saida_erros_semanticos.txt", "w");
    if(arquivo) {
        fprintf(arquivo, "%s\n", texto);
        fclose(arquivo);
    }
    fprintf(stderr, "%s\nCompilação interrompida.\n", texto);
    exit(1);
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
 
    // Analise Lexica
    while((token = nextToken(file)).type != End_Of_File && index < MAX_TOKENS - 1) {
        vetorTokens[index++] = token;
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
    programa();


    if (arquivoErros) {
        fprintf(arquivoErros, "Nenhum erro sintático encontrado.\n");
        fclose(arquivoErros);
    }

    FILE *arquivoTabelas = fopen("saida_tabelas_simbolos.txt", "w");
    if (arquivoTabelas) {
        imprimirTabelas(arquivoTabelas);
        fclose(arquivoTabelas);
    }


    liberarTabelas();
 
    return 0;
}

// Como compilar gcc -Wall -Wextra -std=c11 Compilador.c -o compilador