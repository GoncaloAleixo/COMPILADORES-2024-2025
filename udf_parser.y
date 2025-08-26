%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <algorithm>
#include <memory>
#include <cstring>
#include <cdk/compiler.h>
#include <cdk/types/types.h>
#include ".auto/all_nodes.h"
#define LINE                         compiler->scanner()->lineno()
#define yylex()                      compiler->scanner()->scan()
#define yyerror(compiler, s)         compiler->scanner()->error(s)
//-- don't change *any* of these --- END!
%}

%parse-param {std::shared_ptr<cdk::compiler> compiler}

%union {
  //--- don't change *any* of these: if you do, you'll break the compiler
  YYSTYPE() : type(cdk::primitive_type::create(0, cdk::TYPE_VOID)) {}
  ~YYSTYPE() {}
  YYSTYPE(const YYSTYPE &other) { *this = other; }
  YYSTYPE& operator=(const YYSTYPE &other) { type = other.type; return *this; }

  std::shared_ptr<cdk::basic_type> type;        /* expr type */
  //-- don't change *any* of these --- END!

  int                   i;          /* integer value */
  double                d;          /* real value */
  std::string          *s;          /* symbol name or string literal */

  cdk::basic_node      *node;       /* node pointer */
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expr nodes */
  cdk::lvalue_node     *lvalue;
  udf::block_node      *block;
  std::vector<std::string>     *v;
  std::vector<size_t>       *dims;
};

%token <i> tINTEGER
%token <d> tREAL
%token <s> tIDENTIFIER tSTRING

// Tipos
%token tTYPE_INT tTYPE_REAL tTYPE_PTR tTYPE_STRING tTYPE_TENSOR tTYPE_VOID tTYPE_AUTO

// Declarações
%token tFORWARD tPUBLIC tPRIVATE

// Instruções
%token tIF tELIF tELSE tFOR tBREAK tCONTINUE tRETURN tUNLESS tITERATE tUSING
%token tWRITE tWRITELN tINPUT 

// Tensores
%token tCAPACITY tRANK tDIMS tDIM tRESHAPE tCONTRACT tINDEX

// Expressões
%token tNULLPTR tSIZEOF tOBJECTS

// Operadores de igualdade e comparação
%token tGE tLE tEQ tNE
// Lógicos
%token tOR tAND

%nonassoc tIF 
%nonassoc tELSE tELIF

%nonassoc tINDEX '.'
%nonassoc tCONTRACT
%left tOR 
%left tAND
%right '='
%nonassoc '~'
%left tEQ tNE
%left tGE tLE '>' '<'
%left '+' '-' 
%left '*' '/' '%'
%nonassoc tUNARY
%nonassoc '?'

// Tipagens de não-terminais
%type <node> declaration function variable blockvar instruction iteration condition elifelse tensor_cont
%type <sequence> declarations exprs file variables blockdecls instructions arguments tensor_conts 
%type <expression> expr integer real tensor
%type <block> block
%type <type> type auto 
%type <lvalue> lval
%type <s> string
%type <dims> dim_list

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

file           :                                  { compiler->ast($$ = new cdk::sequence_node(LINE)); }
               | declarations                     { compiler->ast($$ = $1); }
               ;


declarations   : declaration                      { $$ = new cdk::sequence_node(LINE,$1); }
               | declarations declaration         { $$ = new cdk::sequence_node(LINE,$2,$1); }
               ;

declaration    : variable ';'                     { $$ = $1; }
               | function                         { $$ = $1; }
               ;

variable       : tPUBLIC  type tIDENTIFIER   { std::vector<std::string> v; v.push_back(*$3) ;$$ = new udf::variable_declaration_node(LINE,tPUBLIC,std::shared_ptr<cdk::basic_type>($2),v,nullptr); }
               | tFORWARD type tIDENTIFIER   { std::vector<std::string> v; v.push_back(*$3) ;$$ = new udf::variable_declaration_node(LINE,tFORWARD,std::shared_ptr<cdk::basic_type>($2),v,nullptr); }
               |          type tIDENTIFIER   { std::vector<std::string> v; v.push_back(*$2) ;$$ = new udf::variable_declaration_node(LINE,tPRIVATE,std::shared_ptr<cdk::basic_type>($1),v,nullptr); }
               | tPUBLIC  type tIDENTIFIER '=' expr         { std::vector<std::string> v; v.push_back(*$3) ;$$ = new udf::variable_declaration_node(LINE,tPUBLIC,std::shared_ptr<cdk::basic_type>($2),v,$5); }
               |          type tIDENTIFIER '=' expr         { std::vector<std::string> v; v.push_back(*$2) ;$$ = new udf::variable_declaration_node(LINE,tPRIVATE,std::shared_ptr<cdk::basic_type>($1),v,$4); }
               | tPUBLIC auto tIDENTIFIER '=' expr           { std::vector<std::string> v{*$3}; $$ = new udf::variable_declaration_node(LINE, tPUBLIC, nullptr, v, $5); delete $3;}
               | auto tIDENTIFIER '=' expr                   { std::vector<std::string> v{* $2 }; $$ = new udf::variable_declaration_node(LINE, tPRIVATE, nullptr, v, $4 ); delete $2; }
               ;

variables      : variable                         { $$ = new cdk::sequence_node(LINE,$1); }
               | variables ',' variable           { $$ = new cdk::sequence_node(LINE,$3,$1); }
               ;

function       :         type tIDENTIFIER '(' ')'                   { $$ = new udf::function_declaration_node(LINE,tPRIVATE,std::shared_ptr<cdk::basic_type>($1),*$2,nullptr); delete $2; }
               |         type tIDENTIFIER '(' arguments ')'             { $$ = new udf::function_declaration_node(LINE,tPRIVATE,std::shared_ptr<cdk::basic_type>($1),*$2,$4); delete $2; }
               |         type tIDENTIFIER '('  ')' block            { $$ = new udf::function_definition_node(LINE,tPRIVATE,std::shared_ptr<cdk::basic_type>($1),*$2,nullptr,$5); delete $2; }
               |         type tIDENTIFIER '(' arguments ')' block       { $$ = new udf::function_definition_node(LINE,tPRIVATE,std::shared_ptr<cdk::basic_type>($1),*$2,$4,$6); delete $2; }
               |         auto tIDENTIFIER '(' ')'                   { $$ = new udf::function_declaration_node(LINE,tPRIVATE,std::shared_ptr<cdk::basic_type>($1),*$2,nullptr); delete $2; }
               |         auto tIDENTIFIER '(' arguments ')'             { $$ = new udf::function_declaration_node(LINE,tPRIVATE,std::shared_ptr<cdk::basic_type>($1),*$2,$4); delete $2; }
               |         auto tIDENTIFIER '('  ')' block            { $$ = new udf::function_definition_node(LINE,tPRIVATE,std::shared_ptr<cdk::basic_type>($1),*$2,nullptr,$5); delete $2; }
               |         auto tIDENTIFIER '(' arguments ')' block       { $$ = new udf::function_definition_node(LINE,tPRIVATE,std::shared_ptr<cdk::basic_type>($1),*$2,$4,$6); delete $2; }
               | tPUBLIC type tIDENTIFIER '(' ')'                   { $$ = new udf::function_declaration_node(LINE,tPUBLIC,std::shared_ptr<cdk::basic_type>($2),*$3,nullptr); delete $3; }
               | tPUBLIC type tIDENTIFIER '(' arguments ')'             { $$ = new udf::function_declaration_node(LINE,tPUBLIC,std::shared_ptr<cdk::basic_type>($2),*$3,$5); delete $3; }
               | tPUBLIC type tIDENTIFIER '('  ')' block            { $$ = new udf::function_definition_node(LINE,tPUBLIC,std::shared_ptr<cdk::basic_type>($2),*$3,nullptr,$6); delete $3; }
               | tPUBLIC type tIDENTIFIER '(' arguments ')' block       { $$ = new udf::function_definition_node(LINE,tPUBLIC,std::shared_ptr<cdk::basic_type>($2),*$3,$5,$7); delete $3; }
               | tPUBLIC auto tIDENTIFIER '(' ')'                   { $$ = new udf::function_declaration_node(LINE,tPUBLIC,std::shared_ptr<cdk::basic_type>($2),*$3,nullptr); delete $3; }
               | tPUBLIC auto tIDENTIFIER '(' arguments ')'             { $$ = new udf::function_declaration_node(LINE,tPUBLIC,std::shared_ptr<cdk::basic_type>($2),*$3,$5); delete $3; }
               | tPUBLIC auto tIDENTIFIER '('  ')' block            { $$ = new udf::function_definition_node(LINE,tPUBLIC,std::shared_ptr<cdk::basic_type>($2),*$3,nullptr,$6); delete $3; }
               | tPUBLIC auto tIDENTIFIER '(' arguments ')' block       { $$ = new udf::function_definition_node(LINE,tPUBLIC,std::shared_ptr<cdk::basic_type>($2),*$3,$5,$7); delete $3; }
               | tFORWARD type tIDENTIFIER '(' ')'                  { $$ = new udf::function_declaration_node(LINE,tFORWARD,std::shared_ptr<cdk::basic_type>($2),*$3,nullptr); delete $3; }
               | tFORWARD type tIDENTIFIER '(' arguments ')'            { $$ = new udf::function_declaration_node(LINE,tFORWARD,std::shared_ptr<cdk::basic_type>($2),*$3,$5); delete $3; }
               | tFORWARD auto tIDENTIFIER '(' ')'                  { $$ = new udf::function_declaration_node(LINE,tFORWARD,std::shared_ptr<cdk::basic_type>($2),*$3,nullptr); delete $3; }
               | tFORWARD auto tIDENTIFIER '(' arguments ')'            { $$ = new udf::function_declaration_node(LINE,tFORWARD,std::shared_ptr<cdk::basic_type>($2),*$3,$5); delete $3; }
               ;

arguments      : type tIDENTIFIER                   { std::vector<std::string> v; v.push_back(*$2); $$ = new cdk::sequence_node(LINE,new udf::variable_declaration_node(LINE,-1,std::shared_ptr<cdk::basic_type>($1),v,nullptr)); }
               | arguments ',' type tIDENTIFIER     { std::vector<std::string> v; v.push_back(*$4); $$ = new cdk::sequence_node(LINE,new udf::variable_declaration_node(LINE,-1,std::shared_ptr<cdk::basic_type>($3),v,nullptr),$1); }
               ; 

exprs          : expr                               {  $$ = new cdk::sequence_node(LINE, $1); }
               | exprs ',' expr                     {  $$ = new cdk::sequence_node(LINE, $3, $1); }
               ;

expr           : integer                                    {  $$ = $1; }
               | string                                     {  $$ = new cdk::string_node(LINE, $1); }
               | real                                       {  $$ = $1; }
               | tINPUT					                    {  $$ = new udf::input_node(LINE); }
               | tNULLPTR				                    {  $$ = new udf::nullptr_node(LINE); }
	           | tIDENTIFIER '(' ')'			            {  $$ = new udf::function_call_node(LINE,*$1, nullptr); delete $1; }
      	       | tIDENTIFIER '(' exprs ')'		            {  $$ = new udf::function_call_node(LINE,*$1, $3); delete $1; }
               | '-' expr %prec tUNARY                      {  $$ = new cdk::unary_minus_node(LINE, $2); }
               | '+' expr %prec tUNARY 		                {  $$ = new cdk::unary_plus_node(LINE,$2);}
               | '~' expr                                   {  $$ = new cdk::not_node(LINE,$2); }
               | expr '+' expr	                            {  $$ = new cdk::add_node(LINE, $1, $3); }
               | expr '-' expr	                            {  $$ = new cdk::sub_node(LINE, $1, $3); }
               | expr '*' expr	                            {  $$ = new cdk::mul_node(LINE, $1, $3); }
               | expr '/' expr	                            {  $$ = new cdk::div_node(LINE, $1, $3); }
               | expr '%' expr	                            {  $$ = new cdk::mod_node(LINE, $1, $3); }
               | expr '<' expr	                            {  $$ = new cdk::lt_node(LINE, $1, $3); }
               | expr '>' expr	                            {  $$ = new cdk::gt_node(LINE, $1, $3); }
               | expr tGE expr	                            {  $$ = new cdk::ge_node(LINE, $1, $3); }
               | expr tLE expr                              {  $$ = new cdk::le_node(LINE, $1, $3); }
               | expr tNE expr	                            {  $$ = new cdk::ne_node(LINE, $1, $3); }
               | expr tEQ expr	                            {  $$ = new cdk::eq_node(LINE, $1, $3); }
               | '(' expr ')'                               {  $$ = $2; }
               | lval                                       {  $$ = new cdk::rvalue_node(LINE, $1); }
               | lval '=' expr                              {  $$ = new cdk::assignment_node(LINE, $1, $3); }
               | expr tAND expr			                    { $$ = new cdk::and_node(LINE, $1,$3); }
               | expr tOR expr			                    { $$ = new cdk::or_node(LINE, $1,$3); }
               | tOBJECTS '(' expr ')'                      { $$ = new udf::objects_node(LINE, $3); } 
               | lval '?' 			                        { $$ = new udf::address_of_node(LINE,$1); }
               | tSIZEOF '(' expr ')'                       { $$ = new udf::sizeof_node(LINE, $3); }
               | tensor                                     { $$ = $1; }
               | expr '.' tCAPACITY                         { $$ = new udf::tensor_capacity_node(LINE, $1); } 
               | expr '.' tRANK                             { $$ = new udf::tensor_rank_node(LINE, $1); }
               | expr '.' tDIMS                             { $$ = new udf::tensor_dims_node(LINE, $1); }
               | expr '.' tDIM '(' expr ')'                 { $$ = new udf::tensor_dim_node(LINE, $1, $5); }
               | expr '.' tRESHAPE '(' exprs ')'            { $$ = new udf::tensor_reshape_node(LINE, $1, $5); }
               | expr '.' tCONTRACT expr                    { $$ = new udf::tensor_contract_node(LINE, $1, $4); }
               ;

type           : tTYPE_INT                        { $$ = cdk::primitive_type::create(4, cdk::TYPE_INT); }
               | tTYPE_REAL                       { $$ = cdk::primitive_type::create(8, cdk::TYPE_DOUBLE); }
               | tTYPE_STRING                     { $$ = cdk::primitive_type::create(4, cdk::TYPE_STRING); }
               | tTYPE_VOID                       { $$ = cdk::primitive_type::create(0, cdk::TYPE_VOID);}
               | tTYPE_PTR '<' type '>'             { $$ = cdk::reference_type::create(4, $3); }
               | tTYPE_PTR '<' auto '>'             { $$ = cdk::reference_type::create(4, $3); }
               | tTYPE_TENSOR '<' dim_list '>'  {$$ = cdk::tensor_type::create(*$3); delete $3;  } 
               ;

auto  :     tTYPE_AUTO { $$ = cdk::primitive_type::create(0, cdk::TYPE_UNSPEC); }

block : '{' '}'                                     { $$ = new udf::block_node(LINE, nullptr, nullptr); }
      | '{' blockdecls '}'                          { $$ = new udf::block_node(LINE, $2, nullptr); }
      | '{' instructions '}'                        { $$ = new udf::block_node(LINE, nullptr, $2); }
      | '{' blockdecls instructions '}'             { $$ = new udf::block_node(LINE, $2, $3); }
      ;

blockdecls : blockvar ';'                           { $$ = new cdk::sequence_node(LINE, $1); }
           | blockdecls blockvar ';'                { $$ = new cdk::sequence_node(LINE, $2, $1); }
           ;
           
blockvar : type tIDENTIFIER                   { std::vector<std::string> v; v.push_back(*$2); $$ = new udf::variable_declaration_node(LINE,tPRIVATE,std::shared_ptr<cdk::basic_type>($1),v,nullptr); }
         | type tIDENTIFIER '=' expr          { std::vector<std::string> v; v.push_back(*$2); $$ = new udf::variable_declaration_node(LINE,tPRIVATE,std::shared_ptr<cdk::basic_type>($1),v,$4); }
         | auto tIDENTIFIER '=' expr         { std::vector<std::string> v; v.push_back(*$2); $$ = new udf::variable_declaration_node(LINE, tPRIVATE, std::shared_ptr<cdk::basic_type>($1), v, $4); delete $2; }
         ;

instructions : instruction                     { $$ = new cdk::sequence_node(LINE, $1); }
            | instructions instruction        { $$ = new cdk::sequence_node(LINE, $2, $1); }
            ;

instruction : expr ';'                        { $$ = new udf::evaluation_node(LINE, $1); }
           | tWRITE exprs ';'                 { $$ = new udf::write_node(LINE, $2, false); }
           | tWRITELN exprs ';'               { $$ = new udf::write_node(LINE, $2, true); }
           | tITERATE expr tFOR expr tUSING expr tIF expr ';' { $$ = new udf::unless_node(LINE, $8, $2, $4, $6); }
           | tBREAK                           { $$ = new udf::break_node(LINE); }
           | tCONTINUE                        { $$ = new udf::continue_node(LINE); }
           | tRETURN ';'                      { $$ = new udf::return_node(LINE, nullptr); }
           | tRETURN expr ';'                 { $$ = new udf::return_node(LINE, $2); }
           | condition                        { $$ = $1; }
           | iteration                        { $$ = $1; }
           | block                            { $$ = $1; }
           ;

condition
    : tIF '(' expr ')' instruction %prec tIF          { $$ = new udf::if_node(LINE, $3, $5); }
    | tIF '(' expr ')' instruction elifelse           { $$ = new udf::if_else_node(LINE, $3, $5, $6); }
    ;

elifelse
    : tELSE instruction                               { $$ = $2; }
    | tELIF '(' expr ')' instruction %prec tIF        { $$ = new udf::if_node(LINE, $3, $5); }
    | tELIF '(' expr ')' instruction elifelse         { $$ = new udf::if_else_node(LINE, $3, $5, $6); }
    ;

dim_list : tINTEGER                       { $$ = new std::vector<size_t>(); $$->push_back($1); }
         | dim_list ',' tINTEGER          { $$ = $1; $$->push_back($3); }
             ;

iteration
    : tFOR '('         ';'         ';'         ')' instruction     { $$ = new udf::for_node(LINE, nullptr, nullptr, nullptr, $6); }
    | tFOR '(' variables ';'       ';'         ')' instruction     { $$ = new udf::for_node(LINE, $3, nullptr, nullptr, $7); }
    | tFOR '('         ';' exprs   ';'         ')' instruction     { $$ = new udf::for_node(LINE, nullptr, $4, nullptr, $7); }
    | tFOR '('         ';'         ';' exprs   ')' instruction     { $$ = new udf::for_node(LINE, nullptr, nullptr, $5, $7); }
    | tFOR '(' variables ';' exprs ';'         ')' instruction     { $$ = new udf::for_node(LINE, $3, $5, nullptr, $8); }
    | tFOR '(' variables ';'       ';' exprs   ')' instruction     { $$ = new udf::for_node(LINE, $3, nullptr, $6, $8); }
    | tFOR '(' variables ';' exprs ';' exprs   ')' instruction     { $$ = new udf::for_node(LINE, $3, $5, $7, $9); }
    | tFOR '(' exprs    ';'         ';'        ')' instruction     { $$ = new udf::for_node(LINE, $3, nullptr, nullptr, $7); }
    | tFOR '(' exprs    ';' exprs   ';'        ')' instruction     { $$ = new udf::for_node(LINE, $3, $5, nullptr, $8); }
    | tFOR '(' exprs    ';'         ';' exprs  ')' instruction     { $$ = new udf::for_node(LINE, $3, nullptr, $6, $8); }
    | tFOR '('          ';' exprs   ';' exprs  ')' instruction     { $$ = new udf::for_node(LINE, nullptr, $4, $6, $8); }
    | tFOR '(' exprs    ';' exprs   ';' exprs  ')' instruction     { $$ = new udf::for_node(LINE, $3, $5, $7, $9); }
    ;

lval    : tIDENTIFIER                               { $$ = new cdk::variable_node(LINE, *$1); delete $1; }
        | lval '[' expr ']' 	                    { $$ = new udf::indexptr_node(LINE, new cdk::rvalue_node(LINE,$1), $3); }
        | '(' expr ')' '[' expr ']'                 { $$ = new udf::indexptr_node(LINE, $2, $5); }
        | tIDENTIFIER '(' ')' '[' expr ']' 	        { $$ = new udf::indexptr_node(LINE, new udf::function_call_node(LINE,*$1, nullptr), $5); delete $1;}
        | tIDENTIFIER '(' exprs ')' '[' expr ']'    { $$ = new udf::indexptr_node(LINE, new udf::function_call_node(LINE,*$1, $3), $6); delete $1;}
        | expr tINDEX '(' exprs ')'                 { $$ = new udf::tensor_index_node(LINE, $1, $4); }

tensor     : '[' tensor_conts ']'                   { $$ = new udf::tensor_node(LINE, $2); }

tensor_conts : tensor_cont                          { $$ = new cdk::sequence_node(LINE, $1); } 
             | tensor_conts ',' tensor_cont         { $$ = new cdk::sequence_node(LINE,$3, $1); }

tensor_cont : expr                                 { $$ = $1; }
            ;

integer         : tINTEGER                      { $$ = new cdk::integer_node(LINE, $1); };
real            : tREAL                         { $$ = new cdk::double_node(LINE, $1); };
string          : tSTRING                       { $$ = $1; }
                | string tSTRING                { $$ = new std::string(*$1 + *$2); delete $1; delete $2; }
                ;
            
%%
