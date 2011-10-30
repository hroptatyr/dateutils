/*** dexpr-parser.y -- parsing date expressions -*- c -*- */

%start stmt

%token TOK_EQ
%token TOK_NE
%token TOK_LT
%token TOK_LE
%token TOK_GT
%token TOK_GE

%token TOK_SPEC
%token TOK_STRING
%token TOK_DATE
%token TOK_TIME

%token TOK_OR
%token TOK_AND
%token TOK_NOT

%left TOK_OR
%left TOK_AND
%left TOK_NOT

%%


stmt
	: exp	{ ; }
	| stmt TOK_OR stmt
	| stmt TOK_AND stmt
	| TOK_NOT stmt
	| '(' stmt ')'
	;

exp
	: rhs
	| spec TOK_LT rhs
	| spec TOK_GT rhs
	| spec TOK_LE rhs
	| spec TOK_GE rhs
	| spec TOK_EQ rhs
	| spec TOK_NE rhs
	;

spec
	:
	| TOK_SPEC
	;

rhs
	: TOK_DATE
	| TOK_TIME
	| TOK_STRING
	;

%%

/* dexpr-parser.y ends here */
