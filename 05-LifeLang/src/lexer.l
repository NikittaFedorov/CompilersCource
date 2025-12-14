%{
#include <cstdlib>
#include <cstring>
#include "parser.tab.h"

static long long parse_int(const char* s) {
    return strtoll(s, NULL, 0);
}
%}

%option noyywrap
%option yylineno

%x COMMENT

%%

"//".*                 { /* skip line comment */ }
"/*"                   { BEGIN(COMMENT); }
<COMMENT>"*/"          { BEGIN(INITIAL); }
<COMMENT>.|\n          { /* eat */ }

[ \t\r\n]+             { /* skip whitespace */ }

"const"                { return KW_CONST; }
"func"                 { return KW_FUNC; }
"var"                  { return KW_VAR; }
"if"                   { return KW_IF; }
"else"                 { return KW_ELSE; }
"while"                { return KW_WHILE; }
"for"                  { return KW_FOR; }
"break"                { return KW_BREAK; }
"continue"             { return KW_CONTINUE; }
"return"               { return KW_RETURN; }
"i32"                  { return KW_I32; }
"void"                 { return KW_VOID; }

"=="                   { return OP_EQ; }
"!="                   { return OP_NE; }
"<="                   { return OP_LE; }
">="                   { return OP_GE; }
"&&"                   { return OP_AND; }
"||"                   { return OP_OR; }

0x[0-9A-Fa-f]+         { yylval.ival = parse_int(yytext); return INT; }
[0-9]+                 { yylval.ival = parse_int(yytext); return INT; }

[A-Za-z_][A-Za-z0-9_]* { yylval.str = strdup(yytext); return IDENT; }

.                      { return yytext[0]; }

%%

