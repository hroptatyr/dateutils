/*** time-core.h -- our universe of times
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
/* implementation part of date-core.h */
#if !defined INCLUDED_time_core_c_
#define INCLUDED_time_core_c_

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
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

struct strpt_s {
	unsigned int h;
	unsigned int m;
	unsigned int s;
	unsigned int ns;
	unsigned int flags;
#define STRPT_AM_PM_BIT	(1U)
};


/* guessing parsers */
static const char hms_dflt[] = "%H:%M:%S";

static struct dt_t_s
__guess_ttyp(struct strpt_s t)
{
	struct dt_t_s res;

	if (UNLIKELY(t.h == -1U)) {
		t.h = 0;
	}
	if (UNLIKELY(t.m == -1U)) {
		t.m = 0;
	}
	if (UNLIKELY(t.s == -1U)) {
		t.s = 0;
	}
	if (UNLIKELY(t.ns == -1U)) {
		t.ns = 0;
	}

	res.hms.s = t.s;
	res.hms.m = t.m;
	res.hms.h = t.h;
	res.hms.ns = t.ns;

	if (t.flags & STRPT_AM_PM_BIT) {
		/* pm */
		res.hms.h += 12;
		res.hms.h %= HOURS_PER_DAY;
	}
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


/* parser implementations */
DEFUN struct dt_t_s
dt_strpt(const char *str, const char *fmt, char **ep)
{
	struct dt_t_s res = {.s = -1};
	struct strpt_s d = {0};
	const char *sp = str;

	/* translate high-level format names */
	__trans_tfmt(&fmt);

	for (const char *fp = fmt; *fp && *sp; fp++) {
		int shaught = 0;

		if (*fp != '%') {
		literal:
			if (*fp != *sp++) {
				sp = str;
				goto out;
			}
			continue;
		}
		switch (*++fp) {
		default:
			goto literal;
		case 'T':
			shaught = 1;
		case 'H':
			d.h = strtoui_lim(sp, &sp, 0, 23);
			if (UNLIKELY(shaught == 0)) {
				break;
			} else if (UNLIKELY(d.h == -1U || *sp++ != ':')) {
				sp = str;
				goto out;
			}
		case 'M':
			d.m = strtoui_lim(sp, &sp, 0, 59);
			if (UNLIKELY(shaught == 0)) {
				break;
			} else if (UNLIKELY(d.m == -1U || *sp++ != ':')) {
				sp = str;
				goto out;
			}
		case 'S':
			d.s = strtoui_lim(sp, &sp, 0, 60);
			if (UNLIKELY(d.s == -1U)) {
				sp = str;
				goto out;
			}
			break;
		case 'I':
			d.h = strtoui_lim(sp, &sp, 1, 12);
			if (UNLIKELY(d.h == -1U)) {
				sp = str;
				goto out;
			}
			break;
		case 'N':
			/* nanoseconds */
			d.ns = strtoui_lim(sp, &sp, 0, 999999999);
			if (UNLIKELY(d.ns == -1U)) {
				sp = str;
				goto out;
			}
			break;
		case 'P':
		case 'p': {
			unsigned int casebit = 0;

			if (UNLIKELY(*fp == 'P')) {
				casebit = 0x20;
			}
			if (sp[0] == ('A' | casebit) &&
			    sp[1] == ('M' | casebit)) {
				;
			} else if (sp[0] == ('P' | casebit) &&
				   sp[1] == ('M' | casebit)) {
				d.flags |= STRPT_AM_PM_BIT;
			} else {
				sp = str;
				goto out;
			}
			break;
		}
		case 't':
			if (*sp++ != '\t') {
				sp = str;
				goto out;
			}
			break;
		case 'n':
			if (*sp++ != '\n') {
				sp = str;
				goto out;
			}
			break;
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
	size_t res = 0;
	struct strpt_s d = {0};

	if (UNLIKELY(buf == NULL || bsz == 0)) {
		goto out;
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

	for (const char *fp = fmt; *fp && res < bsz; fp++) {
		int shaught = 0;
		if (*fp != '%') {
		literal:
			buf[res++] = *fp;
			continue;
		}
		switch (*++fp) {
		default:
			goto literal;
		case 'T':
			shaught = 1;
		case 'H':
			res += ui32tostr(buf + res, bsz - res, d.h, 2);
			if (UNLIKELY(shaught == 0)) {
				break;
			}
			buf[res++] = ':';
		case 'M':
			res += ui32tostr(buf + res, bsz - res, d.m, 2);
			if (UNLIKELY(shaught == 0)) {
				break;
			}
			buf[res++] = ':';
		case 'S':
			res += ui32tostr(buf + res, bsz - res, d.s, 2);
			break;
		case 'I': {
			unsigned int h;

			if ((h = d.h) > 12) {
				h -= 12;
			} else if (h == 0) {
				h = 12;
			}
			res += ui32tostr(buf + res, bsz - res, h, 2);
			break;
		}
		case 'P':
		case 'p': {
			unsigned int casebit = 0;

			if (UNLIKELY(*fp == 'P')) {
				casebit = 0x20;
			}
			if (d.h >= 12) {
				buf[res++] = (char)('P' | casebit);
				buf[res++] = (char)('M' | casebit);
			} else {
				buf[res++] = (char)('A' | casebit);
				buf[res++] = (char)('M' | casebit);
			}
			break;
		}
		case 'N':
			res += ui32tostr(buf + res, bsz - res, d.ns, 9);
			break;
		case 't':
			/* literal tab */
			buf[res++] = '\t';
			break;
		case 'n':
			/* literal \n */
			buf[res++] = '\n';
			break;
		}
	}
out:
	if (res < bsz) {
		buf[res] = '\0';
	}
	return res;
}


/* arith */
DEFUN struct dt_t_s
dt_tadd(struct dt_t_s t, struct dt_t_s dur)
{
	signed int sec;
	signed int tmp;

	sec = dur.sdur;
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
	struct dt_t_s dur = {.u = 0};

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

#endif	/* INCLUDED_time_core_c_ */
/* time-core.c ends here */
