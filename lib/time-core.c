/*** time-core.h -- our universe of times
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
/* implementation part of date-core.h */
#if !defined INCLUDED_time_core_c_
#define INCLUDED_time_core_c_

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include "strops.h"

#if !defined LIKELY
# define LIKELY(_x)	__builtin_expect(!!(_x), 1)
#endif
#if !defined UNLIKELY
# define UNLIKELY(_x)	__builtin_expect(!!(_x), 0)
#endif
#if !defined countof
# define countof(x)	(sizeof(x) / sizeof(*(x)))
#endif	/* !countof */
#if !defined UNUSED
# define UNUSED(x)	__attribute__((unused)) x
#endif	/* !UNUSED */
#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

struct strpt_s {
	unsigned int h;
	unsigned int m;
	unsigned int s;
	unsigned int ns;
	union {
		unsigned int flags;
		struct {
			/* 0 for am, 1 for pm */
			unsigned int am_pm_bit:1;
			/* 0 if no component has been set, 1 otherwise */
			unsigned int component_set:1;
		};
	};
};


/* guessing parsers */
#include "token.c"
#include "strops.c"

#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

static const char hms_dflt[] = "%H:%M:%S";

static struct dt_t_s
__guess_ttyp(struct strpt_s t)
{
#if defined __C1X
	struct dt_t_s res = {
		/* assume all's good for now */
		.typ = DT_HMS,
	};
#else
	struct dt_t_s res;
#endif	/* __C1X */

#if !defined __C1X
	res.typ = DT_HMS;
#endif	/* __C1X */

	if (UNLIKELY(!t.component_set)) {
		goto fucked;
	}
	if (UNLIKELY(t.h == -1U)) {
		goto fucked;
	}
	if (UNLIKELY(t.m == -1U)) {
		goto fucked;
	}
	if (UNLIKELY(t.s == -1U)) {
		goto fucked;
	}
	if (UNLIKELY(t.ns == -1U)) {
		goto fucked;
	}

	res.hms.s = t.s;
	res.hms.m = t.m;
	res.hms.h = t.h;
	res.hms.ns = t.ns;

	if (t.am_pm_bit) {
		/* pm */
		res.hms.h += 12;
		res.hms.h %= HOURS_PER_DAY;
	}
	return res;
fucked:
	res.typ = DT_TUNK;
	res.u = 0;
	return res;
}

static void
__trans_tfmt(const char **fmt)
{
	if (UNLIKELY(*fmt == NULL)) {
		/* don't worry about it */
		*fmt = hms_dflt;
	} if (LIKELY(**fmt == '%')) {
		/* don't worry about it */
		;
	} else if (strcasecmp(*fmt, "hms") == 0) {
		*fmt = hms_dflt;
	}
	return;
}

static int
__strpt_card(struct strpt_s *d, const char *sp, struct dt_spec_s s, char **ep)
{
	int res = 0;

	switch (s.spfl) {
	default:
	case DT_SPFL_UNK:
		res = -1;
		break;
	case DT_SPFL_N_TSTD:
		d->h = strtoui_lim(sp, &sp, 0, 23);
		sp++;
		d->m = strtoui_lim(sp, &sp, 0, 59);
		sp++;
		d->s = strtoui_lim(sp, &sp, 0, 60);
		break;
	case DT_SPFL_N_HOUR:
		if (!s.sc12) {
			d->h = strtoui_lim(sp, &sp, 0, 23);
		} else {
			d->h = strtoui_lim(sp, &sp, 1, 12);
		}
		break;
	case DT_SPFL_N_MIN:
		d->m = strtoui_lim(sp, &sp, 0, 59);
		break;
	case DT_SPFL_N_SEC:
		d->s = strtoui_lim(sp, &sp, 0, 60);
		break;
	case DT_SPFL_N_NANO:
		/* nanoseconds */
		d->ns = strtoui_lim(sp, &sp, 0, 999999999);
		break;
	case DT_SPFL_S_AMPM: {
		const unsigned int casebit = 0x20;

		if ((sp[0] | casebit) == 'a' &&
		    (sp[1] | casebit) == 'm') {
			;
		} else if ((sp[0] | casebit) == 'p' &&
			   (sp[1] | casebit) == 'm') {
			d->am_pm_bit = 1;
		} else {
			res = -1;
		}
		break;
	}
	case DT_SPFL_LIT_PERCENT:
		if (*sp++ != '%') {
			res = -1;
		}
		break;
	case DT_SPFL_LIT_TAB:
		if (*sp++ != '\t') {
			res = -1;
		}
		break;
	case DT_SPFL_LIT_NL:
		if (*sp++ != '\n') {
			res = -1;
		}
		break;
	}
	/* quickly check if any of the conversions has gone wrong */
	if (d->h == -1U ||
	    d->m == -1U ||
	    d->s == -1U ||
	    d->ns == -1U) {
		res = -1;
	} else {
		d->component_set = 1;
	}
	/* assign end pointer */
	if (ep) {
		*ep = (char*)sp;
	}
	return res;
}

static size_t
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
			ui32tostr(buf + 0, bsz, d->h, 2);
			buf[2] = ':';
			ui32tostr(buf + 3, bsz, d->m, 2);
			buf[5] = ':';
			ui32tostr(buf + 6, bsz, d->s, 2);
			res = 8;
		}
		break;
	case DT_SPFL_N_HOUR:
		if (!s.sc12 || (d->h >= 1 && d->h <= 12)) {
			res = ui32tostr(buf, bsz, d->h, 2);
		} else {
			unsigned int h = d->h ? d->h - 12 : 12;
			res = ui32tostr(buf, bsz, h, 2);
		}
		break;
	case DT_SPFL_N_MIN:
		res = ui32tostr(buf, bsz, d->m, 2);
		break;
	case DT_SPFL_N_SEC:
		res = ui32tostr(buf, bsz, d->s, 2);
		break;
	case DT_SPFL_S_AMPM: {
		unsigned int casebit = 0;

		if (UNLIKELY(!s.cap)) {
			casebit = 0x20;
		}
		if (d->h >= 12) {
			buf[res++] = (char)('P' | casebit);
			buf[res++] = (char)('M' | casebit);
		} else {
			buf[res++] = (char)('A' | casebit);
			buf[res++] = (char)('M' | casebit);
		}
		break;
	}
	case DT_SPFL_N_NANO:
		res = ui32tostr(buf, bsz, d->ns, 9);
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


/* parser implementations */
DEFUN struct dt_t_s
dt_strpt(const char *str, const char *fmt, char **ep)
{
#if defined __C1X
	struct dt_t_s res = {.s = -1};
#else
	struct dt_t_s res;
#endif
	struct strpt_s d = {0};
	const char *sp;
	const char *fp;

#if !defined __C1X
/* thanks gcc */
	res.s = -1;
#endif

	/* translate high-level format names */
	__trans_tfmt(&fmt);

	fp = fmt;
	sp = str;
	while (*fp && *sp) {
		const char *fp_sav = fp;
		struct dt_spec_s spec = __tok_spec(fp_sav, (char**)&fp);

		if (spec.spfl == DT_SPFL_UNK) {
			/* must be literal */
			if (*fp_sav != *sp++) {
				sp = str;
				goto out;
			}
		} else if (__strpt_card(&d, sp, spec, (char**)&sp) < 0) {
			sp = str;
			goto out;
		}
	}
		
	/* set the end pointer */
	res = __guess_ttyp(d);
out:
	if (ep) {
		*ep = (char*)sp;
	}
	return res;
}

DEFUN size_t
dt_strft(char *restrict buf, size_t bsz, const char *fmt, struct dt_t_s that)
{
	struct strpt_s d = {0};
	const char *fp;
	char *bp;

	if (UNLIKELY(buf == NULL || bsz == 0)) {
		return 0;
	}

	d.h = that.hms.h;
	d.m = that.hms.m;
	d.s = that.hms.s;
	d.ns = that.hms.ns;
	if (fmt == NULL) {
		fmt = hms_dflt;
	}
	/* translate high-level format names */
	__trans_tfmt(&fmt);

	/* assign and go */
	bp = buf;
	fp = fmt;
	for (char *const eo = buf + bsz; *fp && bp < eo;) {
		const char *fp_sav = fp;
		struct dt_spec_s spec = __tok_spec(fp_sav, (char**)&fp);

		if (spec.spfl == DT_SPFL_UNK) {
			/* must be literal then */
			*bp++ = *fp_sav;
		} else {
			bp += __strft_card(bp, eo - bp, spec, &d, that);
		}
	}
	if (bp < buf + bsz) {
		*bp = '\0';
	}
	return bp - buf;
}


/* arith */
DEFUN struct dt_t_s
dt_tadd(struct dt_t_s t, struct dt_t_s dur)
{
	signed int sec;
	signed int tmp;

	if (!dur.neg) {
		sec = dur.sdur;
	} else {
		sec = -dur.sdur;
	}
	sec += t.hms.s;
	if ((tmp = sec % (signed int)SECS_PER_MIN) >= 0) {
		t.hms.s = tmp;
	} else {
		t.hms.s = tmp + SECS_PER_MIN;
		sec -= SECS_PER_MIN;
	}

	sec /= (signed int)SECS_PER_MIN;
	sec += t.hms.m;
	if ((tmp = sec % (signed int)MINS_PER_HOUR) >= 0) {
		t.hms.m = tmp;
	} else {
		t.hms.m = tmp + MINS_PER_HOUR;
		sec -= MINS_PER_HOUR;
	}

	sec /= (signed int)MINS_PER_HOUR;
	sec += t.hms.h;
	if ((tmp = sec % (signed int)HOURS_PER_DAY) >= 0) {
		t.hms.h = tmp;
	} else {
		t.hms.h = tmp + HOURS_PER_DAY;
	}
	return t;
}

DEFUN struct dt_t_s
dt_tdiff(struct dt_t_s t1, struct dt_t_s t2)
{
/* compute t2 - t1 */
#if defined __C1X
	struct dt_t_s dur = {.u = 0};
#else
	struct dt_t_s dur;
#endif

#if !defined __C1X
/* thanks gcc */
	dur.u = 0;
#endif
	dur.sdur = (t2.hms.h - t1.hms.h) * SECS_PER_HOUR;
	dur.sdur += (t2.hms.m - t1.hms.m) * SECS_PER_MIN;
	dur.sdur += (t2.hms.s - t1.hms.s);
	return dur;
}

DEFUN int
dt_tcmp(struct dt_t_s t1, struct dt_t_s t2)
{
	if (t1.u < t2.u) {
		return -1;
	} else if (t1.u > t2.u) {
		return 1;
	} else {
		return 0;
	}
}

static struct dt_t_s
dt_time(void)
{
#if defined __C1X
	struct dt_t_s res = {.u = 0};
#else
	struct dt_t_s res;
#endif
	struct timeval tv;
	unsigned int tonly;

#if !defined __C1X
/* thanks gcc */
	res.u = 0;
#endif
	if (gettimeofday(&tv, NULL) < 0) {
		return res;
	}

	tonly = tv.tv_sec % 86400U;
	res.hms.h = tonly / SECS_PER_HOUR;
	tonly %= SECS_PER_HOUR;
	res.hms.m = tonly / SECS_PER_MIN;
	tonly %= SECS_PER_MIN;
	res.hms.s = tonly;
	res.hms.ns = tv.tv_usec * 1000;
	return res;
}

#if defined __INTEL_COMPILER
# pragma warning (default:2203)
#elif defined __GNUC__
# pragma GCC diagnostic warning "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

#endif	/* INCLUDED_time_core_c_ */
/* time-core.c ends here */
