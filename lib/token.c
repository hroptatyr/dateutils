/*** token.c -- tokeniser specs and stuff
 *
 * Copyright (C) 2011-2012 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
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
 **/
/* implementation part of token.h */
#if !defined INCLUDED_token_c_
#define INCLUDED_token_c_

#include "token.h"

static struct dt_spec_s
__tok_spec(const char *fp, char **ep)
{
	struct dt_spec_s res = spec_initialiser();

	if (*fp != '%') {
		goto out;
	}

next:
	switch (*++fp) {
	default:
		goto out;
	case 'F':
		res.spfl = DT_SPFL_N_DSTD;
		break;
	case 'T':
		res.spfl = DT_SPFL_N_TSTD;
		break;
	case 'y':
		res.abbr = DT_SPMOD_ABBR;
	case 'Y':
		res.spfl = DT_SPFL_N_YEAR;
		break;
	case 'm':
		res.spfl = DT_SPFL_N_MON;
		break;
	case 'd':
		res.spfl = DT_SPFL_N_DCNT_MON;
		break;
	case 'w':
		res.spfl = DT_SPFL_N_DCNT_WEEK;
		break;
	case 'D':
	case 'j':
		res.spfl = DT_SPFL_N_DCNT_YEAR;
		break;
	case 'c':
		res.spfl = DT_SPFL_N_WCNT_MON;
		break;
	case 'U':
		res.cnt_weeks_iso = 0;
		res.cnt_wdays_from = 0;
		res.spfl = DT_SPFL_N_WCNT_YEAR;
		break;
	case 'V':
		res.cnt_weeks_iso = 1;
		res.cnt_wdays_from = 1;
		res.spfl = DT_SPFL_N_WCNT_YEAR;
		break;
	case 'C':
	case 'W':
		res.cnt_weeks_iso = 0;
		res.cnt_wdays_from = 1;
		res.spfl = DT_SPFL_N_WCNT_YEAR;
		break;
	case 'A':
		res.abbr = DT_SPMOD_LONG;
	case 'a':
		res.spfl = DT_SPFL_S_WDAY;
		break;
	case 'B':
		res.abbr = DT_SPMOD_LONG;
	case 'b':
	case 'h':
		res.spfl = DT_SPFL_S_MON;
		break;

		/* time specs */
	case 'I':
		res.sc12 = 1;
	case 'H':
		res.spfl = DT_SPFL_N_HOUR;
		break;
	case 'M':
		res.spfl = DT_SPFL_N_MIN;
		break;
	case 'S':
		res.spfl = DT_SPFL_N_SEC;
		break;
	case 'N':
		res.spfl = DT_SPFL_N_NANO;
		break;
		/* am/pm indicator */
	case 'p':
		res.cap = 1;
	case 'P':
		res.spfl = DT_SPFL_S_AMPM;
		break;
	case 's':
		res.spfl = DT_SPFL_N_EPOCH;
		break;

	case '_':
		/* abbrev modifier */
		res.abbr = DT_SPMOD_ABBR;
		goto next;
	case '%':
		res.spfl = DT_SPFL_LIT_PERCENT;
		break;
	case 't':
		res.spfl = DT_SPFL_LIT_TAB;
		break;
	case 'n':
		res.spfl = DT_SPFL_LIT_NL;
		break;
	case 'Q':
		res.spfl = DT_SPFL_S_QTR;
		break;
	case 'q':
		res.spfl = DT_SPFL_N_QTR;
		break;
	case 'O':
		/* roman numerals modifier */
		res.rom = 1;
		goto next;
	}
	/* check for ordinals */
	if (res.spfl > DT_SPFL_UNK && res.spfl <= DT_SPFL_N_LAST &&
	    fp[1] == 't' && fp[2] == 'h' &&
	    !res.rom) {
		res.ord = 1;
		fp += 2;
	}
	/* check for bizda suffix */
	if (res.spfl == DT_SPFL_N_DCNT_MON || res.spfl == DT_SPFL_N_DCNT_YEAR) {
		switch (*++fp) {
		case 'B':
			res.ab = BIZDA_BEFORE;
		case 'b':
			res.bizda = 1;
			break;
		default:
			fp--;
			break;
		}
	}
out:
	if (ep) {
		*ep = (char*)(fp + 1);
	}
	return res;
}

#endif	/* INCLUDED_token_c_ */
