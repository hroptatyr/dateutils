/*** date-core.h -- our universe of dates
 *
 * Copyright (C) 2011 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of datetools.
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

#include "date-core.h"

#if defined DEBUG_FLAG
# include <stdio.h>
#endif	/* DEBUG_FLAG */
#if !defined LIKELY
# define LIKELY(_x)	__builtin_expect((_x), 1)
#endif
#if !defined UNLIKELY
# define UNLIKELY(_x)	__builtin_expect((_x), 0)
#endif

/* weekdays of the first day of the year,
 * 3 bits per year, times 10 years makes 1 uint32_t */
typedef struct {
	int y0:3;
	int y1:3;
	int y2:3;
	int y3:3;
	int y4:3;
	int y5:3;
	int y6:3;
	int y7:3;
	int y8:3;
	int y9:3;
	/* 2 bits left */
} __jan01_wday_block_t;

#define M	DT_MONDAY
#define T	DT_TUESDAY
#define W	DT_WEDNESDAY
#define R	DT_THURSDAY
#define F	DT_FRIDAY
#define A	DT_SATURDAY
#define S	DT_SUNDAY


/* helpers */
static const __jan01_wday_block_t __jan01_wday[] = {
	{
		/* 1970 - 1979 */
		R, F, A, M, T, W, R, A, S, M,
	}, {
		/* 1980 - 1989 */
		T, R, F, A, S, T, W, R, F, S,
	}, {
		/* 1990 - 1999 */
		M, T, W, F, A, S, M, W, R, F,
	}, {
		/* 2000 - 2009 */
		A, M, T, W, R, A, S, M, T, R,
	}, {
		/* 2010 - 2019 */
		F, A, S, T, W, R, F, S, M, T,
	}, {
		/* 2020 - 2029 */
		W, F, A, S, M, W, R, F, A, M,
	}
};

/* stolen from Klaus Klein/David Laight's strptime() */
static const char*
conv_num(const char *str, int *dest, uint32_t llim, uint32_t ulim)
{
	int result = 0;
	char ch;
	/* The limit also determines the number of valid digits. */
	int rulim = ulim;

	ch = *str;
	if (ch < '0' || ch > '9') {
		return NULL;
	}
	do {
		result *= 10;
		result += ch - '0';
		rulim /= 10;
		ch = *++str;
	} while ((result * 10 <= ulim) && rulim && ch >= '0' && ch <= '9');

	if (result < llim || result > ulim) {
		return NULL;
	}
	*dest = result;
	return str;
}

static size_t
ltostr(char *restrict buf, size_t bsz, int d, int pad)
{
/* all strings should be little */
#define C(x, d)	((x) / (d) % 10 + '0')
	size_t res;

	if (UNLIKELY(d > 10000 || d < 0)) {
		return 0;
	}
	switch ((res = pad) < bsz ? res : bsz) {
	case 4:
		buf[pad - 4] = C(d, 1000);
	case 3:
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


/* implementations */
DEFUN struct dt_d_s
dt_strpd(const char *str, const char *fmt)
{
	struct dt_d_s res = {DT_UNK, 0};
	int y;
	int m;
	int d;
	int c;

	if (UNLIKELY(fmt == NULL)) {
		return res;
	}

	for (const char *fp = fmt, *sp = str; *fp && sp; fp++) {
		if (*fp == '%') {
			int shaught = 0;
			switch (*++fp) {
			default:
				goto literal;
			case 'F':
				shaught = 1;
			case 'Y':
				sp = conv_num(sp, &y, 0, 9999);
				if (UNLIKELY(shaught == 0 || *sp++ != '-')) {
					break;
				}
			case 'm':
				sp = conv_num(sp, &m, 0, 12);
				if (UNLIKELY(shaught == 0 || *sp++ != '-')) {
					break;
				}
			case 'd':
				/* gregorian mode */
				res.typ = DT_YMD;
				sp = conv_num(sp, &d, 0, 31);
				break;
			case 'w':
				/* ymcd mode */
				res.typ = DT_YMCD;
				sp = conv_num(sp, &d, 0, 7);
				break;
			case 'c':
				/* ymcd mode */
				res.typ = DT_YMCD;
				sp = conv_num(sp, &c, 0, 5);
				break;
			}
		} else {
		literal:
			if (*fp != *sp++) {
				res.typ = DT_UNK;
				break;
			}
		}
	}
	switch (res.typ) {
	default:
	case DT_UNK:
		break;
	case DT_YMD:
		res.ymd.y = y;
		res.ymd.m = m;
		res.ymd.d = d;
		break;
	case DT_YMCD:
		res.ymcd.y = y;
		res.ymcd.m = m;
		res.ymcd.c = c;
		res.ymcd.d = d;
		break;
	}
	return res;
}

DEFUN size_t
dt_strfd(char *restrict buf, size_t bsz, const char *fmt, struct dt_d_s this)
{
	size_t res = 0;
	int y;
	int m;
	int d;
	int c;

	if (UNLIKELY(fmt == NULL || buf == NULL || bsz == 0)) {
		goto out;
	}

	switch (this.typ) {
	default:
	case DT_UNK:
		goto out;
	case DT_YMD:
		y = this.ymd.y;
		m = this.ymd.m;
		d = this.ymd.d;
		break;
	case DT_YMCD:
		y = this.ymcd.y;
		m = this.ymcd.m;
		c = this.ymcd.c;
		d = this.ymcd.d;
		break;
	}

	for (const char *fp = fmt; *fp && res < bsz; fp++) {
		if (*fp == '%') {
			int shaught = 0;
			switch (*++fp) {
			default:
				goto literal;
			case 'F':
				shaught = 1;
			case 'Y':
				res += ltostr(buf + res, bsz - res, y, 4);
				if (UNLIKELY(shaught == 0)) {
					break;
				}
				buf[res++] = '-';
			case 'm':
				res += ltostr(buf + res, bsz - res, m, 2);
				if (UNLIKELY(shaught == 0)) {
					break;
				}
				buf[res++] = '-';
			case 'd':
				/* ymd mode check? */
				res += ltostr(buf + res, bsz - res, d, 2);
				break;
			case 'w':
				/* ymcd mode check? */
				res += ltostr(buf + res, bsz - res, d, 2);
				break;
			case 'c':
				/* ymcd mode check? */
				res += ltostr(buf + res, bsz - res, c, 2);
				break;
			}
		} else {
		literal:
			buf[res++] = *fp;
		}
	}
out:
	if (res < bsz) {
		buf[res] = '\0';
	}
	return res;
}

/* date-core.c ends here */
