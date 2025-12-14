%{
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <utility>

#include "ast.hpp"

// Bison/Flex interface
extern int yylex();
extern int yylineno;
void yyerror(const char* s);

// Parse result
lifelang::Program* gProgram = nullptr;

using namespace lifelang;

// Helpers to avoid strdup leaks in actions
static std::string takeStr(char* s) { std::string r(s); free(s); return r; }

%}


%code requires {
#include "ast.hpp"
#include <vector>
#include <string>
}

%define parse.error verbose

%union {
  long long ival;
  char* str;
  lifelang::Expr* expr;
  lifelang::Stmt* stmt;
  lifelang::BlockStmt* block;
  lifelang::Decl* decl;
  lifelang::RetType rtype;
  std::vector<lifelang::Decl*>* decls;
  std::vector<lifelang::Stmt*>* stmts;
  std::vector<lifelang::Expr*>* exprs;
}

%token <ival> INT
%token <str>  IDENT

%token KW_CONST KW_FUNC KW_VAR
%token KW_IF KW_ELSE
%token KW_WHILE KW_FOR
%token KW_BREAK KW_CONTINUE
%token KW_RETURN
%token KW_I32 KW_VOID

%token OP_EQ OP_NE OP_LE OP_GE OP_AND OP_OR

%type <decls> Program DeclList
%type <decl>  Decl ConstDecl FuncDecl
%type <rtype> ReturnOpt TypeSpec
%type <block> Block
%type <stmts> StmtList
%type <stmt>  Stmt VarDecl AssignStmt IfStmt WhileStmt ForStmt BreakStmt ContinueStmt ReturnStmt ExprStmt
%type <stmt>  ForInit ForStep
%type <expr>  Expr ExprOpt OrExpr AndExpr EqExpr RelExpr AddExpr MulExpr Unary Primary LValue
%type <exprs> ArgListOpt ArgList IndexList DimListOpt DimList

%left OP_OR
%left OP_AND
%left OP_EQ OP_NE
%left '<' '>' OP_LE OP_GE
%left '+' '-'
%left '*' '/' '%'
%right '!' UMINUS

%%

Program
  : DeclList {
      gProgram = new Program(std::move(*$1));
      delete $1;
    }
  ;

DeclList
  : /* empty */ { $$ = new std::vector<Decl*>(); }
  | DeclList Decl { $1->push_back($2); $$ = $1; }
  ;

Decl
  : ConstDecl { $$ = $1; }
  | FuncDecl  { $$ = $1; }
  ;

ConstDecl
  : KW_CONST IDENT '=' INT ';' {
      $$ = new ConstDecl(takeStr($2), $4);
    }
  ;

FuncDecl
  : KW_FUNC IDENT '(' ')' ReturnOpt Block {
      $$ = new FuncDecl(takeStr($2), $5, $6);
    }
  ;

ReturnOpt
  : /* empty */ { $$ = RetType::Void; }
  | ':' TypeSpec { $$ = $2; }
  ;

TypeSpec
  : KW_I32  { $$ = RetType::I32; }
  | KW_VOID { $$ = RetType::Void; }
  ;

Block
  : '{' StmtList '}' {
      $$ = new BlockStmt(std::move(*$2));
      delete $2;
    }
  ;

StmtList
  : /* empty */ { $$ = new std::vector<Stmt*>(); }
  | StmtList Stmt { $1->push_back($2); $$ = $1; }
  ;

Stmt
  : VarDecl        { $$ = $1; }
  | AssignStmt     { $$ = $1; }
  | IfStmt         { $$ = $1; }
  | WhileStmt      { $$ = $1; }
  | ForStmt        { $$ = $1; }
  | BreakStmt      { $$ = $1; }
  | ContinueStmt   { $$ = $1; }
  | ReturnStmt     { $$ = $1; }
  | ExprStmt       { $$ = $1; }
  | Block          { $$ = $1; }
  ;

VarDecl
  : KW_VAR IDENT DimListOpt ';' {
      $$ = new VarDeclStmt(takeStr($2), std::move(*$3), nullptr);
      delete $3;
    }
  | KW_VAR IDENT DimListOpt '=' Expr ';' {
      $$ = new VarDeclStmt(takeStr($2), std::move(*$3), $5);
      delete $3;
    }
  ;

AssignStmt
  : LValue '=' Expr ';' {
      $$ = new AssignStmt($1, $3);
    }
  ;

IfStmt
  : KW_IF '(' Expr ')' Block {
      $$ = new IfStmt($3, $5, nullptr);
    }
  | KW_IF '(' Expr ')' Block KW_ELSE Block {
      $$ = new IfStmt($3, $5, $7);
    }
  ;

WhileStmt
  : KW_WHILE '(' Expr ')' Block {
      $$ = new WhileStmt($3, $5);
    }
  ;

ForStmt
  : KW_FOR '(' ForInit ';' ExprOpt ';' ForStep ')' Block {
      $$ = new ForStmt($3, $5, $7, $9);
    }
  ;

ExprOpt
  : /* empty */ { $$ = nullptr; }
  | Expr { $$ = $1; }
  ;

ForInit
  : /* empty */ { $$ = nullptr; }
  | KW_VAR IDENT DimListOpt {
      $$ = new VarDeclStmt(takeStr($2), std::move(*$3), nullptr);
      delete $3;
    }
  | KW_VAR IDENT DimListOpt '=' Expr {
      $$ = new VarDeclStmt(takeStr($2), std::move(*$3), $5);
      delete $3;
    }
  | LValue '=' Expr {
      $$ = new AssignStmt($1, $3);
    }
  ;

ForStep
  : /* empty */ { $$ = nullptr; }
  | LValue '=' Expr {
      $$ = new AssignStmt($1, $3);
    }
  | Expr { $$ = new ExprStmt($1); }
  ;

BreakStmt
  : KW_BREAK ';' { $$ = new BreakStmt(); }
  ;

ContinueStmt
  : KW_CONTINUE ';' { $$ = new ContinueStmt(); }
  ;

ReturnStmt
  : KW_RETURN ';' { $$ = new ReturnStmt(nullptr); }
  | KW_RETURN Expr ';' { $$ = new ReturnStmt($2); }
  ;

ExprStmt
  : Expr ';' { $$ = new ExprStmt($1); }
  ;

DimListOpt
  : /* empty */ { $$ = new std::vector<Expr*>(); }
  | DimList { $$ = $1; }
  ;

DimList
  : DimList '[' Expr ']' { $1->push_back($3); $$ = $1; }
  | '[' Expr ']' { $$ = new std::vector<Expr*>(); $$->push_back($2); }
  ;

ArgListOpt
  : /* empty */ { $$ = new std::vector<Expr*>(); }
  | ArgList { $$ = $1; }
  ;

ArgList
  : ArgList ',' Expr { $1->push_back($3); $$ = $1; }
  | Expr { $$ = new std::vector<Expr*>(); $$->push_back($1); }
  ;

IndexList
  : IndexList '[' Expr ']' { $1->push_back($3); $$ = $1; }
  | '[' Expr ']' { $$ = new std::vector<Expr*>(); $$->push_back($2); }
  ;

Expr
  : OrExpr { $$ = $1; }
  ;

OrExpr
  : OrExpr OP_OR AndExpr { $$ = new BinaryExpr("||", $1, $3); }
  | AndExpr { $$ = $1; }
  ;

AndExpr
  : AndExpr OP_AND EqExpr { $$ = new BinaryExpr("&&", $1, $3); }
  | EqExpr { $$ = $1; }
  ;

EqExpr
  : EqExpr OP_EQ RelExpr { $$ = new BinaryExpr("==", $1, $3); }
  | EqExpr OP_NE RelExpr { $$ = new BinaryExpr("!=", $1, $3); }
  | RelExpr { $$ = $1; }
  ;

RelExpr
  : RelExpr '<' AddExpr { $$ = new BinaryExpr("<", $1, $3); }
  | RelExpr '>' AddExpr { $$ = new BinaryExpr(">", $1, $3); }
  | RelExpr OP_LE AddExpr { $$ = new BinaryExpr("<=", $1, $3); }
  | RelExpr OP_GE AddExpr { $$ = new BinaryExpr(">=", $1, $3); }
  | AddExpr { $$ = $1; }
  ;

AddExpr
  : AddExpr '+' MulExpr { $$ = new BinaryExpr("+", $1, $3); }
  | AddExpr '-' MulExpr { $$ = new BinaryExpr("-", $1, $3); }
  | MulExpr { $$ = $1; }
  ;

MulExpr
  : MulExpr '*' Unary { $$ = new BinaryExpr("*", $1, $3); }
  | MulExpr '/' Unary { $$ = new BinaryExpr("/", $1, $3); }
  | MulExpr '%' Unary { $$ = new BinaryExpr("%", $1, $3); }
  | Unary { $$ = $1; }
  ;

Unary
  : '!' Unary { $$ = new UnaryExpr("!", $2); }
  | '-' Unary %prec UMINUS { $$ = new UnaryExpr("-", $2); }
  | Primary { $$ = $1; }
  ;

LValue
  : IDENT { $$ = new NameExpr(takeStr($1)); }
  | IDENT IndexList {
      $$ = new IndexExpr(takeStr($1), std::move(*$2));
      delete $2;
    }
  ;

Primary
  : INT { $$ = new IntExpr($1); }
  | IDENT { $$ = new NameExpr(takeStr($1)); }
  | IDENT '(' ArgListOpt ')' {
      $$ = new CallExpr(takeStr($1), std::move(*$3));
      delete $3;
    }
  | IDENT IndexList {
      $$ = new IndexExpr(takeStr($1), std::move(*$2));
      delete $2;
    }
  | '(' Expr ')' { $$ = $2; }
  ;

%%

void yyerror(const char* s) {
  std::fprintf(stderr, "[parser] line %d: %s\n", yylineno, s);
}
