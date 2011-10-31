/*** dexpr-parser.y -- parsing date expressions -*- c -*-
 *
 * Copyright (C) 2002-2011 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@fresse.org>
 *
 * This file is part of dateutils.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the author nor the names of any contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***/

%defines
%output="y.tab.c"
%pure-parser
%parse-param{dexpr_t cur}

%{
#include <stdlib.h>
#include <stdio.h>
#include "dexpr.h"

extern int yylex();
extern int yyerror();

int
yyerror(dexpr_t __attribute__((unused)) cur, const char *errmsg)
{
	fputs(errmsg, stderr);
	fputc('\n', stderr);
	return -1;
}

%}

%expect 0

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
	: exp {
		$$ = calloc(1, sizeof(struct dexpr_s));
	}
	| stmt TOK_OR stmt {
		cur->type = DEX_DISJ;
		cur->left = $1;
		cur->right = $3;
	}
	| stmt TOK_AND stmt {
		cur->type = DEX_CONJ;
		cur->left = $1;
		cur->right = $3;
	}
	| TOK_NOT stmt {
		cur->type = DEX_NOT;
		cur->value = $2;
	}
	| '(' stmt ')' {
		$$ = $2;
	}
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
