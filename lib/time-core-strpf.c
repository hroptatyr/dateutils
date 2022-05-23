/*** time-core-strpf.c -- parser and formatter funs for time-core
 *
 * Copyright (C) 2011-2022 Sebastian Freundt
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
/* implementation part of time-core-strpf.h */
#if !defined INCLUDED_time_core_strpf_c_
#define INCLUDED_time_core_strpf_c_

#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include "strops.h"
#include "token.h"
#include "time-core.h"
#include "time-core-strpf.h"

#if !defined DEFUN
# define DEFUN
#endif	/* !DEFUN */

#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

DEFUN int
__strpt_card(struct strpt_s *d, const char *str, struct dt_spec_s s, char **ep)
{
	const char *sp = str;

	switch (s.spfl) {
	default:
	case DT_SPFL_UNK:
		goto fucked;
	case DT_SPFL_N_TSTD:
		if ((d->h = strtoi_lim(sp, &sp, 0, 23)) < 0 ||
		    *sp++ != ':') {
			goto fucked;
		} else if ((d->m = strtoi_lim(sp, &sp, 0, 59)) < 0 ||
			   *sp++ != ':') {
			goto fucked;
		} else if ((d->s = strtoi_lim(sp, &sp, 0, 60)) < 0) {
			goto fucked;
		}
		break;
	case DT_SPFL_N_HOUR:
		if (!s.sc12) {
			d->h = strtoi_lim(sp, &sp, 0, 23);
		} else {
			d->h = strtoi_lim(sp, &sp, 1, 12);
		}
		if (d->h < 0) {
			goto fucked;
		}
		break;
	case DT_SPFL_N_MIN:
		if ((d->m = strtoi_lim(sp, &sp, 0, 59)) < 0) {
			goto fucked;
		}
		break;
	case DT_SPFL_N_SEC:
		if ((d->s = strtoi_lim(sp, &sp, 0, 60)) < 0) {
			goto fucked;
		}
		break;
	case DT_SPFL_N_NANO: {
		/* nanoseconds */
		const char *on;

		if ((d->ns = strtoi_lim(sp, &on, 0, 999999999)) < 0) {
			goto fucked;
		}
		switch (on - sp) {
		case 0:
			goto fucked;
		case 1:
			d->ns *= 10;
		case 2:
			d->ns *= 10;
		case 3:
			d->ns *= 10;
		case 4:
			d->ns *= 10;
		case 5:
			d->ns *= 10;
		case 6:
			d->ns *= 10;
		case 7:
			d->ns *= 10;
		case 8:
			d->ns *= 10;
		default:
		case 9:
			break;
		}
		sp = on;
		break;
	}
	case DT_SPFL_S_AMPM: {
		const unsigned int casebit = 0x20;

		d->flags.am_pm_bit = 1;
		if ((sp[0] | casebit) == 'a' &&
		    (sp[1] | casebit) == 'm') {
			d->flags.pm_p = 0;
		} else if ((sp[0] | casebit) == 'p' &&
			   (sp[1] | casebit) == 'm') {
			d->flags.pm_p = 1;
		} else {
			goto fucked;
		}
		sp += 2;
		break;
	}
	case DT_SPFL_LIT_PERCENT:
		if (*sp++ != '%') {
			goto fucked;
		}
		break;
	case DT_SPFL_LIT_TAB:
		if (*sp++ != '\t') {
			goto fucked;
		}
		break;
	case DT_SPFL_LIT_NL:
		if (*sp++ != '\n') {
			goto fucked;
		}
		break;
	}

	/* check if components got set */
	switch (s.spfl) {
	case DT_SPFL_N_TSTD:
		d->flags.h_set = 1;
		d->flags.m_set = 1;
		d->flags.s_set = 1;
		break;
	case DT_SPFL_N_HOUR:
		d->flags.h_set = 1;
		break;
	case DT_SPFL_N_MIN:
		d->flags.m_set = 1;
		break;
	case DT_SPFL_N_SEC:
		d->flags.s_set = 1;
		break;
	case DT_SPFL_N_NANO:
		d->flags.ns_set = 1;
		break;
	default:
		break;
	}

	/* assign end pointer */
	if (ep != NULL) {
		*ep = (char*)sp;
	}
	return 0;
fucked:
	if (ep != NULL) {
		*ep = (char*)str;
	}
	return -1;
}

DEFUN size_t
__strft_card(
	char *buf, size_t bsz, struct dt_spec_s s,
	struct strpt_s *d, struct dt_t_s UNUSED(that))
{
	size_t res = 0;

	switch (s.spfl) {
	default:
	case DT_SPFL_UNK:
		break;
	case DT_SPFL_N_TSTD:
		if (LIKELY(bsz >= 8)) {
			ui99topstr(buf + 0, bsz, d->h, 2, '0');
			buf[2] = ':';
			ui99topstr(buf + 3, bsz, d->m, 2, '0');
			buf[5] = ':';
			ui99topstr(buf + 6, bsz, d->s, 2, '0');
			res = 8;
		}
		break;
	case DT_SPFL_N_HOUR:
		if (!s.sc12 || (d->h >= 1 && d->h <= 12)) {
			res = ui99topstr(
				buf, bsz, d->h,
				2 - (s.pad == DT_SPPAD_OMIT), padchar(s));
		} else {
			unsigned int h = d->h ? d->h - 12 : 12;
			res = ui99topstr(
				buf, bsz, h,
				2 - (s.pad == DT_SPPAD_OMIT), padchar(s));
		}
		break;
	case DT_SPFL_N_MIN:
		res = ui99topstr(
			buf, bsz, d->m,
			2 - (s.pad == DT_SPPAD_OMIT), padchar(s));
		break;
	case DT_SPFL_N_SEC:
		res = ui99topstr(
			buf, bsz, d->s,
			2 - (s.pad == DT_SPPAD_OMIT), padchar(s));
		break;
	case DT_SPFL_S_AMPM: {
		unsigned int casebit = 0;

		if (UNLIKELY(!s.cap)) {
			casebit = 0x20;
		}
		if (d->h >= 12 && d->h < 24) {
			buf[res++] = (char)('P' | casebit);
		} else {
			buf[res++] = (char)('A' | casebit);
		}
		buf[res++] = (char)('M' | casebit);
		break;
	}
	case DT_SPFL_N_NANO:
		res = ui999999999tostr(buf, bsz, d->ns);
		break;

	case DT_SPFL_LIT_PERCENT:
		/* literal % */
		buf[res++] = '%';
		break;
	case DT_SPFL_LIT_TAB:
		/* literal tab */
		buf[res++] = '\t';
		break;
	case DT_SPFL_LIT_NL:
		/* literal \n */
		buf[res++] = '\n';
		break;
	}
	return res;
}

#endif	/* INCLUDED_time_core_strpf_c_ */
