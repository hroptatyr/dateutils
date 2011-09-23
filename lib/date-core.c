/*** date-core.h -- our universe of dates
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
#if !defined INCLUDED_date_core_c_
#define INCLUDED_date_core_c_

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

/* weekdays of the first day of the year,
 * 3 bits per year, times 10 years makes 1 uint32_t */
typedef struct {
#define __JAN01_Y_PER_B		(10)
	unsigned int y0:3;
	unsigned int y1:3;
	unsigned int y2:3;
	unsigned int y3:3;
	unsigned int y4:3;
	unsigned int y5:3;
	unsigned int y6:3;
	unsigned int y7:3;
	unsigned int y8:3;
	unsigned int y9:3;
	/* 2 bits left */
	unsigned int rest:2;
} __jan01_wday_block_t;

#define M	(unsigned int)(DT_MONDAY)
#define T	(unsigned int)(DT_TUESDAY)
#define W	(unsigned int)(DT_WEDNESDAY)
#define R	(unsigned int)(DT_THURSDAY)
#define F	(unsigned int)(DT_FRIDAY)
#define A	(unsigned int)(DT_SATURDAY)
#define S	(unsigned int)(DT_SUNDAY)

struct strpd_s {
	unsigned int y;
	unsigned int m;
	unsigned int d;
	unsigned int c;
	unsigned int w;
	/* for bizda and other parametrised cals */
	signed int b;
	unsigned int ref;
#define STRPD_BIZDA_BIT	(1U)
	/* general flags */
	unsigned int flags;
};


/* helpers */
#define SECS_PER_MINUTE	(60U)
#define SECS_PER_HOUR	(SECS_PER_MINUTE * 60U)
#define SECS_PER_DAY	(SECS_PER_HOUR * 24U)

static const __jan01_wday_block_t __jan01_wday[] = {
#define __JAN01_WDAY_BEG	(1970)
	{
		/* 1970 - 1979 */
		R, F, A, M, T, W, R, A, S, M, 0,
	}, {
		/* 1980 - 1989 */
		T, R, F, A, S, T, W, R, F, S, 0,
	}, {
		/* 1990 - 1999 */
		M, T, W, F, A, S, M, W, R, F, 0,
	}, {
		/* 2000 - 2009 */
		A, M, T, W, R, A, S, M, T, R, 0,
	}, {
		/* 2010 - 2019 */
		F, A, S, T, W, R, F, S, M, T, 0,
	}, {
		/* 2020 - 2029 */
		W, F, A, S, M, W, R, F, A, M, 0,
	}
#define __JAN01_WDAY_END	(2029)
};

static uint16_t __mon_yday[] = {
/* this is \sum ml, first element is a bit set of leap days to add */
	0xfff8, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

static const char *__long_wday[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
	"Miracleday",
};

static const char *__abbr_wday[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Mir",
};

static const char __abab_wday[] = "SMTWRFAX";

static const char *__long_mon[] = {
	"Miraculary",
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December",
};

static const char *__abbr_mon[] = {
	"Mir",
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
};

/* futures expiry codes, how convenient */
static const char __abab_mon[] = "_FGHJKMNQUVXZ";

static inline char*
__c2p(const char *p)
{
	union {
		char *p;
		const char *c;
	} res = {.c = p};
	return res.p;
}

/* stolen from Klaus Klein/David Laight's strptime() */
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

static uint32_t
__romstr_v(const char c)
{
	switch (c) {
	case 'n':
	case 'N':
		return 0;
	case 'i':
	case 'I':
		return 1;
	case 'v':
	case 'V':
		return 5;
	case 'x':
	case 'X':
		return 10;
	case 'l':
	case 'L':
		return 50;
	case 'c':
	case 'C':
		return 100;
	case 'd':
	case 'D':
		return 500;
	case 'm':
	case 'M':
		return 1000;
	default:
		return -1U;
	}
}

static uint32_t
romstrtoui_lim(const char *str, const char **ep, uint32_t llim, uint32_t ulim)
{
	uint32_t res = 0;
	const char *sp;
	uint32_t v;

	/* loops through characters */
	for (sp = str, v = __romstr_v(*sp); *sp; sp++) {
		uint32_t nv = __romstr_v(sp[1]);

		if (UNLIKELY(v == -1U)) {
			break;
		} else if (LIKELY(nv == -1U || v >= nv)) {
			res += v;
		} else {
			res -= v;
		}
		v = nv;
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
__rom_pr1(char *buf, size_t bsz, unsigned int i, char cnt, char hi, char lo)
{
	size_t res = 0;

	if (UNLIKELY(bsz < 4)) {
		return 0;
	}
	switch (i) {
	case 9:
		buf[res++] = cnt;
		buf[res++] = hi;
		break;
	case 4:
		buf[res++] = cnt;
		buf[res++] = lo;
		break;
	case 8:
		buf[++res] = cnt;
	case 7:
		buf[++res] = cnt;
	case 6:
		buf[++res] = cnt;
	case 5:
		buf[0] = lo;
		res++;
		break;
	case 3:
		buf[res++] = cnt;
	case 2:
		buf[res++] = cnt;
	case 1:
		buf[res++] = cnt;
		break;
	}
	return res;
}

static size_t
ui32tostrrom(char *restrict buf, size_t bsz, uint32_t d)
{
	size_t res;

	for (res = 0; d >= 1000 && res < bsz; d -= 1000) {
		buf[res++] = 'M';
	}

	res += __rom_pr1(buf + res, bsz - res, d / 100U, 'C', 'M', 'D');
	d %= 100;
	res += __rom_pr1(buf + res, bsz - res, d / 10U, 'X', 'C', 'L');
	d %= 10;
	res += __rom_pr1(buf + res, bsz - res, d, 'I', 'X', 'V');
	return res;
}

static uint32_t
strtoarri(const char *buf, const char **ep, const char *const *arr, size_t narr)
{
	for (size_t i = 0; i < narr; i++) {
		const char *chk = arr[i];
		size_t len = strlen(chk);

		if (strncasecmp(chk, buf, len) == 0) {
			*ep = buf + len;
			return i;
		}
	}
	/* no matches */
	*ep = buf;
	return -1U;
}

static size_t
arritostr(
	char *restrict buf, size_t bsz, size_t i,
	const char *const *arr, size_t narr)
{
	size_t ncp;
	size_t len;

	if (i > narr) {
		return 0;
	}
	len = strlen(arr[i]);
	ncp = bsz > len ? len : bsz;
	memcpy(buf, arr[i], ncp);
	return ncp;
}

static inline bool
__leapp(unsigned int y)
{
	return y % 4 == 0;
}

static void
ffff_gmtime(struct tm *tm, const time_t t)
{
	register int days;
	register unsigned int yy;
	const uint16_t *ip;

	/* just go to day computation */
	days = (typeof(days))(t / SECS_PER_DAY);
	/* week day computation, that one's easy, 1 jan '70 was Thu */
	tm->tm_wday = (days + 4) % 7;

	/* gotta do the date now */
	yy = 1970;
	/* stolen from libc */
#define DIV(a, b)		((a) / (b))
/* we only care about 1901 to 2099 and there are no bullshit leap years */
#define LEAPS_TILL(y)		(DIV(y, 4))
	while (days < 0 || days >= (!__leapp(yy) ? 365 : 366)) {
		/* Guess a corrected year, assuming 365 days per year. */
		register unsigned int yg = yy + days / 365 - (days % 365 < 0);

		/* Adjust DAYS and Y to match the guessed year.  */
		days -= (yg - yy) * 365 +
			LEAPS_TILL(yg - 1) - LEAPS_TILL(yy - 1);
		yy = yg;
	}
	/* set the year */
	tm->tm_year = (int)yy;

	ip = __mon_yday;
	/* unrolled */
	yy = 13;
	if (days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy]) {
		yy = 0;
	}
	/* set the rest of the tm structure */
	tm->tm_mday = days - ip[yy] + 1;
	tm->tm_yday = days;
	tm->tm_mon = (int)yy;
	/* fix up leap years */
	if (UNLIKELY(__leapp(tm->tm_year))) {
		if ((ip[0] >> (yy)) & 1) {
			if (UNLIKELY(tm->tm_yday == 59)) {
				tm->tm_mon = 2;
				tm->tm_mday = 29;
			} else if (UNLIKELY(tm->tm_yday == ip[yy])) {
				tm->tm_mday = tm->tm_yday - ip[--tm->tm_mon];
			} else {
				tm->tm_mday--;
			}
		}
	}
	return;
}


/* converters and getters */
static inline __jan01_wday_block_t
__get_jan01_block(unsigned int year)
{
	return __jan01_wday[(year - __JAN01_WDAY_BEG) / __JAN01_Y_PER_B];
}

static inline dt_daisy_t
__jan00_daisy(unsigned int year)
{
/* daisy's base year is both 1 mod 4 and starts on a monday, so ... */
#define TO_BASE(x)	((x) - DT_DAISY_BASE_YEAR)
#define TO_YEAR(x)	((x) + DT_DAISY_BASE_YEAR)
	int by = TO_BASE(year);
	return by * 365 + by / 4;
}

static inline dt_dow_t
__get_jan01_wday(unsigned int year)
{
/* get the weekday of jan01 in YEAR */
	unsigned int res;
	__jan01_wday_block_t j01b;

	if (UNLIKELY(year < __JAN01_WDAY_BEG || year > __JAN01_WDAY_END)) {
		return DT_MIRACLEDAY;
	}
	j01b = __get_jan01_block(year);

	switch (year % 10) {
	default:
		res = DT_MIRACLEDAY;
		break;
	case 0:
		res = j01b.y0;
		break;
	case 1:
		res = j01b.y1;
		break;
	case 2:
		res = j01b.y2;
		break;
	case 3:
		res = j01b.y3;
		break;
	case 4:
		res = j01b.y4;
		break;
	case 5:
		res = j01b.y5;
		break;
	case 6:
		res = j01b.y6;
		break;
	case 7:
		res = j01b.y7;
		break;
	case 8:
		res = j01b.y8;
		break;
	case 9:
		res = j01b.y9;
		break;
	}
	return (dt_dow_t)res;
}

static dt_dow_t
__get_m01_wday(unsigned int year, unsigned int mon)
{
/* get the weekday of the first of MONTH in YEAR */
	unsigned int off;
	dt_dow_t cand;

	if (UNLIKELY(mon < 1 && mon > 12)) {
		return DT_MIRACLEDAY;
	}
	cand = __get_jan01_wday(year);
	off = __mon_yday[mon] % 7;
	/* fixup leap years */
	if (UNLIKELY(__leapp(year))) {
		off += (__mon_yday[0] >> mon) & 1;
	}
	return (dt_dow_t)(cand + off);
}

static inline unsigned int
__get_mdays(unsigned int y, unsigned int m)
{
	unsigned int res;

	if (UNLIKELY(m < 1 || m > 12)) {
		return 0;
	}

	/* use our cumulative yday array */
	res = __mon_yday[m + 1] - __mon_yday[m];

	/* fixup leap years */
	if (UNLIKELY(__leapp(y) && m == 2)) {
		res++;
	}
	return res;
}

static unsigned int
__ymd_get_yday(dt_ymd_t that)
{
	unsigned int res;

	if (UNLIKELY(that.y == 0 || that.m == 0 || that.m > 12)) {
		return 0;
	}
	/* process */
	res = that.d + __mon_yday[that.m];
	/* fixup leap years */
	if (UNLIKELY(__leapp(that.y))) {
		res += (__mon_yday[0] >> that.m) & 1;
	}
	return res;
}

static dt_dow_t
__ymd_get_wday(dt_ymd_t that)
{
	unsigned int yd;
	dt_dow_t j01_wd;
	if ((yd = __ymd_get_yday(that)) > 0 &&
	    (j01_wd = __get_jan01_wday(that.y)) != DT_MIRACLEDAY) {
		return (dt_dow_t)((yd - 1 + (unsigned int)j01_wd) % 7);
	}
	return DT_MIRACLEDAY;
}

static unsigned int
__ymd_get_count(dt_ymd_t that)
{
	if (UNLIKELY(that.d + 7U > __get_mdays(that.y, that.m))) {
		return 5;
	}
	return (that.d - 1U) / 7U + 1U;
}

static dt_dow_t
__ymcw_get_wday(dt_ymcw_t that)
{
	return (dt_dow_t)that.w;
}

static unsigned int
__ymcw_get_yday(dt_ymcw_t that)
{
/* return the N-th W-day in Y, this is equivalent with 8601's Y-W-D calendar
 * where W is the week of the year and D the day in the week */
/* if a year starts on W, then it's
 * 5 Ws in jan
 * 4 Ws in feb
 * 4 Ws in mar
 * 5 Ws in apr
 * 4 Ws in may
 * 4 Ws in jun
 * 5 Ws in jul
 * 4 Ws in aug
 * 4 + leap Ws in sep
 * 5 - leap Ws in oct
 * 4 Ws in nov
 * 5 Ws in dec,
 * so go back to the last W, and compute its number instead */
	/* we guess the number of Ws up to the previous month */
	unsigned int ws = 4 * (that.m - 1);
	dt_dow_t j01w = __get_jan01_wday(that.y);
	dt_dow_t m01w = __get_m01_wday(that.y, that.m);

	switch (that.m) {
	case 10:
		ws += 3 + (__leapp(that.y) ? 1 : 0);
		break;
	case 11:
		ws++;
	case 9:
	case 8:
		ws++;
	case 7:
	case 6:
	case 5:
		ws++;
	case 4:
	case 3:
	case 2:
		ws++;
	case 1:
		break;
	}
	/* now find the count of the last W before/eq today */
	if (m01w <= j01w &&
	    that.w >= m01w && that.w < j01w) {
		ws--;
	} else if (that.w >= m01w || that.w < j01w) {
		ws--;
	}
	return ws + that.c;
}

static unsigned int
__ymcw_get_mday(dt_ymcw_t that)
{
	unsigned int wd01;
	unsigned int wd_jan01;
	unsigned int res;

	/* weekday the year started with */
	wd_jan01 = __get_jan01_wday(that.y);
	/* see what weekday the first of the month was*/
	wd01 = __ymd_get_yday((dt_ymd_t){.y = that.y, .m = that.m, .d = 01});
	wd01 = (wd_jan01 - 1 + wd01) % 7;

	/* first WD1 is 1, second WD1 is 8, third WD1 is 15, etc.
	 * so the first WDx with WDx > WD1 is on (WDx - WD1) + 1 */
	res = (that.w + 7 - wd01) % 7 + 1 + 7 * (that.c - 1);
	/* not all months have a 5th X, so check for this */
	if (res > __get_mdays(that.y, that.m)) {
		 /* 5th = 4th in that case */
		res -= 7;
	}
	return res;
}

static int
__ymd_get_bday(dt_ymd_t that, unsigned int ba, unsigned int ref)
{
	dt_dow_t wdd;
	unsigned int wk;
	int res;

	if (ba != BIZDA_AFTER || ref != BIZDA_ULTIMO) {
		/* no support yet */
		return -1;
	}

	/* weekday the month started with */
	switch ((wdd = __ymd_get_wday(that))) {
	case DT_SUNDAY:
	case DT_SATURDAY:
		return -1;
	default:
		break;
	}
	wk = (that.d - 1) / 7; 
	res = wk * 5 + (unsigned int)wdd;
	return res;
}

static int
__ymcw_get_bday(dt_ymcw_t that, unsigned int ba, unsigned int ref)
{
	dt_dow_t wd01;
	int res;

	switch (that.w) {
	case DT_SUNDAY:
	case DT_SATURDAY:
		return -1;
	default:
		break;
	}
	if (ba != BIZDA_AFTER || ref != BIZDA_ULTIMO) {
		/* no support yet */
		return -1;
	}

	/* weekday the month started with */
	wd01 = __ymd_get_wday((dt_ymd_t){.y = that.y, .m = that.m, .d = 01});
	res = (signed int)(that.w - wd01) + 5 * (that.c) + 1;
	return res;
}

static unsigned int
__bizda_get_mday(dt_bizda_t that)
{
	dt_dow_t wd01;
	unsigned int res;

	/* find first of the month first */
	wd01 = __ymd_get_wday((dt_ymd_t){.y = that.y, .m = that.m, .d = 01});
	switch (wd01) {
	case DT_MONDAY:
	case DT_TUESDAY:
	case DT_WEDNESDAY:
	case DT_THURSDAY:
	case DT_FRIDAY:
		res = 1;
		break;
	case DT_SATURDAY:
		wd01 = DT_MONDAY;
		res = 3;
		break;
	case DT_SUNDAY:
		wd01 = DT_MONDAY;
		res = 2;
		break;
	case DT_MIRACLEDAY:
	default:
		res = 0;
		break;
	}
	/* now just add up bdays */
	{
		unsigned int wk;
		unsigned int nd;
		unsigned int b = that.bd;
		unsigned int magic = (b - 1 + wd01 - 1);

		wk = magic / 5;
		nd = magic % 5;
		res += wk * 7 + nd - wd01 + 1;
	}
	/* fixup mdays */
	if (res > __get_mdays(that.y, that.m)) {
		res = 0;
	}
	return res;
}

static dt_dow_t
__bizda_get_wday(dt_bizda_t that)
{
	dt_dow_t wd01;
	unsigned int b;
	unsigned int magic;

	/* find first of the month first */
	wd01 = __ymd_get_wday((dt_ymd_t){.y = that.y, .m = that.m, .d = 01});
	b = that.bd;
	magic = (b - 1 + (wd01 ?: 6) - 1);
	/* now just add up bdays */
	return (dt_dow_t)((magic % 5) + DT_MONDAY);
}

static dt_ymcw_t
__ymd_to_ymcw(dt_ymd_t d)
{
	unsigned int c = __ymd_get_count(d);
	unsigned int w = __ymd_get_wday(d);
	return (dt_ymcw_t){.y = d.y, .m = d.m, .c = c, .w = w};
}

static dt_dow_t
__daisy_get_wday(dt_daisy_t d)
{
/* daisy wdays are simple because the base year is chosen so that day 0
 * in the daisy calendar is a sunday */
	return (dt_dow_t)(d % 7);
}

static unsigned int
__daisy_get_year(dt_daisy_t d)
{
/* given days since 1917-01-01 (Mon), compute a year */
	int by;

	if (UNLIKELY(d == 0)) {
		return 0;
	}
	for (by = d / 365; __jan00_daisy(TO_YEAR(by)) >= d; by--);
	return TO_YEAR(by);
}

static dt_ymd_t
__daisy_to_ymd(dt_daisy_t that)
{
	dt_daisy_t j00;
	unsigned int doy;
	unsigned int y;
	unsigned int m;
	unsigned int d;

	if (UNLIKELY(that == 0)) {
		return (dt_ymd_t){.u = 0};
	}
	y = __daisy_get_year(that);
	j00 = __jan00_daisy(y);
	doy = that - j00;
	for (m = 1; m < 12 && doy > __mon_yday[m + 1]; m++);
	d = doy - __mon_yday[m];

	/* fix up leap years */
	if (UNLIKELY(__leapp(y))) {
		if ((__mon_yday[0] >> (m)) & 1) {
			if (UNLIKELY(doy == 60)) {
				m = 2;
				d = 29;
			} else if (UNLIKELY(doy == __mon_yday[m] + 1U)) {
				m--;
				d = doy - __mon_yday[m] - 1U;
			} else {
				d--;
			}
		}
	}
	return (dt_ymd_t){.y = y, .m = m, .d = d};
}

static dt_ymcw_t
__daisy_to_ymcw(dt_daisy_t that)
{
	dt_ymd_t tmp;
	unsigned int c;
	unsigned int w;

	if (UNLIKELY(that == 0)) {
		return (dt_ymcw_t){.u = 0};
	}
	tmp = __daisy_to_ymd(that);
	c = __ymd_get_count(tmp);
	w = __daisy_get_wday(that);
	return (dt_ymcw_t){.y = tmp.y, .m = tmp.m, .c = c, .w = w};
}

static dt_ymd_t
__ymcw_to_ymd(dt_ymcw_t d)
{
	unsigned int md = __ymcw_get_mday(d);
	return (dt_ymd_t){.y = d.y, .m = d.m, .d = md};
}


/* converting accessors */
DEFUN int
dt_get_year(struct dt_d_s that)
{
	switch (that.typ) {
	case DT_YMD:
		return that.ymd.y;
	case DT_YMCW:
		return that.ymcw.y;
	case DT_DAISY:
		return 0;
	case DT_BIZDA:
		return that.bizda.y;
	default:
	case DT_UNK:
		return 0;
	}
}

DEFUN int
dt_get_mon(struct dt_d_s that)
{
	switch (that.typ) {
	case DT_YMD:
		return that.ymd.m;
	case DT_YMCW:
		return that.ymcw.m;
	case DT_DAISY:
		return 0;
	case DT_BIZDA:
		return that.bizda.m;
	default:
	case DT_UNK:
		return 0;
	}
}

DEFUN dt_dow_t
dt_get_wday(struct dt_d_s that)
{
	switch (that.typ) {
	case DT_YMD:
		return __ymd_get_wday(that.ymd);
	case DT_YMCW:
		return __ymcw_get_wday(that.ymcw);
	case DT_DAISY:
		return __daisy_get_wday(that.daisy);
	case DT_BIZDA:
		return __bizda_get_wday(that.bizda);
	default:
	case DT_UNK:
		return DT_MIRACLEDAY;
	}
}

DEFUN int
dt_get_mday(struct dt_d_s that)
{
	if (LIKELY(that.typ == DT_YMD)) {
		return that.ymd.d;
	}
	switch (that.typ) {
	case DT_YMCW:
		return __ymcw_get_mday(that.ymcw);
	case DT_DAISY: {
		dt_ymd_t tmp = __daisy_to_ymd(that.daisy);
		return tmp.m;
	}
	case DT_BIZDA:
		return __bizda_get_mday(that.bizda);;
	default:
	case DT_UNK:
		return 0;
	}
}

/* too exotic to be public */
static int
dt_get_count(struct dt_d_s that)
{
	if (LIKELY(that.typ == DT_YMCW)) {
		return that.ymcw.c;
	}
	switch (that.typ) {
	case DT_YMD:
		return __ymd_get_count(that.ymd);
	default:
	case DT_UNK:
		return 0;
	}
}

DEFUN unsigned int
dt_get_yday(struct dt_d_s that)
{
	switch (that.typ) {
	case DT_YMD:
		return __ymd_get_yday(that.ymd);
	case DT_YMCW:
		return __ymcw_get_yday(that.ymcw);
	default:
	case DT_UNK:
		return 0;
	}
}

DEFUN int
dt_get_bday(struct dt_d_s that)
{
/* get N where N is the N-th business day Before/After REF */
	switch (that.typ) {
	case DT_BIZDA:
		if (that.bizda.x == BIZDA_ULTIMO &&
		    that.bizda.ba == BIZDA_AFTER) {
			return that.bizda.bd;
		}
		return 0;
	case DT_DAISY:
		that.ymd = __daisy_to_ymd(that.daisy);
	case DT_YMD:
		return __ymd_get_bday(that.ymd, BIZDA_AFTER, BIZDA_ULTIMO);
	case DT_YMCW:
		return __ymcw_get_bday(that.ymcw, BIZDA_AFTER, BIZDA_ULTIMO);
	default:
	case DT_UNK:
		return 0;
	}
}


/* converters */
static dt_daisy_t
dt_conv_to_daisy(struct dt_d_s that)
{
	dt_daisy_t res;
	unsigned int y;
	unsigned int m;
	unsigned int d;

	if (that.typ == DT_DAISY) {
		return that.daisy;
	} else if (that.typ == DT_UNK) {
		return 0;
	}

	y = dt_get_year(that);
	m = dt_get_mon(that);
	d = dt_get_mday(that);

	if (UNLIKELY((signed int)TO_BASE(y) < 0)) {
		return 0;
	}
	res = __jan00_daisy(y);
	res += __mon_yday[m];
	/* add up days too */
	res += d;
	if (UNLIKELY(__leapp(y))) {
		res += (__mon_yday[0] >> (m)) & 1;
	}
	return res;
}

static dt_ymd_t
dt_conv_to_ymd(struct dt_d_s that)
{
	switch (that.typ) {
	case DT_YMD:
		return that.ymd;
	case DT_YMCW:
		return __ymcw_to_ymd(that.ymcw);
	case DT_DAISY:
		return __daisy_to_ymd(that.daisy);
	case DT_BIZDA:
		break;
	case DT_UNK:
	default:
		break;
	}
	return (dt_ymd_t){.u = 0};
}

static dt_ymcw_t
dt_conv_to_ymcw(struct dt_d_s that)
{
	switch (that.typ) {
	case DT_YMD:
		return __ymd_to_ymcw(that.ymd);
	case DT_YMCW:
		return that.ymcw;
	case DT_DAISY:
		return __daisy_to_ymcw(that.daisy);
	case DT_BIZDA:
		break;
	case DT_UNK:
	default:
		break;
	}
	return (dt_ymcw_t){.u = 0};
}


/* arithmetic */
static dt_daisy_t
__daisy_add(dt_daisy_t d, struct dt_dur_s dur)
{
	switch (dur.typ) {
	case DT_DUR_WD:
		d += dur.wd.w * 7 + dur.wd.d;
		break;
	case DT_DUR_MD:
	case DT_DUR_YM:
		/* daisies have no notion of years and months */
	case DT_DUR_UNK:
	default:
		break;
	}
	return d;
}

static struct dt_dur_s
__daisy_diff(dt_daisy_t d1, dt_daisy_t d2)
{
/* compute d2 - d1 */
	struct dt_dur_s res = {.typ = DT_DUR_WD};
	int32_t diff = d2 - d1;

#if 0
	res.wd.d = diff;
	res.wd.w = 0;
#elif 1
	res.wd.w = diff / 7;
	res.wd.d = diff % 7;
#endif
	return res;
}

static dt_ymd_t
__ymd_add(dt_ymd_t d, struct dt_dur_s dur)
{
	unsigned int tgty = 0;
	unsigned int tgtm = 0;
	int tgtd = 0;

	switch (dur.typ) {
		int tmp;
		unsigned int mdays;
	case DT_DUR_YM:
	case DT_DUR_MD:
		/* init tmp */
		switch (dur.typ) {
		case DT_DUR_YM:
			tmp = dur.ym.y * 12 + dur.ym.m;
			break;
		case DT_DUR_MD:
			tmp = dur.md.m;
			break;
		}

		/* construct new month */
		tmp += d.m - 1;
		tgty = tmp / 12 + d.y;
		tgtm = tmp % 12 + 1;

		/* fixup day */
		if ((tgtd = d.d) > (int)(mdays = __get_mdays(tgty, tgtm))) {
			tgtd = mdays;
		}
		if (dur.typ == DT_DUR_YM) {
			/* we dont have to bother about day corrections */
			break;
		}
		/* otherwise we may need to fixup the day, let's do that
		 * in the next step */
	case DT_DUR_WD:
		if (dur.typ == DT_DUR_MD) {
			/* fallthrough from above */
			tgtd += dur.md.d;
		} else {
			tgtd = d.d + dur.wd.w * 7 + dur.wd.d;
			mdays = __get_mdays((tgty = d.y), (tgtm = d.m));
		}
		/* fixup the day */
		while (tgtd > (int)mdays) {
			tgtd -= mdays;
			if (++tgtm > 12) {
				++tgty;
				tgtm = 1;
			}
			mdays = __get_mdays(tgty, tgtm);
		}
		/* and the other direction */
		while (tgtd < 1) {
			if (--tgtm < 1) {
				--tgty;
				tgtm = 12;
			}
			mdays = __get_mdays(tgty, tgtm);
			tgtd += mdays;
		}
		break;
	case DT_DUR_UNK:
	default:
		break;
	}
	d.y = tgty;
	d.m = tgtm;
	d.d = tgtd;
	return d;
}

static struct dt_dur_s
__ymd_diff(dt_ymd_t d1, dt_ymd_t d2)
{
/* compute d2 - d1 entirely in terms of ymd */
	struct dt_dur_s res = {.typ = DT_DUR_MD};
	signed int tgtd = 0;
	signed int tgtm = 0;

	/* first compute the difference in months Y2-M2-01 - Y1-M1-01 */
	tgtm = 12 * (d2.y - d1.y) + (d2.m - d1.m);
	if ((tgtd = d2.d - d1.d) < 1 && tgtm != 0) {
		/* if tgtm is 0 it remains 0 and tgtd remains negative */
		/* get the target month's mdays */
		unsigned int d2m = d2.m;
		unsigned int d2y = d2.y;

		if (--d2m < 1) {
			d2m = 12;
			d2y--;
		}
		tgtd += __get_mdays(d2y, d2m);
		tgtm--;
	}
	/* fill in the results */
	res.md.m = tgtm;
	res.md.d = tgtd;
	return res;
}

static dt_ymcw_t
__ymcw_add(dt_ymcw_t d, struct dt_dur_s dur)
{
	switch (dur.typ) {
	case DT_DUR_YM:
		d.y += dur.ym.y;
		d.m += dur.ym.m;
		break;
	case DT_DUR_MD:
		d.y += dur.md.m / 12;
		d.m += dur.md.m % 12;
		d.c += dur.md.d / 7;
		d.w += dur.md.d % 7;
		break;
	case DT_DUR_WD:
		d.c += dur.wd.w;
		d.w += dur.wd.d;
		break;
	case DT_DUR_UNK:
	default:
		break;
	}
	return d;
}


/* guessing parsers */
static struct dt_d_s
__guess_dtyp(struct strpd_s d)
{
	struct dt_d_s res;
	bool bizdap;

	if (UNLIKELY(d.y == -1U)) {
		d.y = 0;
	}
	if (UNLIKELY(d.m == -1U)) {
		d.m = 0;
	}
	if (UNLIKELY(d.d == -1U)) {
		d.d = 0;
	}
	if (UNLIKELY(d.w == -1U)) {
		d.w = 0;
	}
	if (UNLIKELY(d.c == -1U)) {
		d.c = 0;
	}

	bizdap = (d.flags & STRPD_BIZDA_BIT);
	if (LIKELY(d.y && (d.m == 0 || d.c == 0) && !bizdap)) {
		/* nearly all goes to ymd */
		res.typ = DT_YMD;
		res.ymd = (dt_ymd_t){
			.y = d.y,
			.m = d.m,
			.d = d.d,
		};
	} else if (d.y && d.c && !bizdap) {
		/* its legit for d.w to be naught */
		res.typ = DT_YMCW;
		res.ymcw = (dt_ymcw_t){
			.y = d.y,
			.m = d.m,
			.c = d.c,
			.w = d.w,
		};
	} else if (d.y && bizdap) {
		/* d.c can be legit'ly naught */
		res.typ = DT_BIZDA;
		res.bizda = (dt_bizda_t){
			.y = d.y,
			.m = d.m,
			.bda = d.b,
			.x = d.ref,
		};
	} else {
		/* anything else is bollocks for now */
		res.typ = DT_UNK;
		res.u = 0;
	}
	return res;
}

static char ymd_dflt[] = "%F";
static char ymcw_dflt[] = "%Y-%m-%c-%w";
static char bizda_dflt[] = "%Y-%m-%_d%q";
static char *bizda_ult[] = {"ultimo", "ult"};

static void
__trans_fmt(const char **fmt)
{
	if (LIKELY(**fmt == '%')) {
		/* don't worry about it */
		;
	} else if (strcasecmp(*fmt, "ymd") == 0) {
		*fmt = ymd_dflt;
	} else if (strcasecmp(*fmt, "ymcw") == 0) {
		*fmt = ymcw_dflt;
	} else if (strcasecmp(*fmt, "bizda") == 0) {
		*fmt = bizda_dflt;
	}
	return;
}

static struct dt_d_s
__strpd_std(const char *str, char **ep)
{
	struct dt_d_s res = {.typ = DT_UNK, .u = 0};
	struct strpd_s d = {0};
	const char *sp;

	if ((sp = str) == NULL) {
		goto out;
	}
	/* read the year */
	if ((d.y = strtoui_lim(sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR)) == -1U ||
	    *sp++ != '-') {
		sp = str;
		goto out;
	}
	/* read the month */
	if ((d.m = strtoui_lim(sp, &sp, 0, 12)) == -1U ||
	    *sp++ != '-') {
		sp = str;
		goto out;
	}
	/* read the day or the count */
	if ((d.d = strtoui_lim(sp, &sp, 0, 31)) == -1U) {
		/* didn't work, fuck off */
		sp = str;
		goto out;
	}
	/* check the date type */
	switch (*sp) {
	case '-':
		/* it is a YMCW date */
		if ((d.c = d.d) > 5) {
			/* nope, it was bollocks */
			break;
		}
		d.d = 0;
		if ((d.w = strtoui_lim(++sp, &sp, 0, 7)) == -1U) {
			/* didn't work, fuck off */
			sp = str;
			goto out;
		}
		break;
	case '<':
		/* it's a bizda/YMDU date */
		d.b = -d.d;
		goto on;
	case '>':
		/* it's a bizda/YMDU date */
		d.b = d.d;
	on:
		d.d = 0;
		d.flags |= STRPD_BIZDA_BIT;
		if (strtoarri(++sp, &sp, bizda_ult, countof(bizda_ult)) < -1U ||
		    (d.ref = strtoui_lim(sp, &sp, 0, 23)) < -1U) {
			break;
		}
		sp = str;
		goto out;
	default:
		/* we don't care */
		break;
	}
	/* guess what we're doing */
	res = __guess_dtyp(d);
out:
	if (ep) {
		*ep = (char*)sp;
	}
	return res;
}

static size_t
__strfd_O(char *buf, size_t bsz, const char spec, struct dt_d_s that)
{
	size_t res = 0;
	struct strpd_s d;

	switch (that.typ) {
	case DT_YMD:
		d.y = that.ymd.y;
		d.m = that.ymd.m;
		d.d = that.ymd.d;
		break;
	case DT_YMCW:
		d.y = that.ymcw.y;
		d.m = that.ymcw.m;
		d.d = __ymcw_get_mday(that.ymcw);
		break;
	case DT_DAISY: {
		dt_ymd_t tmp = __daisy_to_ymd(that.daisy);
		d.y = tmp.y;
		d.m = tmp.m;
		d.d = tmp.d;
		break;
	}
	case DT_BIZDA:
	default:
		return 0;
	}

	if (that.typ != DT_YMD) {
		/* not supported for non-ymds */
		return res;
	}

	switch (spec) {
	case 'Y':
		res = ui32tostrrom(buf, bsz, d.y);
		break;
	case 'y':
		res = ui32tostrrom(buf, bsz, d.y % 100);
		break;
	case 'm':
		res = ui32tostrrom(buf, bsz, d.m);
		break;
	case 'd':
		res = ui32tostrrom(buf, bsz, d.d);
		break;
	case 'c':
		d.c = dt_get_count(that);
		res = ui32tostrrom(buf, bsz, d.c);
		break;
	default:
		break;
	}
	return res;
}


/* parser implementations */
DEFUN struct dt_d_s
dt_strpd(const char *str, const char *fmt, char **ep)
{
	struct dt_d_s res = {.typ = DT_UNK, .u = 0};
	struct strpd_s d = {0};
	const char *sp = str;

	if (UNLIKELY(fmt == NULL)) {
		return __strpd_std(str, ep);
	}
	/* translate high-level format names */
	__trans_fmt(&fmt);

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
		case 'F':
			shaught = 1;
		case 'Y':
			d.y = strtoui_lim(sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR);
			if (UNLIKELY(shaught == 0)) {
				break;
			} else if (UNLIKELY(d.y == -1U || *sp++ != '-')) {
				sp = str;
				goto out;
			}
		case 'm':
			d.m = strtoui_lim(sp, &sp, 0, 12);
			if (UNLIKELY(shaught == 0)) {
				break;
			} else if (UNLIKELY(d.m == -1U || *sp++ != '-')) {
				sp = str;
				goto out;
			}
		case 'd':
			/* gregorian mode */
			if ((d.d = strtoui_lim(sp, &sp, 0, 31)) == -1U) {
				sp = str;
				goto out;
			}
			break;
		case 'w':
			/* ymcw mode */
			if ((d.w = strtoui_lim(sp, &sp, 0, 7)) == -1U) {
				sp = str;
				goto out;
			}
			break;
		case 'c':
			/* ymcw mode */
			if ((d.c = strtoui_lim(sp, &sp, 0, 5)) == -1U) {
				sp = str;
				goto out;
			}
			break;
		case 'a':
			/* ymcw mode! */
			d.w = strtoarri(
				sp, &sp,
				__abbr_wday, countof(__abbr_wday));
			break;
		case 'A':
			/* ymcw mode! */
			d.w = strtoarri(
				sp, &sp,
				__long_wday, countof(__long_wday));
			break;
		case 'b':
		case 'h':
			d.m = strtoarri(
				sp, &sp,
				__abbr_mon, countof(__abbr_mon));
			break;
		case 'B':
			d.m = strtoarri(
				sp, &sp,
				__long_mon, countof(__long_mon));
			break;
		case 'y':
			d.y = strtoui_lim(sp, &sp, 0, 99);
			if (UNLIKELY(d.y == -1U)) {
				;
			} else if ((d.y += 2000) > 2068) {
				d.y -= 100;
			}
			break;
		case 'q':
			/* bizda date and we take the arg from sp */
			switch (*sp++) {
			case '<':
				/* it's a bizda/YMDU date */
				d.flags |= BIZDA_BEFORE << 1;
			case '>':
				/* it's a bizda/YMDU date */
				d.flags |= STRPD_BIZDA_BIT;
				if (strtoarri(
					    sp, &sp,
					    bizda_ult,
					    countof(bizda_ult)) < -1U ||
				    (d.ref = strtoui_lim(
					     sp, &sp, 0, 23)) < -1U) {
					/* yay, it did work */
					break;
				}
				/*@fallthrough@*/
			default:
				sp = str;
				goto out;
			}
			break;

		case '_':
			switch (*++fp) {
				const char *pos;
			case 'b':
				if ((pos = strchr(__abab_mon, *sp++))) {
					d.m = pos - __abab_mon;
				}
				break;
			case 'a':
				if ((pos = strchr(__abab_wday, *sp++))) {
					/* ymcw mode! */
					d.w = pos - __abab_wday;
				}
				break;
			case 'd': {
				const char *fp_sav = fp++;

				/* business days */
				d.flags |= STRPD_BIZDA_BIT;
				if ((d.b = strtoui_lim(
					     sp, &sp, 0, 23)) == -1U) {
					sp = str;
					goto out;
				}
				/* bizda handling, reference could be in fp */
				switch (*fp++) {
				case '<':
					/* it's a bizda/YMDU date */
					d.flags |= BIZDA_BEFORE << 1;
				case '>':
					/* it's a bizda/YMDU date */
					if (strtoarri(
						    fp, &fp,
						    bizda_ult,
						    countof(bizda_ult)) < -1U ||
					    (d.ref = strtoui_lim(
						     fp, &fp, 0, 23)) < -1U) {
						/* worked, yippie, we have to
						 * reset fp though, as it will
						 * be advanced in the outer
						 * loop */
						fp--;
						break;
					}
					/*@fallthrough@*/
				default:
					fp = fp_sav;
					break;
				}
				break;
			}
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
		case 'W':
			/* cannot be used at the moment */
			strtoui_lim(sp, &sp, 1, 366);
			break;
		case 'j':
			/* cannot be used at the moment */
			strtoui_lim(sp, &sp, 0, 53);
			break;
		case 'O':
			/* roman numerals modifier */
			switch (*++fp) {
			case 'Y':
				d.y = romstrtoui_lim(
					sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR);
				break;
			case 'y':
				d.y = romstrtoui_lim(sp, &sp, 0, 99);
				if (UNLIKELY(d.y == -1U)) {
					;
				} else if ((d.y += 2000) > 2068) {
					d.y -= 100;
				}
				break;
			case 'm':
				d.m = romstrtoui_lim(sp, &sp, 0, 12);
				break;
			case 'd':
				d.d = romstrtoui_lim(sp, &sp, 0, 31);
				break;
			case 'c':
				d.c = romstrtoui_lim(sp, &sp, 0, 5);
				break;
			default:
				sp = str;
				goto out;
			}
			break;
		}
	}
	/* set the end pointer */
	res = __guess_dtyp(d);
out:
	if (ep) {
		*ep = (char*)sp;
	}
	return res;
}

DEFUN size_t
dt_strfd(char *restrict buf, size_t bsz, const char *fmt, struct dt_d_s that)
{
	size_t res = 0;
	struct strpd_s d = {0};

	if (UNLIKELY(buf == NULL || bsz == 0)) {
		goto out;
	}

	switch (that.typ) {
	case DT_YMD:
		d.y = that.ymd.y;
		d.m = that.ymd.m;
		d.d = that.ymd.d;
		if (fmt == NULL) {
			fmt = ymd_dflt;
		}
		break;
	case DT_YMCW:
		d.y = that.ymcw.y;
		d.m = that.ymcw.m;
		if (fmt == NULL) {
			fmt = ymcw_dflt;
		}
		break;
	case DT_DAISY: {
		dt_ymd_t tmp = __daisy_to_ymd(that.daisy);
		d.y = tmp.y;
		d.m = tmp.m;
		d.d = tmp.d;
		if (fmt == NULL) {
			/* subject to change */
			fmt = "%F";
		}
		break;
	}
	case DT_BIZDA:
		d.y = that.bizda.y;
		d.m = that.bizda.m;
		d.b = that.bizda.bd;
		d.ref = that.bizda.x;
		d.flags = (that.bizda.ba << 1) | STRPD_BIZDA_BIT;
		if (fmt == NULL) {
			fmt = bizda_dflt;
		}
		break;
	default:
	case DT_UNK:
		goto out;
	}
	/* translate high-level format names */
	__trans_fmt(&fmt);

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
		case 'F':
			shaught = 1;
		case 'Y':
			res += ui32tostr(buf + res, bsz - res, d.y, 4);
			if (UNLIKELY(shaught == 0)) {
				break;
			}
			buf[res++] = '-';
		case 'm':
			res += ui32tostr(buf + res, bsz - res, d.m, 2);
			if (UNLIKELY(shaught == 0)) {
				break;
			}
			buf[res++] = '-';
		case 'd':
			/* ymd mode check? */
			d.d = d.d ?: dt_get_mday(that);
			res += ui32tostr(buf + res, bsz - res, d.d, 2);
			break;
		case 'w':
			/* ymcw mode check */
			d.d = dt_get_wday(that);
			res += ui32tostr(buf + res, bsz - res, d.d, 2);
			break;
		case 'c':
			/* ymcw mode check? */
			d.c = dt_get_count(that);
			res += ui32tostr(buf + res, bsz - res, d.c, 2);
			break;
		case 'a':
			/* get the weekday in ymd mode!! */
			res += arritostr(
				buf + res, bsz - res,
				dt_get_wday(that),
				__abbr_wday, countof(__abbr_wday));
			break;
		case 'A':
			/* get the weekday in ymd mode!! */
			res += arritostr(
				buf + res, bsz - res,
				dt_get_wday(that),
				__long_wday, countof(__long_wday));
			break;
		case 'b':
		case 'h':
			res += arritostr(
				buf + res, bsz - res, d.m,
				__abbr_mon, countof(__abbr_mon));
			break;
		case 'B':
			res += arritostr(
				buf + res, bsz - res, d.m,
				__long_mon, countof(__long_mon));
			break;
		case 'y':
			res += ui32tostr(buf + res, bsz - res, d.y, 2);
			break;
		case 'q':
			/* bizda mode check? */
			if (((d.flags >> 1) & 1) == BIZDA_AFTER) {
				buf[res++] = '>';
			} else {
				buf[res++] = '<';
			}
			res += ui32tostr(buf + res, bsz - res, d.ref, 2);
			break;
		case '_':
			/* secret mode */
			switch (*++fp) {
			case 'b':
				/* super abbrev'd month */
				if (d.m < countof(__abab_mon)) {
					buf[res++] = __abab_mon[d.m];
				}
				break;
			case 'a': {
				dt_dow_t wd = dt_get_wday(that);
				/* super abbrev'd wday */
				if (wd < countof(__abab_wday)) {
					buf[res++] = __abab_wday[wd];
				}
				break;
			}
			case 'd':
				/* get business days */
				if ((d.b = d.b ?: dt_get_bday(that)) >= 0) {
					res += ui32tostr(
						buf + res, bsz - res, d.b, 2);
				} else {
					buf[res++] = '0';
					buf[res++] = '0';
				}
				break;
			}
			break;
		case 't':
			/* literal tab */
			buf[res++] = '\t';
			break;
		case 'n':
			/* literal \n */
			buf[res++] = '\n';
			break;
		case 'j':
			if (that.typ == DT_YMD) {
				int yd = __ymd_get_yday(that.ymd);
				res += ui32tostr(buf + res, bsz - res, yd, 3);
			}
			break;
		case 'W':
			if (that.typ == DT_YMCW) {
				int yd = __ymcw_get_yday(that.ymcw);
				res += ui32tostr(buf + res, bsz - res, yd, 2);
			}
			break;
		case 'O':
			/* o modifier for roman dates */
			res += __strfd_O(buf + res, bsz - res, *++fp, that);
			break;
		}
	}
out:
	if (res < bsz) {
		buf[res] = '\0';
	}
	return res;
}

DEFUN struct dt_dur_s
dt_strpdur(const char *str)
{
/* at the moment we allow only one format */
	struct dt_dur_s res = {DT_DUR_UNK};
	char *sp = ((union {char *p; const char *c;}){.c = str}).p;
	int tmp;
	struct strpd_s d = {0};

	if (str == NULL) {
		goto out;
	}
	/* read the year */
	do {
		tmp = strtol(sp, &sp, 10);
		switch (*sp++) {
		case '\0':
			/* must have been day then */
			d.d = tmp;
			goto assess;
		case 'd':
		case 'D':
			d.d = tmp;
			break;
		case 'y':
		case 'Y':
			d.y = tmp;
			break;
		case 'm':
		case 'M':
			d.m = tmp;
			break;
		case 'w':
		case 'W':
			d.w = tmp;
			break;
		default:
			goto out;
		}
	} while (*sp);
assess:
	if (LIKELY((d.m && d.d) ||
		   (d.y == 0 && d.m == 0 && d.w == 0) ||
		   (d.y == 0 && d.w == 0 && d.d == 0))) {
		res.typ = DT_DUR_MD;
		res.md = (dt_mddur_t){
			.m = d.m,
			.d = d.d,
		};
	} else if (d.w) {
		res.typ = DT_DUR_WD;
		res.wd = (dt_wddur_t){
			.w = d.w,
			.d = d.d,
		};
	} else if (d.y) {
		res.typ = DT_DUR_YM;
		res.ym = (dt_ymdur_t){
			.y = d.y,
			.m = d.m,
		};
	}
out:
	return res;
}

DEFUN size_t
dt_strfdur(char *restrict buf, size_t bsz, struct dt_dur_s that)
{
/* at the moment we allow only one format */
	size_t res = 0;

	if (UNLIKELY(buf == NULL || bsz == 0)) {
		goto out;
	}

	switch (that.typ) {
	case DT_DUR_MD:
		/* auto-newline */
		if ((that.md.m >= 0 && that.md.d >= 0) ||
		    (that.md.m > 0 && that.md.d < 0)) {
			res = snprintf(
				buf, bsz, "%dm%dd\n", that.md.m, that.md.d);
		} else if (that.md.m < 0 && that.md.d > 0) {
			res = snprintf(
				buf, bsz, "%dm+%dd\n", that.md.m, that.md.d);
		} else if (that.md.m < 0 && that.md.d < 0) {
			res = snprintf(
				buf, bsz, "%dm%dd\n", that.md.m, -that.md.d);
		}
		break;
	case DT_DUR_WD:
		/* auto-newline */
		if ((that.wd.w >= 0 && that.wd.d >= 0) ||
		    (that.wd.w > 0 && that.wd.d < 0)) {
			res = snprintf(
				buf, bsz, "%dw%dd\n", that.wd.w, that.wd.d);
		} else if (that.wd.w < 0 && that.wd.d > 0) {
			res = snprintf(
				buf, bsz, "%dw+%dd\n", that.wd.w, that.wd.d);
		} else if (that.wd.w < 0 && that.wd.d < 0) {
			res = snprintf(
				buf, bsz, "%dw%dd\n", that.wd.w, -that.wd.d);
		}
		break;
	case DT_DUR_YM:
		/* auto-newline */
		if ((that.ym.y >= 0 && that.ym.m >= 0) ||
		    (that.ym.y > 0 && that.ym.m < 0)) {
			res = snprintf(
				buf, bsz, "%dy%dm\n", that.ym.y, that.ym.m);
		} else if (that.ym.y < 0 && that.ym.m > 0) {
			res = snprintf(
				buf, bsz, "%dy+%dm\n", that.ym.y, that.ym.m);
		} else if (that.ym.y < 0 && that.ym.m < 0) {
			res = snprintf(
				buf, bsz, "%dy%dm\n", that.ym.m, -that.ym.m);
		}
		break;
	case DT_DUR_UNK:
	default:
		buf[0] = '\0';
		break;
	}
out:
	return res;
}


/* date getters, platform dependent */
DEFUN struct dt_d_s
dt_date(dt_dtyp_t outtyp)
{
	struct dt_d_s res;
	time_t t = time(NULL);

	switch ((res.typ = outtyp)) {
	case DT_YMD:
	case DT_YMCW: {
		struct tm tm;
		ffff_gmtime(&tm, t);
		switch (res.typ) {
		case DT_YMD:
			res.ymd.y = tm.tm_year;
			res.ymd.m = tm.tm_mon;
			res.ymd.d = tm.tm_mday;
			break;
		case DT_YMCW: {
			dt_ymd_t tmp = {
				.y = tm.tm_year,
				.m = tm.tm_mon,
				.d = tm.tm_mday,
			};
			res.ymcw.y = tm.tm_year;
			res.ymcw.m = tm.tm_mon;
			res.ymcw.c = __ymd_get_count(tmp);
			res.ymcw.w = tm.tm_wday;
			break;
		}
		}
		break;
	}
	case DT_DAISY:
		/* time_t's base is 1970-01-01, which is daisy 19359 */
		res.daisy = t / 86400 + 19359;
		break;
	default:
	case DT_UNK:
		res.u = 0;
	}
	return res;
}

DEFUN struct dt_d_s
dt_conv(dt_dtyp_t tgttyp, struct dt_d_s d)
{
	struct dt_d_s res;

	switch ((res.typ = tgttyp)) {
	case DT_YMD:
		res.ymd = dt_conv_to_ymd(d);
		break;
	case DT_YMCW:
		res.ymcw = dt_conv_to_ymcw(d);
		break;
	case DT_DAISY:
		res.daisy = dt_conv_to_daisy(d);
		break;
	case DT_BIZDA:
		break;
	case DT_UNK:
	default:
		res.typ = DT_UNK;
		break;
	}
	return res;
}

DEFUN struct dt_d_s
dt_add(struct dt_d_s d, struct dt_dur_s dur)
{
	switch (d.typ) {
	case DT_DAISY:
		d.daisy = __daisy_add(d.daisy, dur);
		break;

	case DT_YMD:
		d.ymd = __ymd_add(d.ymd, dur);
		break;

	case DT_YMCW:
		d.ymcw = __ymcw_add(d.ymcw, dur);
		break;

	case DT_UNK:
	default:
		d.typ = DT_UNK;
		d.u = 0;
		break;
	}
	return d;
}

DEFUN struct dt_dur_s
dt_diff(struct dt_d_s d1, struct dt_d_s d2)
{
	struct dt_dur_s res = {.typ = DT_DUR_UNK};

	switch (d1.typ) {
	case DT_DAISY: {
		dt_daisy_t tmp = dt_conv_to_daisy(d2);
		res = __daisy_diff(d1.daisy, tmp);
		break;
	}
	case DT_YMD: {
		dt_ymd_t tmp = dt_conv_to_ymd(d2);
		res = __ymd_diff(d1.ymd, tmp);
		break;
	}
	case DT_UNK:
	default:
		res.u = 0;
		break;
	}
	return res;
}

DEFUN struct dt_dur_s
dt_neg_dur(struct dt_dur_s dur)
{
	switch (dur.typ) {
	case DT_DUR_WD:
		dur.wd.d = -dur.wd.d;
		dur.wd.w = -dur.wd.w;
		break;
	case DT_DUR_MD:
		dur.md.d = -dur.md.d;
		dur.md.m = -dur.md.m;
		break;
	case DT_DUR_YM:
		dur.ym.m = -dur.ym.m;
		dur.ym.y = -dur.ym.y;
		break;
	case DT_DUR_UNK:
	default:
		dur.u = 0;
		break;
	}
	return dur;
}

#endif	/* INCLUDED_date_core_c_ */
/* date-core.c ends here */
