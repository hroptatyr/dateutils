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
%parse-param{dexpr_t *cur}
%parse-param{struct dexkv_s *ckv}

%{
#include <stdlib.h>
#include <stdio.h>
#include "dexpr.h"

extern int yylex();
extern int yyerror();
extern char *yytext;

/* grrrrr, bug in bison */
extern int yyparse();

#if !defined UNUSED
# define UNUSED(x)	__attribute__((unused)) x
#endif	/* !UNUSED */

#define YYENABLE_NLS		0
#define YYLTYPE_IS_TRIVIAL	1
#define YYSTACK_USE_ALLOCA	1

int
yyerror(dexpr_t *UNUSED(cur), struct dexkv_s *UNUSED(ckv), const char *errmsg)
{
	fputs(errmsg, stderr);
	fputc('\n', stderr);
	return -1;
}
%}

%union {
	dexpr_t dex;
	char *sval;
}

%expect 0

%start root

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

%token TOK_LPAREN
%token TOK_RPAREN

%left TOK_OR
%left TOK_AND
%left TOK_NOT

%%

root:
	stmt {
		*cur = $<dex>$;
		YYACCEPT;
	}

stmt
	: exp {
		$<dex>$ = calloc(1, sizeof(struct dexpr_s));
		$<dex>$->type = DEX_VAL;
		*($<dex>$->kv) = *ckv;
	}
	| stmt TOK_OR stmt {
		$<dex>$ = calloc(1, sizeof(struct dexpr_s));
		$<dex>$->type = DEX_DISJ;
		$<dex>$->left = $<dex>1;
		$<dex>$->right = $<dex>3;
	}
	| stmt TOK_AND stmt {
		$<dex>$ = calloc(1, sizeof(struct dexpr_s));
		$<dex>$->type = DEX_CONJ;
		$<dex>$->left = $<dex>1;
		$<dex>$->right = $<dex>3;
	}
	| TOK_NOT stmt {
		($<dex>$ = $<dex>2)->nega = 1;
	}
	| TOK_LPAREN stmt TOK_RPAREN {
		$<dex>$ = $<dex>2;
	}
	;

exp
	: rhs
	| spec TOK_LT { ckv->op = TOK_LT; } rhs
	| spec TOK_GT { ckv->op = TOK_GT; } rhs
	| spec TOK_LE { ckv->op = TOK_LE; } rhs
	| spec TOK_GE { ckv->op = TOK_GE; } rhs
	| spec TOK_EQ { ckv->op = TOK_EQ; } rhs
	| spec TOK_NE { ckv->op = TOK_NE; } rhs
	;

spec
	:
	| TOK_SPEC {
		struct dt_spec_s sp = __tok_spec($<sval>1, NULL);
		ckv->sp = sp;
	}
	;

rhs
	: TOK_DATE {
		struct dt_d_s d = dt_strpd($<sval>1, NULL, NULL);
		ckv->d = d;
		ckv->sp.spfl = DT_SPFL_N_STD;
	}
	| TOK_TIME {
		/* no support yet */
	}
	| TOK_STRING {
		switch (ckv->sp.spfl) {
		case DT_SPFL_N_MDAY:
		case DT_SPFL_N_MON:
		case DT_SPFL_N_YEAR:
		case DT_SPFL_N_CNT_WEEK:
		case DT_SPFL_N_CNT_MON:
		case DT_SPFL_N_CNT_YEAR:
			ckv->s = strtol($<sval>1, NULL, 10);
			break;
		case DT_SPFL_S_WDAY:
			switch (ckv->sp.abbr) {
			case DT_SPMOD_NORM:
				ckv->s = strtoarri(
					$<sval>1, NULL,
					__abbr_wday, countof(__abbr_wday));
				break;
			case DT_SPMOD_LONG:
				ckv->s = strtoarri(
					$<sval>1, NULL,
					__long_wday, countof(__long_wday));
				break;
			case DT_SPMOD_ABBR: {
				const char *pos;
				if ((pos = strchr(__abab_wday, *$<sval>1))) {
					ckv->s = pos - __abab_wday;
					break;
				}
			}
			case DT_SPMOD_ILL:
			default:
				ckv->s = -1;
				break;
			}
			break;
		case DT_SPFL_S_MON:
			switch (ckv->sp.abbr) {
			case DT_SPMOD_NORM:
				ckv->s = strtoarri(
					$<sval>1, NULL,
					__abbr_mon, countof(__abbr_mon));
				break;
			case DT_SPMOD_LONG:
				ckv->s = strtoarri(
					$<sval>1, NULL,
					__long_mon, countof(__long_mon));
				break;
			case DT_SPMOD_ABBR: {
				const char *pos;
				if ((pos = strchr(__abab_mon, *$<sval>1))) {
					ckv->s = pos - __abab_mon;
					break;
				}
			}
			case DT_SPMOD_ILL:
			default:
				ckv->s = -1;
				break;
			}
			break;
		default:
			break;
		}
	}
	;

%%

/* dexpr-parser.y ends here */
