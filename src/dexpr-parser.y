/*** dexpr-parser.y -- parsing date expressions -*- c -*-
 *
 * Copyright (C) 2002-2020 Sebastian Freundt
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

%pure-parser
%parse-param{dexpr_t *cur}

%{
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "token.h"
#include "date-core.h"
#include "date-core-strpf.h"
#include "dt-locale.h"
#include "dexpr.h"
#include "dexpr-parser.h"
#include "dt-io.h"

extern int yylex(YYSTYPE *yylval_param);
extern int yyerror(dexpr_t *cur __attribute__((unused)), const char *errmsg);
extern char *yytext;

/* grrrrr, bug in bison */
extern int yyparse();

#define YYENABLE_NLS		0
#define YYLTYPE_IS_TRIVIAL	1
#define YYSTACK_USE_ALLOCA	1

int
yyerror(dexpr_t *cur __attribute__((unused)), const char *errmsg)
{
	fputs("dexpr-parser: ", stderr);
	fputs(errmsg, stderr);
	if (errno) {
		fputc(':', stderr);
		fputc(' ', stderr);
		fputs(strerror(errno), stderr);
	}
	fputc('\n', stderr);
	return -1;
}

/* static stuff */
static struct dexkv_s ckv[1];

static char *const *ckv_fmt = NULL;
static size_t ckv_nfmt = 0;
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
%token TOK_DATETIME
%token TOK_INT

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
	: {
		memset(ckv, 0, sizeof(*ckv));
	} exp {
		$<dex>$ = calloc(1, sizeof(struct dexpr_s));
		$<dex>$->type = DEX_VAL;
		$<dex>$->kv[0] = *ckv;
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
	| spec TOK_LT { ckv->op = OP_LT; } rhs
	| spec TOK_GT { ckv->op = OP_GT; } rhs
	| spec TOK_LE { ckv->op = OP_LE; } rhs
	| spec TOK_GE { ckv->op = OP_GE; } rhs
	| spec TOK_EQ { ckv->op = OP_EQ; } rhs
	| spec TOK_NE { ckv->op = OP_NE; } rhs
	;

spec
	:
	| TOK_SPEC {
		struct dt_spec_s sp = __tok_spec($<sval>1, NULL);
		ckv->sp = sp;
	}
	;

rhs
	: TOK_DATETIME {
		ckv->d = dt_io_strpdt($<sval>1, ckv_fmt, ckv_nfmt, NULL);
		if (ckv->d.typ == DT_UNK) {
			/* one more try */
			ckv->d = dt_strpdt($<sval>1, NULL, NULL);
		}
		ckv->sp.spfl = DT_SPFL_N_STD;
	}
	| TOK_STRING {
		switch (ckv->sp.spfl) {
		case DT_SPFL_N_YEAR:
		case DT_SPFL_N_MON:
		case DT_SPFL_N_DCNT_WEEK:
		case DT_SPFL_N_DCNT_MON:
		case DT_SPFL_N_DCNT_YEAR:
		case DT_SPFL_N_WCNT_MON:
		case DT_SPFL_N_WCNT_YEAR:
			ckv->s = strtol($<sval>1, NULL, 10);
			break;
		case DT_SPFL_S_WDAY:
			switch (ckv->sp.abbr) {
			case DT_SPMOD_NORM:
				ckv->s = strtoarri(
					$<sval>1, NULL,
					dut_abbr_wday, dut_nabbr_wday);
				break;
			case DT_SPMOD_LONG:
				ckv->s = strtoarri(
					$<sval>1, NULL,
					dut_long_wday, dut_nlong_wday);
				break;
			case DT_SPMOD_ABBR: {
				const char *pos;
				if ((pos = strchr(
					     dut_abab_wday, *$<sval>1)) != NULL) {
					ckv->s = pos - dut_abab_wday;
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
					dut_abbr_mon, dut_nabbr_mon);
				break;
			case DT_SPMOD_LONG:
				ckv->s = strtoarri(
					$<sval>1, NULL,
					dut_long_mon, dut_nlong_mon);
				break;
			case DT_SPMOD_ABBR: {
				const char *pos;
				if ((pos = strchr(
					     dut_abab_mon, *$<sval>1)) != NULL) {
					ckv->s = pos - dut_abab_mon;
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
	| TOK_INT {
		switch (ckv->sp.spfl) {
		case DT_SPFL_N_YEAR:
		case DT_SPFL_N_MON:
		case DT_SPFL_N_DCNT_WEEK:
		case DT_SPFL_N_DCNT_MON:
		case DT_SPFL_N_DCNT_YEAR:
		case DT_SPFL_N_WCNT_MON:
		case DT_SPFL_N_WCNT_YEAR:
			ckv->s = strtol($<sval>1, NULL, 10);
			break;
		default:
			/* the rest can hardly have ints as inputs */
			YYERROR;
		}
	}
	;

%%

/* dexpr-parser.y ends here */
