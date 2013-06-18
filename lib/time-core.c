/*** time-core.c -- our universe of times
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
/* implementation part of time-core.h */
#if !defined INCLUDED_time_core_c_
#define INCLUDED_time_core_c_

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

#include "time-core.h"
#include "strops.h"
#include "nifty.h"

#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

static inline unsigned int
__secs_since_midnight(struct dt_t_s t)
{
	return (t.hms.h * MINS_PER_HOUR + t.hms.m) * SECS_PER_MIN + t.hms.s;
}


/* guessing parsers */
#include "token.c"
#include "strops.c"
#include "time-core-strpf.c"

static const char hms_dflt[] = "%H:%M:%S";

static struct dt_t_s
__guess_ttyp(struct strpt_s t)
{
	struct dt_t_s res;

	if (UNLIKELY(!(t.flags.h_set || t.flags.m_set ||
		       t.flags.s_set || t.flags.ns_set))) {
		goto fucked;
	}
	if (UNLIKELY(t.h < 0)) {
		goto fucked;
	}
	if (UNLIKELY(t.m < 0)) {
		goto fucked;
	}
	if (UNLIKELY(t.s < 0)) {
		goto fucked;
	}
	if (UNLIKELY(t.ns < 0)) {
		goto fucked;
	}

	res.typ = DT_HMS;
	res.dur = res.neg = 0;
	res.hms.s = t.s;
	res.hms.m = t.m;
	res.hms.h = t.h;
	res.hms.ns = t.ns;

	if (t.flags.am_pm_bit) {
		/* pm */
		res.hms.h += 12;
		res.hms.h %= HOURS_PER_DAY;
	}
	return res;
fucked:
	return dt_t_initialiser();
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


/* parser implementations */
DEFUN struct dt_t_s
dt_strpt(const char *str, const char *fmt, char **ep)
{
	struct dt_t_s res = dt_t_initialiser();
	struct strpt_s d = strpt_initialiser();
	const char *sp;
	const char *fp;

	/* translate high-level format names */
	__trans_tfmt(&fmt);

	fp = fmt;
	sp = str;
	while (*fp && *sp) {
		const char *fp_sav = fp;
		struct dt_spec_s spec = __tok_spec(fp_sav, &fp);

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
	if (ep != NULL) {
		*ep = (char*)sp;
	}
	return res;
}

DEFUN size_t
dt_strft(char *restrict buf, size_t bsz, const char *fmt, struct dt_t_s that)
{
	struct strpt_s d;
	const char *fp;
	char *bp;

	if (UNLIKELY(buf == NULL || bsz == 0)) {
		return 0;
	}

	d = strpt_initialiser();
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
		struct dt_spec_s spec = __tok_spec(fp_sav, &fp);

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

/* arithmetics helpers */
struct divrem_s {
	signed int div;
	unsigned int rem;
};

static struct divrem_s
divrem(signed int n, unsigned int mod)
{
	register signed int _div;
	register signed int _rem;

	_div = n / (signed int)mod;
	if ((_rem = n % (signed int)mod) < 0) {
		_div--;
		_rem += mod;
	}
	return (struct divrem_s){_div, (unsigned int)_rem};
}


/* arith */
DEFUN struct dt_t_s
dt_tadd(struct dt_t_s t, struct dt_t_s dur, int corr)
{
	signed int sec;
	struct divrem_s tmp;

	/* get both result in seconds since midnight */
	sec = (t.hms.h * MINS_PER_HOUR + t.hms.m) * SECS_PER_MIN + t.hms.s;
	sec = sec + dur.sdur;

	if (LIKELY(corr == 0)) {
		tmp = divrem(sec, SECS_PER_DAY);

		/* fill up biggest first */
		t.hms.h = tmp.rem / SECS_PER_HOUR;
		tmp.rem %= SECS_PER_HOUR;
		t.hms.m = tmp.rem / SECS_PER_MIN;
		tmp.rem %= SECS_PER_MIN;
		t.hms.s = tmp.rem;
	} else {
		/* doesn't work if we span more than 1 day */
		tmp = divrem(sec, SECS_PER_DAY + corr);

		/* fill up biggest first */
		if (LIKELY(tmp.rem < SECS_PER_DAY)) {
			t.hms.h = tmp.rem / SECS_PER_HOUR;
			tmp.rem %= SECS_PER_HOUR;
			t.hms.m = tmp.rem / SECS_PER_MIN;
			tmp.rem %= SECS_PER_MIN;
			t.hms.s = tmp.rem;
		} else {
			/* leap-second day case
			 * corr < 0 will always end up in the above case */
			t.hms.h = 23;
			t.hms.m = 59;
			t.hms.s = 59 + corr;
		}
	}

	/* set up the return type */
	t.typ = DT_HMS;
	t.dur = 0;
	t.neg = 0;
	t.carry = tmp.div;
	return t;
}

DEFUN struct dt_t_s
dt_tdiff(struct dt_t_s t1, struct dt_t_s t2)
{
/* compute t2 - t1 */
#if defined HAVE_ANON_STRUCTS_INIT
	struct dt_t_s dur = {.u = 0};
#else
	struct dt_t_s dur;
#endif

#if !defined HAVE_ANON_STRUCTS_INIT
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

DEFUN struct dt_t_s
dt_time(void)
{
#if defined HAVE_ANON_STRUCTS_INIT
	struct dt_t_s res = {.u = 0};
#else
	struct dt_t_s res;
#endif
	struct timeval tv;
	unsigned int tonly;

#if !defined HAVE_ANON_STRUCTS_INIT
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
