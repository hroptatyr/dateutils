/*** token.c -- tokeniser specs and stuff
 *
 * Copyright (C) 2011 Sebastian Freundt
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

struct dt_spec_s {
	struct {
		/* ordinal flag, 01, 02, 03 -> 1st 2nd 3rd */
		unsigned int ord:1;
		/* roman numeral flag */
		unsigned int rom:1;
		/* controls abbreviation */
		enum {
			DT_SPMOD_NORM,
			DT_SPMOD_ABBR,
			DT_SPMOD_LONG,
			DT_SPMOD_ILL,
		} abbr:2;
		/* for directions a(fter 0)/b(efore 1) */
		unsigned int ab:1;
		/* bizda */
		unsigned int bizda:1;

		/** time specs */
		/* long/short 24h v 12h scale */
		unsigned int sc12:1;
		/* capitalise am/pm indicator */
		unsigned int cap:1;

		/* pad to the next byte */
		unsigned int:0;
	};
	dt_spfl_t spfl:8;
};

#if !defined BIZDA_AFTER
# define BIZDA_AFTER	(0U)/*>*/
#endif	/* !BIZDA_AFTER */
#if !defined BIZDA_BEFORE
# define BIZDA_BEFORE	(1U)/*<*/
#endif	/* !BIZDA_BEFORE */

static struct dt_spec_s
__tok_spec(const char *fp, char **ep)
{
	struct dt_spec_s res = {0};

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
		res.spfl = DT_SPFL_N_MDAY;
		break;
	case 'w':
		res.spfl = DT_SPFL_N_CNT_WEEK;
		break;
	case 'c':
		res.spfl = DT_SPFL_N_CNT_MON;
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
	case 'C':
	case 'j':
		res.spfl = DT_SPFL_N_CNT_YEAR;
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
	if (res.spfl == DT_SPFL_N_MDAY || res.spfl == DT_SPFL_N_CNT_YEAR) {
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
