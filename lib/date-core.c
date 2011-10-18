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
#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

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
	union {
		unsigned int ref;
		signed int q;
	};
#define STRPD_BIZDA_BIT	(1U)
	/* general flags */
	unsigned int flags;
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

static const __jan01_wday_block_t __jan01_wday[] = {
#define __JAN01_WDAY_BEG	(1920)
	{
		/* 1920 - 1929 */
		R, A, S, M, T, R, F, A, S, T, 0,
	}, {
		/* 1930 - 1939 */
		W, R, F, S, M, T, W, F, A, S, 0,
	}, {
		/* 1940 - 1949 */
		M, W, R, F, A, M, T, W, R, A, 0,
	}, {
		/* 1950 - 1959 */
		S, M, T, R, F, A, S, T, W, R, 0,
	}, {
		/* 1960 - 1969 */
		F, S, M, T, W, F, A, S, M, W, 0,
	}, {
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
	}, {
		/* 2030 - 2039 */
		T, W, R, A, S, M, T, R, F, A, 0,
	}, {
		/* 2040 - 2049 */
		S, T, W, R, F, S, M, T, W, F, 0,
	}, {
		/* 2050 - 2059 */
		A, S, M, W, R, F, A, M, T, W, 0,
	}
	/* 2060 - 2069 is 1920 - 1929 */
#define __JAN01_WDAY_END	(2059)
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
	days = (int)(t / SECS_PER_DAY);
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

	if (UNLIKELY(year < __JAN01_WDAY_BEG)) {
		/* use the 140y period property */
		while ((year += 140) < __JAN01_WDAY_BEG);
	} else if (year > __JAN01_WDAY_END) {
		/* use the 140y period property */
		while ((year -= 140) > __JAN01_WDAY_END);
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
__get_bdays(unsigned int y, unsigned int m)
{
/* the 28th exists in every month, and it's exactly 20 bdays
 * away from the first, oh and it's -1 mod 7
 * then to get the number of bdays remaining in the month
 * from the number of days remaining in the month R
 * we use a multiplication table, downwards the weekday of the
 * 28th, rightwards the days in excess of the 28th
 * Miracleday is only there to make the left hand side of the
 * multiplication 3 bits wide:
 * 
 * r->  0  1  2  3
 * Sun  0  1  2  3
 * Mon  0  1  2  3
 * Tue  0  1  2  3
 * Wed  0  1  2  2
 * Thu  0  1  1  1
 * Fri  0  0  0  1
 * Sat  0  0  1  2
 * Mir  0  0  0  0 
 */
	dt_dow_t m01wd = __get_m01_wday(y, m);
	dt_dow_t m28wd = (dt_dow_t)((m01wd - DT_MONDAY/*1*/) % 7U);
	unsigned int md = __get_mdays(y, m);
	unsigned int rd = (unsigned int)(md - 28U);

	if (LIKELY(rd > 0)) {
		switch (m28wd) {
		case DT_SUNDAY:
		case DT_MONDAY:
		case DT_TUESDAY:
			return 20 + rd;
		case DT_WEDNESDAY:
			return 20 + rd - (rd == 3);
		case DT_THURSDAY:
			return 21;
		case DT_FRIDAY:
			return 20 + (rd == 3);
		case DT_SATURDAY:
			return 20 + rd - 1;
		case DT_MIRACLEDAY:
		default:
			abort();
		}
	}
	return 20U;
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
/* get N where N is the N-th occurrence of wday in the month of that year */
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
	case 12:
		break;
	default:
		return 0U;
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
#if defined __C1X
	wd01 = __ymd_get_yday((dt_ymd_t){.y = that.y, .m = that.m, .d = 01});
#else
	{
		dt_ymd_t tmp;
		tmp.y = that.y;
		tmp.m = that.m;
		tmp.d = 01;
		wd01 = __ymd_get_yday(tmp);
	}
#endif
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
	case DT_MONDAY:
	case DT_TUESDAY:
	case DT_WEDNESDAY:
	case DT_THURSDAY:
	case DT_FRIDAY:
	case DT_MIRACLEDAY:
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
#if defined __C1X
	wd01 = __ymd_get_wday((dt_ymd_t){.y = that.y, .m = that.m, .d = 01});
#else
	{
		dt_ymd_t tmp;
		tmp.y = that.y;
		tmp.m = that.m;
		tmp.d = 01;
		wd01 = __ymd_get_wday(tmp);
	}
#endif
	res = (signed int)(that.w - wd01) + 5 * (that.c) + 1;
	return res;
}

static unsigned int
__bizda_get_mday(dt_bizda_t that)
{
	dt_dow_t wd01;
	unsigned int res;

	/* find first of the month first */
#if defined __C1X
	wd01 = __ymd_get_wday((dt_ymd_t){.y = that.y, .m = that.m, .d = 01});
#else
	{
		dt_ymd_t tmp;
		tmp.y = that.y;
		tmp.m = that.m;
		tmp.d = 01;
		wd01 = __ymd_get_wday(tmp);
	}
#endif
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
#if defined __C1X
	wd01 = __ymd_get_wday((dt_ymd_t){.y = that.y, .m = that.m, .d = 01});
#else
	{
		dt_ymd_t tmp;
		tmp.y = that.y;
		tmp.m = that.m;
		tmp.d = 01;
		wd01 = __ymd_get_wday(tmp);
	}
#endif
	b = that.bd;
	magic = (b - 1 + (wd01 ?: 6) - 1);
	/* now just add up bdays */
	return (dt_dow_t)((magic % 5) + DT_MONDAY);
}

static unsigned int
__bizda_get_count(dt_bizda_t that)
{
/* get N where N is the N-th occurrence of wday in the month of that year */
	if (UNLIKELY(that.bd + 5U > __get_bdays(that.y, that.m))) {
		return 5;
	}
	return (that.bd - 1U) / 5U + 1U;
}

static unsigned int
__bizda_get_yday(dt_bizda_t that)
{
/* return the N-th business day in Y,
 * the meaning of ultimo will be stretched to Y-ultimo, either last year's
 * or this year's, other reference points are not yet supported
 *
 * we use the following table (days beyond 20 bdays per month (across)):
 * Mon  3  0  2   1  3  1   2  3  0   3  2  1 
 * Mon  3  1  1   rest like Tue
 * Tue  3  0  1   2  3  0   3  2  1   3  1  2 
 * Tue  3  1  1   rest like Wed
 * Wed  3  0  1   2  2  1   3  1  2   3  0  3 
 * Wed  3  0  2   rest like Thu
 * Thu  2  0  2   2  1  2   3  1  2   2  1  3 
 * Thu  2  0  3   rest like Fri
 * Fri  1  0  3   2  1  2   2  2  2   1  2  3 
 * Fri  1  1  3   rest like Sat
 * Sat  1  0  3   1  2  2   1  3  2   1  2  2 
 * Sat  1  1  3   rest like Sun
 * Sun  2  0  3   0  3  2   1  3  1   2  2  1 
 * Sun  2  1  2   rest like Mon
 * */
	struct __bdays_by_wday_s {
		unsigned int jan:2;
		unsigned int feb:2;
		unsigned int mar:2;
		unsigned int apr:2;
		unsigned int may:2;
		unsigned int jun:2;
		unsigned int jul:2;
		unsigned int aug:2;
		unsigned int sep:2;
		unsigned int oct:2;
		unsigned int nov:2;
		unsigned int dec:2;

		unsigned int feb_leap:2;
		unsigned int mar_leap:2;

		/* 4 bits left */
		unsigned int flags:4;
	};
	static struct __bdays_by_wday_s tbl[7] = {
		{
			/* DT_SUNDAY */
			2, 0, 3,  0, 3, 2,  1, 3, 1,  2, 2, 1,  1, 2, 0
		}, {
			/* DT_MONDAY */
			3, 0, 2,  1, 3, 1,  2, 3, 0,  3, 2, 1,  1, 1, 0
		}, {
			/* DT_TUESDAY */
			3, 0, 1,  2, 3, 0,  3, 2, 1,  3, 1, 2,  1, 1, 0
		}, {
			/* DT_WEDNESDAY */
			3, 0, 1,  2, 2, 1,  3, 1, 2,  3, 0, 3,  0, 2, 0
		}, {
			/* DT_THURSDAY */
			2, 0, 2,  2, 1, 2,  3, 1, 2,  2, 1, 3,  0, 3, 0
		}, {
			/* DT_FRIDAY */
			1, 0, 3,  2, 1, 2,  2, 2, 2,  1, 2, 3,  1, 3, 0
		}, {
			/* DT_SATURDAY */
			1, 0, 3,  1, 2, 2,  1, 3, 2,  1, 2, 2,  1, 3, 0
		},
	};
	dt_dow_t j01wd;
	unsigned int y = that.y;
	unsigned int m = that.m;
	unsigned int accum = 0;

	if (UNLIKELY(that.x != BIZDA_ULTIMO)) {
		return 0;
	}
	j01wd = __get_jan01_wday(that.y);

	if (LIKELY(!__leapp(y))) {
		union {
			uint32_t u;
			struct __bdays_by_wday_s s;
		} page = {
			.s = tbl[j01wd],
		};
		for (unsigned int i = 0; i < m - 1; i++) {
			accum += page.u & /*lower two bits*/3;
			page.u >>= 2;
		}
	} else if (m > 1) {
		union {
			uint32_t u;
			struct __bdays_by_wday_s s;
		} page = {
			.s = tbl[j01wd],
		};
		accum += page.u & /*lowe two bits*/3;
		if (m > 2) {
			accum += page.s.feb_leap;
		}
		if (m > 3) {
			accum += page.s.mar_leap;
		}
		/* load a different page now, shift to the right month */
		page.s = tbl[(j01wd + DT_MONDAY) % 7U];
		page.u >>= 6;
		for (unsigned int i = 4; i < m; i++) {
			accum += page.u & /*lower two bits*/3;
			page.u >>= 2;
		}
	}
	return 20 * (m - 1) + accum + that.bd;
}

static dt_ymcw_t
__ymd_to_ymcw(dt_ymd_t d)
{
	unsigned int c = __ymd_get_count(d);
	unsigned int w = __ymd_get_wday(d);
#if defined __C1X
	return (dt_ymcw_t){.y = d.y, .m = d.m, .c = c, .w = w};
#else
	dt_ymcw_t res;
	res.y = d.y;
	res.m = d.m;
	res.c = c;
	res.w = w;
	return res;
#endif
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

static unsigned int
__daisy_get_yday(dt_daisy_t d)
{
	dt_daisy_t j00;
	unsigned int y;

	if (UNLIKELY(d == 0)) {
		return 0U;
	}
	y = __daisy_get_year(d);
	j00 = __jan00_daisy(y);
	return d - j00;
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
#if defined __C1X
	return (dt_ymd_t){.y = y, .m = m, .d = d};
#else
	{
		dt_ymd_t res;
		res.y = y;
		res.m = m;
		res.d = d;
		return res;
	}
#endif
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
#if defined __C1X
	return (dt_ymcw_t){.y = tmp.y, .m = tmp.m, .c = c, .w = w};
#else
	{
		dt_ymcw_t res;
		res.y = tmp.y;
		res.m = tmp.m;
		res.c = c;
		res.w = w;
		return res;
	}
#endif
}

static dt_ymd_t
__ymcw_to_ymd(dt_ymcw_t d)
{
	unsigned int md = __ymcw_get_mday(d);
#if defined __C1X
	return (dt_ymd_t){.y = d.y, .m = d.m, .d = md};
#else
	dt_ymd_t res;
	res.y = d.y;
	res.m = d.m;
	res.d = md;
	return res;
#endif
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
		return __daisy_to_ymd(that.daisy).y;
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
		return __daisy_to_ymd(that.daisy).m;
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
	case DT_DAISY:
		return __daisy_to_ymd(that.daisy).d;
	case DT_BIZDA:
		return __bizda_get_mday(that.bizda);;
	case DT_YMD:
		/* to shut gcc up */
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
	case DT_DAISY:
		return __ymd_get_count(__daisy_to_ymd(that.daisy));
	case DT_BIZDA:
		return __bizda_get_count(that.bizda);
	case DT_YMCW:
		/* to shut gcc up */
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
	case DT_DAISY:
		return __daisy_get_yday(that.daisy);
	case DT_BIZDA:
		return __bizda_get_yday(that.bizda);
	default:
	case DT_UNK:
		return 0;
	}
}

DEFUN int
dt_get_bday(struct dt_d_s that)
{
/* get N where N is the N-th business day after ultimo */
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

DEFUN int
dt_get_bday_q(struct dt_d_s that, unsigned int ba, unsigned int ref)
{
/* get N where N is the N-th business day Before/After REF */
	switch (that.typ) {
	case DT_BIZDA:
		if (that.bizda.x == ref && that.bizda.ba == ba) {
			return that.bizda.bd;
		}
		return 0/*__bizda_to_bizda(that.bizda, ba, ref)*/;
	case DT_DAISY:
		that.ymd = __daisy_to_ymd(that.daisy);
	case DT_YMD:
		return __ymd_get_bday(that.ymd, ba, ref);
	case DT_YMCW:
		return __ymcw_get_bday(that.ymcw, ba, ref);
	default:
	case DT_UNK:
		return 0;
	}
}

DEFUN int
dt_get_quarter(struct dt_d_s that)
{
	int m;

	switch (that.typ) {
	case DT_YMD:
		m = that.ymd.m;
		break;
	case DT_YMCW:
		m = that.ymcw.m;
		break;
	case DT_BIZDA:
		m = that.bizda.m;
		break;
	default:
	case DT_UNK:
		return 0;
	}
	return (m - 1) / 3 + 1;
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

static dt_bizda_t
dt_conv_to_bizda(struct dt_d_s that)
{
	switch (that.typ) {
	case DT_BIZDA:
		return that.bizda;
	case DT_YMD:
		break;
	case DT_YMCW:
		break;
	case DT_DAISY:
		break;
	case DT_UNK:
	default:
		break;
	}
	return (dt_bizda_t){.u = 0};
}


/* arithmetic */
static int
__get_d_equiv(dt_dow_t dow, int b)
{
	int res = 0;

	switch (dow) {
	case DT_MONDAY:
	case DT_TUESDAY:
	case DT_WEDNESDAY:
	case DT_THURSDAY:
	case DT_FRIDAY:
		res += 7 * (b / 5);
		b = b % 5;
		break;
	case DT_SATURDAY:
		res++;
	case DT_SUNDAY:
		res++;
		b--;
		res += 7 * (b / 5);
		if ((b = b % 5) < 0) {
			/* act as if we're on the monday after */
			res++;
		}
		dow = DT_MONDAY;
		break;
	case DT_MIRACLEDAY:
	default:
		break;
	}

	/* fixup b */
	if (b < 0) {
		res -= 7;
		b += 5;
	}
	/* b >= 0 && b < 5 */
	switch (dow) {
	case DT_SUNDAY:
	case DT_MONDAY:
	case DT_TUESDAY:
	case DT_WEDNESDAY:
	case DT_THURSDAY:
	case DT_FRIDAY:
		if ((int)dow + b <= (int)DT_FRIDAY) {
			res += b;
		} else {
			res += b + 2;
		}
		break;
	case DT_SATURDAY:
		res += b + 1;
		break;
	case DT_MIRACLEDAY:
	default:
		res = 0;
	}
	return res;
}

static dt_daisy_t
__daisy_add(dt_daisy_t d, struct dt_dur_s dur)
{
	switch (dur.typ) {
	case DT_DUR_WD:
		d += dur.wd.w * 7 + dur.wd.d;
		break;
	case DT_DUR_QMB:
		if (dur.qmb.q == 0 && dur.qmb.m == 0) {
			/* daisies can't handle business months and quarters */
			d += __get_d_equiv(__daisy_get_wday(d), dur.qmb.b);
		}
		break;
	case DT_DUR_MD:
		if (dur.md.m == 0) {
			/* daisies can handle days but not months */
			d += dur.md.d;
		}
		break;
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
/* this should be specifiable somehow */
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
	case DT_DUR_QMB:
		/* init tmp */
		switch (dur.typ) {
		case DT_DUR_YM:
			tmp = dur.ym.y * 12 + dur.ym.m;
			break;
		case DT_DUR_MD:
			tmp = dur.md.m;
			break;
		case DT_DUR_QMB:
			tmp = dur.qmb.q * 3 + dur.qmb.m;
			break;
		default:
			tmp = 0;
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
		} else if (dur.typ == DT_DUR_QMB) {
			/* fallthrough from above */
			/* construct a tentative result */
			dt_dow_t tent;
			d.y = tgty;
			d.m = tgtm;
			d.d = tgtd;
			tent = __ymd_get_wday(d);
			tgtd += __get_d_equiv(tent, dur.qmb.b);
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
#if defined __C1X
	struct dt_d_s res = {.u = 0};
#else
	struct dt_d_s res;
#endif
	bool bizdap;

#if !defined __C1X
	res.u = 0;
#endif
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
		res.ymd.y = d.y;
		res.ymd.m = d.m;
		res.ymd.d = d.d;
	} else if (d.y && d.c && !bizdap) {
		/* its legit for d.w to be naught */
		res.typ = DT_YMCW;
		res.ymcw.y = d.y;
		res.ymcw.m = d.m;
		res.ymcw.c = d.c;
		res.ymcw.w = d.w;
	} else if (d.y && bizdap) {
		/* d.c can be legit'ly naught */
		res.typ = DT_BIZDA;
		res.bizda.y = d.y;
		res.bizda.m = d.m;
		res.bizda.bda = d.b;
		res.bizda.x = d.ref;
	} else {
		/* anything else is bollocks for now */
		res.typ = DT_UNK;
		res.u = 0;
	}
	return res;
}

static const char ymd_dflt[] = "%F";
static const char ymcw_dflt[] = "%Y-%m-%c-%w";
static const char bizda_dflt[] = "%Y-%m-%_d%q";
static const char *bizda_ult[] = {"ultimo", "ult"};

static void
__trans_dfmt(const char **fmt)
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
#if defined __C1X
	struct dt_d_s res = {.typ = DT_UNK, .u = 0};
#else
	struct dt_d_s res;
#endif
	struct strpd_s d = {0};
	const char *sp;

#if !defined __C1X
	res.typ = DT_UNK;
	res.u = 0;
#endif

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
	case DT_UNK:
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

/* belong in strops? */
static int
__ordinalp(unsigned int d, const char *str, char **ep)
{
#define __tolower(c)	(c | 0x20)
#define ILEA(a, b)	(((a) << 8) | (b))
	const char *p = str;
	int res = 0;
	int p2 = ILEA(__tolower(p[0]), __tolower(p[1]));

	if (LIKELY(p2 == ILEA('t', 'h'))) {
		/* we accept 1th 2th 3th */
		p += 2;
		goto yep;
	}
	/* check the number */
	switch ((d % 10)) {
	case 1:
		if (p2 == ILEA('s', 't') && (d % 100 != 11)) {
			p += 2;
		} else {
			res = -1;
		}
		break;
	case 2:
		if (p2 == ILEA('n', 'd') && (d % 100 != 12)) {
			p += 2;
		} else {
			res = -1;
		}
		break;
	case 3:
		if (p2 == ILEA('r', 'd') && (d % 100 != 13)) {
			p += 2;
		} else {
			res = -1;
		}
		break;
	default:
		res = -1;
		break;
	}
yep:
	*ep = (char*)p;
	return res;
#undef ILEA
#undef __tolower
}

static size_t
__ordtostr(char *buf, size_t bsz, unsigned int d)
{
	size_t res = 2;

	if (bsz < 2) {
		return 0;
	}
	switch ((d % 10)) {
	default:
	teens:
		buf[0] = 't';
		buf[1] = 'h';
		break;
	case 1:
		if (UNLIKELY((d % 100) == 11)) {
			goto teens;
		}
		buf[0] = 's';
		buf[1] = 't';
		break;
	case 2:
		if (UNLIKELY((d % 100) == 12)) {
			goto teens;
		}
		buf[0] = 'n';
		buf[1] = 'd';
		break;
	case 3:
		if (UNLIKELY((d % 100) == 13)) {
			goto teens;
		}
		buf[0] = 'r';
		buf[1] = 'd';
		break;
	}
	return res;
}


/* parser implementations */
DEFUN struct dt_d_s
dt_strpd(const char *str, const char *fmt, char **ep)
{
#if defined __C1X
	struct dt_d_s res = {.typ = DT_UNK, .u = 0};
#else
	struct dt_d_s res;
#endif
	struct strpd_s d = {0};
	const char *sp = str;

#if !defined __C1X
	res.typ = DT_UNK;
	res.u = 0;
#endif

	if (UNLIKELY(fmt == NULL)) {
		return __strpd_std(str, ep);
	}
	/* translate high-level format names */
	__trans_dfmt(&fmt);

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
			/* check for ordinals */
			if (fp[1] == 't' && fp[2] == 'h' &&
			    __ordinalp(d.d, sp, (char**)&sp) == 0) {
				fp += 2;
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
		case '>':
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
		case 'C':
			/* cannot be used at the moment */
			strtoui_lim(sp, &sp, 1, 366);
			break;
		case 'j':
			/* cannot be used at the moment */
			strtoui_lim(sp, &sp, 0, 53);
			break;
		case 'Q':
			if (*sp++ != 'Q') {
				sp = str;
				goto out;
			}
		case 'q':
			if (d.m == 0) {
				d.m = strtoui_lim(sp, &sp, 1, 4) * 3 - 2;
			}
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
	__trans_dfmt(&fmt);

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
			/* check for ordinals */
			if (fp[1] == 't' && fp[2] == 'h') {
				res += __ordtostr(buf + res, bsz - res, d.d);
				fp += 2;
			}
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
		case 'Q': {
			int q = dt_get_quarter(that);

			if (q <= 4) {
				if (*fp == 'q') {
					buf[res++] = '0';
				} else if (*fp == 'Q') {
					buf[res++] = 'Q';
				}
				buf[res++] = (char)(q + '0');
			}
			break;
		}
		case '>':
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
			case 'D':
				/* get business days */
				if (that.typ == DT_BIZDA) {
					unsigned int b =
						__bizda_get_yday(that.bizda);
					res += ui32tostr(
						buf + res, bsz - res, b, 3);
				} else {
					buf[res++] = '0';
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
		case 'C':
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
dt_strpdur(const char *str, char **ep)
{
/* at the moment we allow only one format */
	struct dt_dur_s res = {DT_DUR_UNK};
	const char *sp = str;
	int tmp;
	struct strpd_s d = {0};

	if (str == NULL) {
		goto out;
	}
	/* read just one component */
	tmp = strtol(sp, (char**)&sp, 10);
	switch (*sp++) {
	case '\0':
		/* must have been day then */
		d.d = tmp;
		sp--;
		break;
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
	case 'b':
	case 'B':
		d.b = tmp;
		break;
	case 'q':
	case 'Q':
		d.q = tmp;
		break;
	default:
		sp = str;
		goto out;
	}
	/* assess */
	if (d.b || d.q) {
		res.typ = DT_DUR_QMB;
		res.qmb.q = d.q;
		res.qmb.m = d.m;
		res.qmb.b = d.b;
	} else if (LIKELY((d.m && d.d) ||
		   (d.y == 0 && d.m == 0 && d.w == 0) ||
		   (d.y == 0 && d.w == 0 && d.d == 0))) {
		res.typ = DT_DUR_MD;
		res.md.m = d.m;
		res.md.d = d.d;
	} else if (d.w) {
		res.typ = DT_DUR_WD;
		res.wd.w = d.w;
		res.wd.d = d.d;
	} else if (d.y) {
		res.typ = DT_DUR_YM;
		res.ym.y = d.y;
		res.ym.m = d.m;
	}
out:
	if (ep) {
		*ep = (char*)sp;
	}
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
	case DT_DUR_QMB:
		/* auto-newline */
		if (that.qmb.q) {
			res = snprintf(
				buf, bsz, "%dq%dm%db\n",
				that.qmb.q, that.qmb.m, that.qmb.b);
		} else if (that.qmb.m) {
			res = snprintf(
				buf, bsz, "%dm%db\n",
				that.qmb.m, that.qmb.b);
		} else {
			res = snprintf(
				buf, bsz, "%db\n", that.qmb.b);
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
#if defined __C1X
			dt_ymd_t tmp = {
				.y = tm.tm_year,
				.m = tm.tm_mon,
				.d = tm.tm_mday,
			};
#else
			dt_ymd_t tmp;
			tmp.y = tm.tm_year,
			tmp.m = tm.tm_mon,
			tmp.d = tm.tm_mday,
#endif
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
		/* actually this is a parametrised date */
		res.bizda = dt_conv_to_bizda(d);
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
	case DT_DUR_QMB:
		dur.qmb.q = -dur.qmb.q;
		dur.qmb.m = -dur.qmb.m;
		dur.qmb.b = -dur.qmb.b;
		break;
	case DT_DUR_UNK:
	default:
		dur.u = 0;
		break;
	}
	return dur;
}

DEFUN int
dt_dur_neg_p(struct dt_dur_s dur)
{
	switch (dur.typ) {
	case DT_DUR_WD:
		if (dur.wd.w == 0 ||
		    dur.wd.w * 7 < dur.wd.d) {
			return dur.wd.d < 0 || dur.wd.w < 0;
		} else {
			return dur.wd.w < 0;
		}
		break;
	case DT_DUR_MD:
		if (dur.md.m == 0 ||
		    dur.md.m * 30 < dur.md.d) {
			/* second case is undefined really */
			return dur.md.d < 0 || dur.md.m < 0;
		} else {
			return dur.md.m < 0;
		}
		break;
	case DT_DUR_YM:
		if (dur.ym.y == 0 ||
		    dur.ym.y * 12 < dur.ym.m) {
			return dur.ym.m < 0 || dur.ym.y < 0;
		} else {
			return dur.ym.y < 0;
		}
		break;
	case DT_DUR_QMB:
		if ((dur.qmb.q == 0 && dur.qmb.m == 0) ||
		    (dur.qmb.q * 3 + dur.qmb.m) * 23 < dur.qmb.b) {
			return dur.qmb.b < 0 || dur.qmb.q < 0 || dur.qmb.m < 0;
		} else if (dur.qmb.q == 0 ||
			   dur.qmb.m * 23 > dur.qmb.b) {
			return dur.qmb.m < 0 || dur.qmb.q < 0;
		} else {
			return dur.qmb.q < 0;
		}
		break;
	case DT_DUR_UNK:
	default:
		break;
	}
	return 0;
}

DEFUN int
dt_cmp(struct dt_d_s d1, struct dt_d_s d2)
{
/* for the moment D1 and D2 have to be of the same type. */
	if (UNLIKELY(d1.typ != d2.typ)) {
		/* always the left one */
		return -2;
	}
	if (d1.u == d2.u) {
		return 0;
	} else if (d1.u < d2.u) {
		return -1;
	} else /*if (d1.u > d2.u)*/ {
		return 1;
	}
}

DEFUN int
dt_in_range_p(struct dt_d_s d, struct dt_d_s d1, struct dt_d_s d2)
{
	return dt_cmp(d, d1) >= 0 && dt_cmp(d, d2) <= 0;
}

#if defined __INTEL_COMPILER
# pragma warning (default:2203)
#elif defined __GNUC__
# pragma GCC diagnostic warning "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

#endif	/* INCLUDED_date_core_c_ */
/* date-core.c ends here */
