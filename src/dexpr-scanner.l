/*** dexpr.l -- date/time expressions for dateutils -*- C -*-
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

%{
#include "dexpr-parser.h"
%}

%option noyywrap
%option warn
%option yylineno
%option nounput
%option noinput
%option bison-bridge
%option header-file="lex.yy.h"
%option outfile="lex.yy.c"
%option pointer
%option nodebug

%{
#define RETURN_TOKEN(_x)			\
	do {					\
		return _x;			\
	} while (0)

/* turn off tedious warnings */
#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:869)
# pragma warning (disable:1419)
# pragma warning (disable:2259)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wswitch-default"
# pragma GCC diagnostic ignored "-Wsign-compare"
# pragma GCC diagnostic ignored "-Wunused-parameter"
#endif	/* __GNUC__ || __INTEL_COMPILER */
%}

%%
"=="	{ RETURN_TOKEN(TOK_EQ); }
"!="	{ RETURN_TOKEN(TOK_NE); }
"<"	{ RETURN_TOKEN(TOK_LT); }
"<="	{ RETURN_TOKEN(TOK_LE); }
">"	{ RETURN_TOKEN(TOK_GT); }
">="	{ RETURN_TOKEN(TOK_GE); }

"%".	{ RETURN_TOKEN(TOK_SPEC); }

\"(\\.|[^\\\"])*\"	{ RETURN_TOKEN(TOK_STRING); }
[ \t\v\n\f]	{ /* ignore whitespace */ }

%%

/* dexpr.l ends here */
