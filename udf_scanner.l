%option c++ prefix="udf_scanner_" outfile="udf_scanner.cpp"
%option stack noyywrap yylineno 8bit debug
%{ 
// make relevant includes before including the parser's tab fil
#include <string>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include "udf_parser.tab.h"

// don't change this
#define yyerror LexerError

#define SAFE_STOI(base) {                         \
  try {                                           \
    yylval.i = std::stoi(yytext, nullptr, base);  \
    return tINTEGER;                              \
  } catch (const std::out_of_range&) {            \
    yyerror("Integer overflow");                  \
  }                                               \
}

#define SAFE_STOD {                      \
  try {                                  \
    yylval.d = std::stod(yytext);        \
    return tREAL;                        \
  } catch (const std::out_of_range&) {   \
    yyerror("Floating point overflow");  \
  }                                      \
}

%}

%x X_COMMENT X_STRING X_STRING_IGN X_BACKSLASH

%%
  yydebug=1; set_debug(1);

  /* ====================================================================== */
  /* ====[                    3.1 - WHITESPACE                        ]==== */
  /* ====================================================================== */

[ \t\n\r]+ ;

  /* ====================================================================== */
  /* ====[                    3.2 - COMMENTS                          ]==== */
  /* ====================================================================== */

"//".*\n                          ;

"/*"                              yy_push_state(X_COMMENT);
<X_COMMENT>"/*"                   yy_push_state(X_COMMENT);
<X_COMMENT>"*/"                   yy_pop_state();  
<X_COMMENT>.|\n                   ;                     

  /* ====================================================================== */
  /* ====[                    3.3 - KEYWORDS                          ]==== */
  /* ====================================================================== */

"forward"   return tFORWARD;
"public"    return tPUBLIC;

"if"        return tIF;
"elif"      return tELIF;
"else"      return tELSE;
"for"       return tFOR;
"break"     return tBREAK;
"continue"  return tCONTINUE;
"return"    return tRETURN;
"write"     return tWRITE;
"writeln"   return tWRITELN;

"input"     return tINPUT;
"nullptr"   return tNULLPTR;
"objects"   return tOBJECTS;
"sizeof"    return tSIZEOF;

"using"               return tUSING;
"iterate"             return tITERATE;


  /* ====================================================================== */
  /* ====[                    Tensores                             ]==== */
  /* ====================================================================== */

"capacity"  return tCAPACITY;
"rank"      return tRANK;
"dims"      return tDIMS;
"dim"       return tDIM;
"reshape"   return tRESHAPE;
"**"        return tCONTRACT;
"@"         return tINDEX;

  /* ====================================================================== */
  /* ====[                    3.4 - TYPES                             ]==== */
  /* ====================================================================== */

"int"     return tTYPE_INT;
"real"    return tTYPE_REAL;
"ptr"     return tTYPE_PTR;
"string"  return tTYPE_STRING;
"auto"    return tTYPE_AUTO;
"tensor"  return tTYPE_TENSOR;
"void"    return tTYPE_VOID;

  /* ====================================================================== */
  /* ====[    3.5 and 3.6 - OPERATORS/ DELIMITERS AND TERMINATORS     ]==== */
  /* ====================================================================== */

">="         return tGE;
"<="         return tLE;
"=="         return tEQ;
"!="         return tNE;
"&&"         return tAND;
"||"         return tOR;

[-()\{\}\[\]<>=+*/%;,@?~] return *yytext;

  /* ====================================================================== */
  /* ====[                    3.6 - DELIMITERS AND TERMINATORS        ]==== */
  /* ====================================================================== */

  /* ====================================================================== */
  /* ====[                    3.7 - IDENTIFIERS                       ]==== */
  /* ====================================================================== */

[A-Za-z][A-Za-z0-9_]*  yylval.s = new std::string(yytext); return tIDENTIFIER;

  /* ====================================================================== */
  /* ====[                    3.8.1 - INTEGERS                        ]==== */
  /* ====================================================================== */

0x[A-Fa-f0-9]+                SAFE_STOI(16);
[0-9]+                        SAFE_STOI(10); 

  /* ====================================================================== */
  /* ====[                    3.8.2 - FLOATING POINT NUMBERS          ]==== */
  /* ====================================================================== */

[0-9]+[Ee]([-+])?[0-9]+                               SAFE_STOD;
([0-9]*\.[0-9]+|[0-9]+\.[0-9])([Ee]([-+])?[0-9]+)?    SAFE_STOD;


  /* ====================================================================== */
  /* ====[                    3.8.3 - STRINGS                         ]==== */
  /* ====================================================================== */


"\""                        yylval.s = new std::string(""); yy_push_state(X_STRING);
<X_STRING>"\""              {
                             yy_pop_state();
                             return tSTRING;
                            }
<X_STRING>"\\"              yy_push_state(X_BACKSLASH);
<X_STRING>.                 *yylval.s += yytext;

<X_STRING_IGN>"\""          {
                            yy_pop_state(); yy_pop_state(); yy_pop_state();
                            yylval.s = new std::string("");
                            return tSTRING;
                            }
                         
<X_STRING_IGN>.             ;

<X_BACKSLASH>n               *yylval.s += "\n"; yy_pop_state();
<X_BACKSLASH>r               *yylval.s += "\r"; yy_pop_state(); 
<X_BACKSLASH>t               *yylval.s += "\t"; yy_pop_state(); 
<X_BACKSLASH>"\""            *yylval.s += "\""; yy_pop_state(); 
<X_BACKSLASH>"\\"            *yylval.s += "\\"; yy_pop_state(); 

<X_BACKSLASH>[A-Fa-f1-9]|[A-Fa-f0-9][A-Fa-f0-9] {
                                                int i = std::stoi(yytext, nullptr, 16);
                                                if (i > 255) yyerror("Special character overflow"); // limite: 8 bits
                                                *yylval.s += (char) i;
                                                yy_pop_state();
                                                }

<X_BACKSLASH>0               yy_push_state(X_STRING_IGN);

<X_BACKSLASH>.               *yylval.s += yytext; yy_pop_state(); 

  /* ====================================================================== */
  /* ====[                    (other characters, tensors)             ]==== */
  /* ====================================================================== */

. yyerror("Unknown character");

%%