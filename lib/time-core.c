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
#include "date-core.h"

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
};


/* helpers */
#if !defined SECS_PER_MINUTE
# define SECS_PER_MINUTE	(60U)
#endif	/* !SECS_PER_MINUTE */
#if !defined SECS_PER_HOUR
# define SECS_PER_HOUR		(SECS_PER_MINUTE * 60U)
#endif	/* !SECS_PER_HOUR */
#if !defined SECS_PER_DAY
# define SECS_PER_DAY		(SECS_PER_HOUR * 24U)
#endif	/* !SECS_PER_DAY */

static const char hms_dflt[] = "%H:%M:%S";

static uint32_t
strtoui_lim(const char *str, const char **ep, uint32_t llim, uint32_t ulim)
{
	uint32_t res = 0;
	const char *sp;
	/* we keep track of the number of digits via rulim */
	uint32_t rulim;

	for (sp = str, rulim = ulim > 10 ? ulim : 10;
	     res * 10 <= ulim && rulim && *sp >= '0' && *sp <= '9';
	     sp++, rulim /= 10) {
		res *= 10;
		res += *sp - '0';
	}
	if (UNLIKELY(sp == str)) {
		res = -1U;
	} else if (UNLIKELY(res < llim || res > ulim)) {
		res = -1U;
	}
	*ep = (char*)sp;
	return res;
}

static size_t
ui32tostr(char *restrict buf, size_t bsz, uint32_t d, int pad)
{
/* all strings should be little */
#define C(x, d)	(char)((x) / (d) % 10 + '0')
	size_t res;

	if (UNLIKELY(d > 10000)) {
		return 0;
	}
	switch ((res = (size_t)pad) < bsz ? res : bsz) {
	case 9:
		/* for nanoseconds */
		buf[pad - 9] = C(d, 100000000);
		buf[pad - 8] = C(d, 10000000);
		buf[pad - 7] = C(d, 1000000);
	case 6:
		/* for microseconds */
		buf[pad - 6] = C(d, 100000);
		buf[pad - 5] = C(d, 10000);
		buf[pad - 4] = C(d, 1000);
	case 3:
		/* for milliseconds */
		buf[pad - 3] = C(d, 100);
	case 2:
		buf[pad - 2] = C(d, 10);
	case 1:
		buf[pad - 1] = C(d, 1);
		break;
	default:
	case 0:
		res = 0;
		break;
	}
	return res;
}


/* guessing parsers */
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
	struct dt_t_s res = {.u = 0};
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
		case 'N':
			/* nanoseconds */
			d.ns = strtoui_lim(sp, &sp, 0, 999999999);
			if (UNLIKELY(d.ns == -1U)) {
				sp = str;
				goto out;
			}
			break;
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

#endif	/* INCLUDED_time_core_c_ */
/* time-core.c ends here */
