/*** date-core.h -- our universe of dates
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
#if !defined INCLUDED_date_core_c_
#define INCLUDED_date_core_c_

#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include "date-core.h"
#include "strops.h"
#include "token.h"

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
# define UNUSED(_x)	__attribute__((unused)) _x
#endif	/* !UNUSED */
#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

#if !defined assert
# define assert(x)
#endif	/* !assert */

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
	union {
		unsigned int d;
		signed int sd;
	};
	unsigned int c;
	unsigned int w;
	/* general flags */
	struct {
		unsigned int ab:1;
		unsigned int bizda:1;
		unsigned int d_dcnt_p:1;
	} flags;
	unsigned int b;
	unsigned int q;
};

struct strpdi_s {
	signed int m;
	signed int d;
	signed int w;
	signed int b;
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

#undef M
#undef T
#undef W
#undef R
#undef F
#undef A
#undef S

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

/* bizda definitions, reference dates */
static __attribute__((unused)) const char *bizda_ult[] = {"ultimo", "ult"};

static inline bool
__leapp(unsigned int y)
{
#if defined WITH_FAST_ARITH
	return y % 4 == 0;
#else  /* !WITH_FAST_ARITH */
	return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
#endif	/* WITH_FAST_ARITH */
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
	tm->tm_wday = (days + 4) % GREG_DAYS_P_WEEK;

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
		yy = 1;
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

/* arithmetics helpers */
static inline unsigned int
__uimod(signed int x, signed int m)
{
	int res = x % m;
	return res >= 0 ? res : res + m;
}

static inline unsigned int
__uidiv(signed int x, signed int m)
{
/* uidiv expects its counterpart (the mod) to be computed with __uimod */
	int res = x / m;
	return x >= 0 ? res : x % m ? res - 1 : res;
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

	if (UNLIKELY(mon < 1 && mon > GREG_MONTHS_P_YEAR)) {
		return DT_MIRACLEDAY;
	}
	cand = __get_jan01_wday(year);
	off = __mon_yday[mon] % GREG_DAYS_P_WEEK;
	/* fixup leap years */
	if (UNLIKELY(__leapp(year))) {
		off += (__mon_yday[0] >> mon) & 1;
	}
	return (dt_dow_t)((cand + off) % GREG_DAYS_P_WEEK);
}

static inline unsigned int
__get_mdays(unsigned int y, unsigned int m)
{
/* get the number of days in Y-M */
	unsigned int res;

	if (UNLIKELY(m < 1 || m > GREG_MONTHS_P_YEAR)) {
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

static inline unsigned int
__get_mcnt(unsigned int y, unsigned int m, dt_dow_t w)
{
/* get the number of weekdays W in Y-M, which is the max count
 * for a weekday W in ymcw dates in year Y and month M */
	dt_dow_t wd01 = __get_m01_wday(y, m);
	unsigned int md = __get_mdays(y, m);
	/* the maximum number of WD01s in Y-M */
	unsigned int wd01cnt = (md - 1) / GREG_DAYS_P_WEEK + 1;
	/* modulus */
	unsigned int wd01mod = (md - 1) % GREG_DAYS_P_WEEK;

	/* now the next WD01MOD days also have WD01CNT occurrences
	 * if wd01 + wd01mod exceeds the DAYS_PER_WEEK barrier wrap
	 * around by extending W to W + DAYS_PER_WEEK */
	if ((w >= wd01 && w <= wd01 + wd01mod) ||
	    (w + GREG_DAYS_P_WEEK) <= wd01 + wd01mod) {
		return wd01cnt;
	} else {
		return wd01cnt - 1;
	}
}

static unsigned int
__get_nwedays(unsigned int dur, dt_dow_t wd)
{
/* get the number of weekend days in a sequence of DUR days ending on WD
 * The minimum number of weekend days is simply 2 for every 7 days
 * to get the exact number observe the following:
 *
 * The number of remaining weekend days depends on the remaining number
 * of days and the DOW of the end point.
 *
 * here's the matrix, mod7 down, WD right:
 *    S M T W R F A
 * 0  0 0 0 0 0 0 0
 * 1  1 0 0 0 0 0 1
 * 2  2 1 0 0 0 0 1
 * 3  2 2 1 0 0 0 1
 * 4  2 2 2 1 0 0 1
 * 5  2 2 2 2 1 0 1
 * 6  2 2 2 2 2 1 1
 *
 * That means
 * (mod7 == 0) -> 0
 * (WD == SAT) -> 1
 * (mod7 - WD) > 1 -> 2
 * (mod7 - WD) > 0 -> 1
 * 
 * and that's all the magic behind the following code */
	unsigned int nss = (dur / GREG_DAYS_P_WEEK) * 2;
	unsigned int mod = (dur % GREG_DAYS_P_WEEK);

	if (mod == 0) {
		return nss;
	} else if (wd == DT_SATURDAY) {
		return nss + 1;
	} else if (((signed)mod - (int)wd) > 1) {
		return nss + 2;
	} else if (((signed)mod - (int)wd) > 0) {
		return nss + 1;
	}
	return nss;
}

static unsigned int
__get_nbdays(unsigned int dur, dt_dow_t wd)
{
/* get the number of business days in a sequence of DUR days ending on WD
 * which is simply the number of days minus the number of weekend-days */
	return dur - __get_nwedays(dur, wd);
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
	unsigned int md = __get_mdays(y, m);
	unsigned int rd = (unsigned int)(md - 28U);
	dt_dow_t m01wd;
	unsigned int m28wd;

	/* rd should not overflow */
	assert((signed int)md - 28 >= 0);

	/* wday of the 1st and 28th */
	m01wd = __get_m01_wday(y, m);
	m28wd = __uimod((signed int)m01wd - 1, GREG_DAYS_P_WEEK);
	if (LIKELY(rd > 0)) {
		switch ((dt_dow_t)m28wd) {
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

struct __md_s {
	unsigned int m;
	unsigned int d;
};

static struct __md_s
__yday_get_md(unsigned int year, unsigned int doy)
{
/* Given a year and the day of the year, return gregorian month + dom
 * we use some clever maths to invert __mon_yday[] above
 * you see __mon_yday[] divrem 16 is
 *   F   0  0  0
 *   G  31  1 15
 *   H  59  3 11
 *   J  90  5 10
 *   K 120  7  8
 *   M 151  9  7
 *   N 181 11  5
 *   Q 212 13  4
 *   U 243 15  3
 *   V 273 17  1
 *   X 304 19  0
 *   Z 334 20 14
 *
 * Now the key is to store only the remainders in a clever way.
 * To resolve the even quotient in the Z month, we simply add 2
 * to the day-of-year.
 *
 * Second, since doy < 32 -> m = 1, d = doy, we only need to store
 * the values from month G onwards.
 *
 * So in total the table we store is 4bit remainders of
 * __mon_yday[] - 30 % 16 */
#define SL(x)	((x) * 4)
	static const uint64_t rem =
		(0ULL << SL(0)) |
		(1ULL << SL(1)) |
		(13ULL << SL(2)) |
		(12ULL << SL(3)) |
		(10ULL << SL(4)) |
		(9ULL << SL(5)) |
		(7ULL << SL(6)) |
		(6ULL << SL(7)) |
		(5ULL << SL(8)) |
		(3ULL << SL(9)) |
		(2ULL << SL(10)) |
		(0ULL << SL(11));
	unsigned int m;
	unsigned int d;
	unsigned int beef;
	unsigned int cake;

	/* unrolled version */
	if (doy < 32) {
		m = 1;
		d = doy;
		goto yay;
	}

	/* get 16-adic doys */
	m = (doy - 30) / 16U;
	d = (doy - 30) % 16U;
	beef = (rem >> SL((m + 1) / 2)) & 0xff;
	cake = beef >> 4;

	/* put leap years into cake */
	if (UNLIKELY(__leapp(year))) {
		beef += m >= 2;
		cake += m >= 1;
	} else if (UNLIKELY(doy > 365)) {
		m = 0;
		d = 0;
		goto yay;
	}

	if (!m || m % 2 && d > cake) {
		d = doy - (m * 16 + 30 + cake);
		m = (m + 1) / 2 + 2;
	} else {
		if (!(m % 2)) {
			/* even m */
			d = doy - ((m - 1) * 16 + 30 + cake);
		} else {
			/* odd m and remainder <= cake */
			beef = 16 + (m > 1 ? 16 : 0) + cake - (beef & 0xf);
			d = doy - (m * 16 + 30 + cake) + beef;
		}
		m = m / 2 + 2;
	}
yay:
	return (__extension__(struct __md_s){.m = m, .d = d});
}


static unsigned int
__ymd_get_yday(dt_ymd_t that)
{
	unsigned int res;

	if (UNLIKELY(that.y == 0 ||
		     that.m == 0 || that.m > GREG_MONTHS_P_YEAR)) {
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
	unsigned int j01_wd;

	if ((yd = __ymd_get_yday(that)) > 0 &&
	    (j01_wd = __get_jan01_wday(that.y)) != DT_MIRACLEDAY) {
		return (dt_dow_t)((yd - 1 + j01_wd) % GREG_DAYS_P_WEEK);
	}
	return DT_MIRACLEDAY;
}

static unsigned int
__ymd_get_count(dt_ymd_t that)
{
/* get N where N is the N-th occurrence of wday in the month of that year */
#if 0
/* this proves to be a disaster when comparing ymcw dates */
	if (UNLIKELY(that.d + GREG_DAYS_P_WEEK > __get_mdays(that.y, that.m))) {
		return 5;
	}
#endif
	return (that.d - 1U) / GREG_DAYS_P_WEEK + 1U;
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
	unsigned int res;

	/* see what weekday the first of the month was*/
	wd01 = __get_m01_wday(that.y, that.m);

	/* first WD1 is 1, second WD1 is 8, third WD1 is 15, etc.
	 * so the first WDx with WDx > WD1 is on (WDx - WD1) + 1 */
	res = (that.w + GREG_DAYS_P_WEEK - wd01) % GREG_DAYS_P_WEEK + 1;
	res += GREG_DAYS_P_WEEK * (that.c - 1);
	/* not all months have a 5th X, so check for this */
	if (res > __get_mdays(that.y, that.m)) {
		 /* 5th = 4th in that case */
		res -= GREG_DAYS_P_WEEK;
	}
	return res;
}

static int
__ymd_get_wcnt(dt_ymd_t d, int wdays_from)
{
	int yd = __ymd_get_yday(d);
	int y01 = (int)__get_jan01_wday(d.y);
	int wk;

	/* yd of the FIRST week of the year */
	if ((wk = 8 - y01 + wdays_from) > 7) {
		wk -= 7;
	}
	/* and now express yd as 7k + n relative to jan01 */
	return (yd - wk + 7) / 7;
}

static int
__ymd_get_wcnt_iso(dt_ymd_t d)
{
/* like __ymd_get_wcnt() but for iso week conventions
 * the week with the first thursday is the first week,
 * so a year starting on S is the first week,
 * a year starting on M is the first week
 * a year starting on T ... */
	/* iso weeks always start on Mon */
	static const int_fast8_t iso[] = {2, 1, 0, -1, -2, 4, 3};
	int yd = __ymd_get_yday(d);
	unsigned int y = d.y;
	int y01 = (int)__get_jan01_wday(y);
	int wk;

	/* express yd as 7k + n relative to jan01 */
	if (UNLIKELY((wk = (yd - iso[y01] + 7) / 7) < 1)) {
		/* get last years y01
		 * which is basically y01 - (365|366 % 7) */
		if (LIKELY(!__leapp(--y))) {
			/* -= 1 */
			y01 += 6;
			yd += 365;
		} else {
			/* -= 2 */
			y01 += 5;
			yd += 366;
		}
		if (y01 >= DT_MIRACLEDAY) {
			y01 -= 7;
		}
		/* same computation now */
		wk = (yd - iso[y01] + 7) / 7;
	}
	if (UNLIKELY(wk == 53)) {
		/* check next year's y01 */
		if (LIKELY(!__leapp(y))) {
			y01 += 1;
		} else {
			/* -= 2 */
			y01 += 2;
		}
		if (!(y01 == DT_FRIDAY || y01 == DT_SATURDAY)) {
			/* 53rd week is no more */
			wk = 1;
		}
	}
	return wk;
}

static int
__ymd_get_bday(dt_ymd_t that, dt_bizda_param_t bp)
{
	dt_dow_t wdd;

	if (bp.ab != BIZDA_AFTER || bp.ref != BIZDA_ULTIMO) {
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
	/* get the number of business days between 1 and that.d */
	return __get_nbdays(that.d, wdd);
}

static int
__ymcw_get_bday(dt_ymcw_t that, dt_bizda_param_t bp)
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
	if (bp.ab != BIZDA_AFTER || bp.ref != BIZDA_ULTIMO) {
		/* no support yet */
		return -1;
	}

	/* weekday the month started with */
	wd01 = __get_m01_wday(that.y, that.m);
	res = (signed int)(that.w - wd01) + DUWW_BDAYS_P_WEEK * (that.c) + 1;
	return res;
}

static unsigned int
__bizda_get_mday(dt_bizda_t that)
{
	dt_dow_t wd01;
	unsigned int res;

	/* find first of the month first */
	wd01 = __get_m01_wday(that.y, that.m);

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

		assert(b - 1 + wd01 - 1 >= 0);
		wk = magic / DUWW_BDAYS_P_WEEK;
		nd = magic % DUWW_BDAYS_P_WEEK;
		res += wk * GREG_DAYS_P_WEEK + nd - wd01 + 1;
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
	wd01 = __get_m01_wday(that.y, that.m);
	b = that.bd;
	magic = (b - 1 + (wd01 ?: 6) - 1);
	/* now just add up bdays */
	return (dt_dow_t)((magic % DUWW_BDAYS_P_WEEK) + DT_MONDAY);
}

static unsigned int
__bizda_get_count(dt_bizda_t that)
{
/* get N where N is the N-th occurrence of wday in the month of that year */
	unsigned int bd = __get_bdays(that.y, that.m);

	if (UNLIKELY(that.bd + DUWW_BDAYS_P_WEEK > bd)) {
		return DUWW_BDAYS_P_WEEK;
	}
	return (that.bd - 1U) / DUWW_BDAYS_P_WEEK + 1U;
}

static unsigned int
__bizda_get_yday(dt_bizda_t that, dt_bizda_param_t param)
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

	if (UNLIKELY(param.ref != BIZDA_ULTIMO)) {
		return 0;
	}
	j01wd = __get_jan01_wday(that.y);

	if (LIKELY(!__leapp(y))) {
		union {
			uint32_t u;
			uint32_t lu:2;
			struct __bdays_by_wday_s s;
		} page = {
			.s = tbl[j01wd],
		};
		for (unsigned int i = 0; i < m - 1; i++) {
			accum += page.lu;
#if defined WORDS_BIGENDIAN
			page.u <<= 2;
#else  /* !WORDS_BIGENDIAN */
			page.u >>= 2;
#endif	/* WORDS_BIGENDIAN */
		}
	} else if (m > 1) {
		union {
			uint32_t u;
			uint32_t lu:2;
			struct __bdays_by_wday_s s;
		} page = {
			.s = tbl[j01wd],
		};
		accum += page.lu;
		if (m > 2) {
			accum += page.s.feb_leap;
		}
		if (m > 3) {
			accum += page.s.mar_leap;
		}
		/* load a different page now, shift to the right month */
		page.s = tbl[(j01wd + DT_MONDAY) % GREG_DAYS_P_WEEK];
#if defined WORDS_BIGENDIAN
		page.u <<= 6;
#else  /* !WORDS_BIGENDIAN */
		page.u >>= 6;
#endif	/* WORDS_BIGENDIAN */
		for (unsigned int i = 4; i < m; i++) {
			accum += page.lu;
#if defined WORDS_BIGENDIAN
			page.u <<= 2;
#else  /* !WORDS_BIGENDIAN */
			page.u >>= 2;
#endif	/* WORDS_BIGENDIAN */
		}
	}
	return 20 * (m - 1) + accum + that.bd;
}

static inline dt_bizda_param_t
__get_bizda_param(struct dt_d_s that)
{
#if defined __C1X
	dt_bizda_param_t p = {.bs = that.param};
#else  /* !__C1X */
	dt_bizda_param_t p;
	p.u = that.param;
#endif	/* __C1X */
	return p;
}

static inline dt_bizda_param_t
__make_bizda_param(unsigned int ab, unsigned int ref)
{
#if defined __C1X
	dt_bizda_param_t p = {.ab = ab, .ref = ref};
#else  /* !__C1X */
	dt_bizda_param_t p;
	p.ab = ab;
	p.ref = ref;
#endif	/* __C1X */
	return p;
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
	return (dt_dow_t)(d % GREG_DAYS_P_WEEK);
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
	for (m = 1; m < GREG_MONTHS_P_YEAR && doy > __mon_yday[m + 1]; m++);
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

static int
__ymcw_cmp(dt_ymcw_t d1, dt_ymcw_t d2)
{
	if (d1.y < d2.y) {
		return -1;
	} else if (d1.y > d2.y) {
		return 1;
	} else if (d1.m < d2.m) {
		return -1;
	} else if (d1.m > d2.m) {
		return 1;
	}

	/* we're down to counts, however, the last W of a month is always
	 * count 5, even though counting forward it would be 4 */
	if (d1.c < d2.c) {
		return -1;
	} else if (d1.c > d2.c) {
		return 1;
	}
	/* now it's up to the first of the month */
	{
		dt_dow_t wd01;
		unsigned int off1;
		unsigned int off2;

		wd01 = __get_m01_wday(d1.y, d1.m);
		/* represent cw as C-th WD01 + OFF */
		off1 = __uimod(d1.w - wd01, GREG_DAYS_P_WEEK);
		off2 = __uimod(d2.w - wd01, GREG_DAYS_P_WEEK);

		if (off1 < off2) {
			return -1;
		} else if (off1 > off2) {
			return 1;
		} else {
			return 0;
		}
	}
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
	case DT_DUNK:
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
	case DT_DUNK:
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
	case DT_DUNK:
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
	case DT_DUNK:
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
	case DT_DUNK:
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
		return __bizda_get_yday(that.bizda, __get_bizda_param(that));
	default:
	case DT_DUNK:
		return 0;
	}
}

DEFUN int
dt_get_bday(struct dt_d_s that)
{
/* get N where N is the N-th business day after ultimo */
	switch (that.typ) {
	case DT_BIZDA: {
		dt_bizda_param_t p = __get_bizda_param(that);
		if (p.ab == BIZDA_AFTER && p.ref == BIZDA_ULTIMO) {
			return that.bizda.bd;
		} else if (p.ab == BIZDA_BEFORE && p.ref == BIZDA_ULTIMO) {
			int mb = __get_bdays(that.bizda.y, that.bizda.m);
			return mb - that.bizda.bd;
		}
		return 0;
	}
	case DT_DAISY:
		that.ymd = __daisy_to_ymd(that.daisy);
	case DT_YMD:
		return __ymd_get_bday(
			that.ymd,
			__make_bizda_param(BIZDA_AFTER, BIZDA_ULTIMO));
	case DT_YMCW:
		return __ymcw_get_bday(
			that.ymcw,
			__make_bizda_param(BIZDA_AFTER, BIZDA_ULTIMO));
	default:
	case DT_DUNK:
		return 0;
	}
}

DEFUN int
dt_get_bday_q(struct dt_d_s that, dt_bizda_param_t bp)
{
/* get N where N is the N-th business day Before/After REF */
	switch (that.typ) {
	case DT_BIZDA: {
		dt_bizda_param_t thatp = __get_bizda_param(that);
		if (UNLIKELY(thatp.ref != bp.ref)) {
			;
		} else if (thatp.ab == bp.ab) {
			return that.bizda.bd;
		} else {
			int mb = __get_bdays(that.bizda.y, that.bizda.m);
			return mb - that.bizda.bd;
		}
		return 0/*__bizda_to_bizda(that.bizda, ba, ref)*/;
	}
	case DT_DAISY:
		that.ymd = __daisy_to_ymd(that.daisy);
	case DT_YMD:
		return __ymd_get_bday(that.ymd, bp);
	case DT_YMCW:
		return __ymcw_get_bday(that.ymcw, bp);
	default:
	case DT_DUNK:
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
	case DT_DUNK:
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
	} else if (that.typ == DT_DUNK) {
		return 0;
	}

	y = dt_get_year(that);
	m = dt_get_mon(that);
#if !defined WITH_FAST_ARITH || defined OMIT_FIXUPS
	/* the non-fast arith has done the fixup already */
	d = dt_get_mday(that);
#else  /* WITH_FAST_ARITH && !OMIT_FIXUPS */
	{
		unsigned int tmp = __get_mdays(y, m);
		if (UNLIKELY((d = dt_get_mday(that)) > tmp)) {
			d = tmp;
		}
	}
#endif	/* !WITH_FAST_ARITH || OMIT_FIXUPS */

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
	case DT_DUNK:
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
	case DT_DUNK:
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
	case DT_DUNK:
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
		res += GREG_DAYS_P_WEEK * (b / (signed int)DUWW_BDAYS_P_WEEK);
		b %= (signed int)DUWW_BDAYS_P_WEEK;
		break;
	case DT_SATURDAY:
		res++;
	case DT_SUNDAY:
		res++;
		b--;
		res += GREG_DAYS_P_WEEK * (b / (signed int)DUWW_BDAYS_P_WEEK);
		if ((b %= (signed int)DUWW_BDAYS_P_WEEK) < 0) {
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
		res -= GREG_DAYS_P_WEEK;
		b += DUWW_BDAYS_P_WEEK;
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
__daisy_add(dt_daisy_t d, struct dt_d_s dur)
{
/* add DUR to D, doesn't check if DUR has the dur flag */
	switch (dur.typ) {
	case DT_DAISY:
		d += dur.daisydur;
		break;
	case DT_BIZSI: {
		dt_dow_t dow = __daisy_get_wday(d);
		int dequiv = __get_d_equiv(dow, dur.bizsidur);
		d += dequiv;
		break;
	}
	case DT_YMD:
	case DT_YMCW:
	case DT_BIZDA:
		/* daisies have no notion of years and months */
	case DT_DUNK:
	default:
		break;
	}
	return d;
}

static struct dt_d_s
__daisy_diff(dt_daisy_t d1, dt_daisy_t d2)
{
/* compute d2 - d1 */
	struct dt_d_s res = {.typ = DT_DAISY, .dur = 1};
	int32_t diff = d2 - d1;

	res.daisydur = diff;
	return res;
}

static inline void
__fill_strpdi(struct strpdi_s *tgt, struct dt_d_s dur)
{
	switch (dur.typ) {
	case DT_YMD:
		tgt->m = dur.ymd.y * GREG_MONTHS_P_YEAR + dur.ymd.m;
		tgt->d = dur.ymd.d;
		break;
	case DT_DAISY:
		tgt->d = dur.daisydur;
		/* we don't need the negation, so return here */
		return;
	case DT_BIZSI:
		tgt->b = dur.bizsidur;
		/* we don't need the negation, so return here */
		return;
	case DT_BIZDA:
		tgt->m = dur.bizda.y * GREG_MONTHS_P_YEAR + dur.bizda.m;
		tgt->b = dur.bizda.bd;
		break;
	case DT_YMCW:
		tgt->m = dur.ymcw.y * GREG_MONTHS_P_YEAR + dur.ymcw.m;
		tgt->w = dur.ymcw.c;
		tgt->d = dur.ymcw.w;
		break;
	case DT_MD:
		tgt->m = dur.md.m;
		tgt->d = dur.md.d;
		break;
	default:
		break;
	}

	if (UNLIKELY(dur.neg)) {
		tgt->m = -tgt->m;
		tgt->d = -tgt->d;
		tgt->w = -tgt->w;
		tgt->b = -tgt->b;
	}
	return;
}

static dt_ymd_t
__ymd_add(dt_ymd_t d, struct dt_d_s dur)
{
/* add DUR to D, doesn't check if DUR has the dur flag */
	unsigned int tgty = 0;
	unsigned int tgtm = 0;
	signed int tgtd = 0;
	struct strpdi_s durcch = {0};

	/* using the strpdi blob is easier */
	__fill_strpdi(&durcch, dur);

	switch (dur.typ) {
		unsigned int mdays;
	case DT_YMD:
	case DT_YMCW:
	case DT_BIZDA:
	case DT_MD:
		/* construct new month */
		durcch.m += d.m - 1;
		tgty = __uidiv(durcch.m, GREG_MONTHS_P_YEAR) + d.y;
		tgtm = __uimod(durcch.m, GREG_MONTHS_P_YEAR) + 1;

		/* fixup day */
		if ((tgtd = d.d) > (int)(mdays = __get_mdays(tgty, tgtm))) {
			tgtd = mdays;
		}
		/* otherwise we may need to fixup the day, let's do that
		 * in the next step */
		/* @fallthrough@ */
	case DT_DAISY:
	case DT_BIZSI:
		switch (dur.typ) {
		case DT_YMD:
		case DT_MD:
			/* fallthrough from above */
			tgtd += durcch.d;
			break;
		case DT_DAISY:
			tgtd = d.d + durcch.d;
			mdays = __get_mdays((tgty = d.y), (tgtm = d.m));
			break;
		case DT_BIZDA: {
			/* fallthrough from above */
			/* construct a tentative result */
			dt_dow_t tent = __ymd_get_wday(d);
			d.y = tgty;
			d.m = tgtm;
			d.d = tgtd;
			tgtd += __get_d_equiv(tent, durcch.b);
			break;
		}
		case DT_BIZSI: {
			/* construct a tentative result */
			dt_dow_t tent = __ymd_get_wday(d);
			tgtd = d.d + __get_d_equiv(tent, durcch.b);
			mdays = __get_mdays((tgty = d.y), (tgtm = d.m));
			break;
		}
		case DT_YMCW:
			/* doesn't happen as the dur parser won't
			 * hand out durs of type YMCW */
			/* @fallthrough@ */
		default:
			mdays = 0;
			tgtd = 0;
			break;
		}
		/* fixup the day */
		while (tgtd > (int)mdays) {
			tgtd -= mdays;
			if (++tgtm > GREG_MONTHS_P_YEAR) {
				++tgty;
				tgtm = 1;
			}
			mdays = __get_mdays(tgty, tgtm);
		}
		/* and the other direction */
		while (tgtd < 1) {
			if (--tgtm < 1) {
				--tgty;
				tgtm = GREG_MONTHS_P_YEAR;
			}
			mdays = __get_mdays(tgty, tgtm);
			tgtd += mdays;
		}
		break;
	case DT_DUNK:
	default:
		break;
	}
	d.y = tgty;
	d.m = tgtm;
	d.d = tgtd;
	return d;
}

static struct dt_d_s
__ymd_diff(dt_ymd_t d1, dt_ymd_t d2)
{
/* compute d2 - d1 entirely in terms of ymd */
	struct dt_d_s res = {.typ = DT_YMD, .dur = 1};
	signed int tgtd;
	signed int tgtm;

	if (d1.u > d2.u) {
		/* swap d1 and d2 */
		dt_ymd_t tmp = d1;
		res.neg = 1;
		d1 = d2;
		d2 = tmp;
	}

	/* first compute the difference in months Y2-M2-01 - Y1-M1-01 */
	tgtm = GREG_MONTHS_P_YEAR * (d2.y - d1.y) + (d2.m - d1.m);
	if ((tgtd = d2.d - d1.d) < 1 && tgtm != 0) {
		/* if tgtm is 0 it remains 0 and tgtd remains negative */
		/* get the target month's mdays */
		unsigned int d2m = d2.m;
		unsigned int d2y = d2.y;

		if (--d2m < 1) {
			d2m = GREG_MONTHS_P_YEAR;
			d2y--;
		}
		tgtd += __get_mdays(d2y, d2m);
		tgtm--;
#if !defined WITH_FAST_ARITH || defined OMIT_FIXUPS
		/* the non-fast arith has done the fixup already */
#else  /* WITH_FAST_ARITH && !defined OMIT_FIXUPS */
	} else if (tgtm == 0) {
		/* check if we're not diffing two lazy representations
		 * e.g. 2010-02-28 and 2010-02-31 */
		;
#endif	/* !OMIT_FIXUPS */
	}
	/* fill in the results */
	res.ymd.y = tgtm / GREG_MONTHS_P_YEAR;
	res.ymd.m = tgtm % GREG_MONTHS_P_YEAR;
	res.ymd.d = tgtd;
	return res;
}

static dt_ymcw_t
__ymcw_add(dt_ymcw_t d, struct dt_d_s dur)
{
/* here's a short draft of the arithmetic for ymcw dates:
 * Y-m-c-w + n years -> (Y + n)-m-c-w
 * Y-m-c-w + n months -> Y-(m + n)-c-w
 * Y-m-c-w + n weeks -> Y'-m'-c'-w
 * Y-m-c-w + n days -> Y'-m'-c'-w' */
	unsigned int tgty;
	unsigned int tgtm;
	unsigned int tgtc;
	dt_dow_t tgtw;
	struct strpdi_s durcch = {0};

	/* first off, give DUR a make-over */
	__fill_strpdi(&durcch, dur);

	switch (dur.typ) {
	case DT_YMD:
	case DT_MD:
	case DT_YMCW:
	case DT_BIZDA:
		/* construct new month */
		durcch.m += d.m - 1;
		tgty = __uidiv(durcch.m, 12) + d.y;
		tgtm = __uimod(durcch.m, 12) + 1;

		/* otherwise we may need to fixup the day, let's do that
		 * in the next step */
	case DT_DAISY:
	case DT_BIZSI: {
		signed int q;
		signed int p;
		signed int mc;

		/* factorise durcch.d into q + p, q = 7k, 0 <= p < 7 */
		q = __uidiv(durcch.d, GREG_DAYS_P_WEEK);
		p = __uimod(durcch.d, GREG_DAYS_P_WEEK);

		tgty = d.y;
		tgtm = d.m;
		q = d.c - 1 + q;
		tgtw = (dt_dow_t)d.w;

		while (1) {
			if (q < 0) {
				if (UNLIKELY(--tgtm < 1)) {
					tgtm = GREG_MONTHS_P_YEAR;
					tgty--;
				}
				mc = __get_mcnt(tgty, tgtm, tgtw);
				q += mc;
			} else if (q >= (mc = __get_mcnt(tgty, tgtm, tgtw))) {
				q -= mc;
				if (UNLIKELY(++tgtm > GREG_MONTHS_P_YEAR)) {
					tgtm = 1;
					tgty++;
				}
			} else {
				break;
			}
		}

		/* offset against p (we know p is >= 0)
		 * and mc still holds the mcnt value for the weekday
		 * we started on */
		tgtc = q + 1;
		if (p) {
			unsigned int newmc;

			tgtw = (dt_dow_t)((tgtw + p) % GREG_DAYS_P_WEEK);
			newmc = __get_mcnt(tgty, tgtm, tgtw);
			if (newmc > (unsigned int)mc) {
				/* cant go negative as we used __uimod above
				 * to properly start left of the fixup */
				if (++tgtc > newmc) {
					/* fixup c, m and y too */
					tgtc = 1;
					if (++tgtm > GREG_MONTHS_P_YEAR) {
						tgty++;
						tgtm = 1;
					}
				}
			}
			/* otherwise it's the same c within the month */
		}
		break;
	}
	case DT_DUNK:
	default:
		tgty = tgtm = tgtc = 0;
		tgtw = DT_MIRACLEDAY;
		break;
	}
	/* reassign to the guy in question */
	d.y = tgty;
	d.m = tgtm;
	d.c = tgtc;
	d.w = tgtw;
	return d;
}

static struct dt_d_s
__ymcw_diff(dt_ymcw_t d1, dt_ymcw_t d2)
{
/* compute d2 - d1 entirely in terms of ymd */
	struct dt_d_s res = {.typ = DT_YMCW, .dur = 1};
	signed int tgtd;
	signed int tgtm;
	dt_dow_t wd01, wd02;

	if (__ymcw_cmp(d1, d2) > 0) {
		dt_ymcw_t tmp = d1;
		d1 = d2;
		d2 = tmp;
		res.neg = 1;
	}

	wd01 = __get_m01_wday(d1.y, d1.m);
	if (d2.y != d1.y || d2.m != d1.m) {
		wd02 = __get_m01_wday(d2.y, d2.m);
	} else {
		wd02 = wd01;
	}

	/* first compute the difference in months Y2-M2-01 - Y1-M1-01 */
	tgtm = GREG_MONTHS_P_YEAR * (d2.y - d1.y) + (d2.m - d1.m);
	/* using the firsts of the month WD01, represent d1 and d2 as
	 * the C-th WD01 plus OFF */
	{
		unsigned int off1;
		unsigned int off2;

		off1 = __uimod(d1.w - wd01, GREG_DAYS_P_WEEK);
		off2 = __uimod(d2.w - wd02, GREG_DAYS_P_WEEK);
		tgtd = off2 - off1 + GREG_DAYS_P_WEEK * (d2.c - d1.c);
	}

	/* fixups */
	if (tgtd < (signed int)GREG_DAYS_P_WEEK && tgtm > 0) {
		/* if tgtm is 0 it remains 0 and tgtd remains negative */
		/* get the target month's mdays */
		unsigned int d2m = d2.m;
		unsigned int d2y = d2.y;

		if (--d2m < 1) {
			d2m = GREG_MONTHS_P_YEAR;
			d2y--;
		}
		tgtd += __get_mdays(d2y, d2m);
		tgtm--;
	}

	/* fill in the results */
	res.ymcw.y = tgtm / GREG_MONTHS_P_YEAR;
	res.ymcw.m = tgtm % GREG_MONTHS_P_YEAR;
	res.ymcw.c = tgtd / GREG_DAYS_P_WEEK;
	res.ymcw.w = tgtd % GREG_DAYS_P_WEEK;
	return res;
}


/* guessing parsers */
#include "token.c"
#include "strops.c"

#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

static struct dt_d_s
__guess_dtyp(struct strpd_s d)
{
	struct dt_d_s res = dt_d_initialiser();

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

	if (LIKELY(d.y && (d.m == 0 || d.c == 0) && !d.flags.bizda)) {
		/* nearly all goes to ymd */
		res.typ = DT_YMD;
		res.ymd.y = d.y;
		if (LIKELY(!d.flags.d_dcnt_p)) {
			res.ymd.m = d.m;
#if defined WITH_FAST_ARITH
			res.ymd.d = d.d;
#else  /* !WITH_FAST_ARITH */
			/* check for illegal dates, like 31st of April */
			if ((res.ymd.d = __get_mdays(d.y, d.m)) > d.d) {
				res.ymd.d = d.d;
			}
		} else {
			/* convert dcnt to m + d */
			struct __md_s r = __yday_get_md(d.y, d.d);
			res.ymd.m = r.m;
			res.ymd.d = r.d;
		}
#endif	/* !WITH_FAST_ARITH */
	} else if (d.y && d.c && !d.flags.bizda) {
		/* its legit for d.w to be naught */
		res.typ = DT_YMCW;
		res.ymcw.y = d.y;
		res.ymcw.m = d.m;
		res.ymcw.c = d.c;
		res.ymcw.w = d.w;
	} else if (d.y && d.flags.bizda) {
		/* d.c can be legit'ly naught */
		dt_bizda_param_t bp = __make_bizda_param(d.flags.ab, 0);
		res.param = bp.u;
		res.typ = DT_BIZDA;
		res.bizda.y = d.y;
		res.bizda.m = d.m;
		res.bizda.bd = d.b;
	} else {
		/* anything else is bollocks for now */
		;
	}
	return res;
}

static const char ymd_dflt[] = "%F";
static const char ymcw_dflt[] = "%Y-%m-%c-%w";
static const char daisy_dflt[] = "%d";
static const char bizsi_dflt[] = "%db";
static const char bizda_dflt[] = "%Y-%m-%db";

static void
__trans_dfmt(const char **fmt)
{
	if (UNLIKELY(*fmt == NULL)) {
		/* great, standing ovations to the user */
		;
	} else if (LIKELY(**fmt == '%')) {
		/* don't worry about it */
		;
	} else if (strcasecmp(*fmt, "ymd") == 0) {
		*fmt = ymd_dflt;
	} else if (strcasecmp(*fmt, "ymcw") == 0) {
		*fmt = ymcw_dflt;
	} else if (strcasecmp(*fmt, "bizda") == 0) {
		*fmt = bizda_dflt;
	} else if (strcasecmp(*fmt, "daisy") == 0) {
		*fmt = daisy_dflt;
	} else if (strcasecmp(*fmt, "bizsi") == 0) {
		*fmt = bizsi_dflt;
	}
	return;
}

static struct dt_d_s
__strpd_std(const char *str, char **ep)
{
	struct dt_d_s res;
	struct strpd_s d = {0};
	const char *sp;

	if ((sp = str) == NULL) {
		goto out;
	}
	/* read the year */
	if ((d.y = strtoui_lim(sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR)) == -1U ||
	    *sp++ != '-') {
		goto fucked;
	}
	/* read the month */
	if ((d.m = strtoui_lim(sp, &sp, 0, GREG_MONTHS_P_YEAR)) == -1U ||
	    *sp++ != '-') {
		goto fucked;
	}
	/* read the day or the count */
	if ((d.d = strtoui_lim(sp, &sp, 0, 31)) == -1U) {
		/* didn't work, fuck off */
		goto fucked;
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
		sp++;
		if ((d.w = strtoui_lim(sp, &sp, 0, GREG_DAYS_P_WEEK)) == -1U) {
			/* didn't work, fuck off */
			goto fucked;
		}
		break;
	case 'B':
		/* it's a bizda/YMDU before ultimo date */
		d.flags.ab = BIZDA_BEFORE;
	case 'b':
		/* it's a bizda/YMDU after ultimo date */
		d.flags.bizda = 1;
		d.b = d.d;
		d.d = 0;
		sp++;
		break;
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
fucked:
	if (ep) {
		*ep = (char*)str;
	}
	return dt_d_initialiser();
}

static int
__strpd_card(struct strpd_s *d, const char *sp, struct dt_spec_s s, char **ep)
{
	/* we're really pessimistic, aren't we? */
	int res = -1;

	switch (s.spfl) {
	default:
	case DT_SPFL_UNK:
		break;
	case DT_SPFL_N_DSTD:
		d->y = strtoui_lim(sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR);
		sp++;
		d->m = strtoui_lim(sp, &sp, 0, GREG_MONTHS_P_YEAR);
		sp++;
		d->d = strtoui_lim(sp, &sp, 0, 31);
		res = 0 - (d->y == -1U || d->m == -1U || d->d == -1U);
		break;
	case DT_SPFL_N_YEAR:
		if (s.abbr == DT_SPMOD_NORM) {
			d->y = strtoui_lim(sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR);
		} else if (s.abbr == DT_SPMOD_ABBR) {
			d->y = strtoui_lim(sp, &sp, 0, 99);
			if (UNLIKELY(d->y == -1U)) {
				;
			} else if ((d->y += 2000) > 2068) {
				d->y -= 100;
			}
		}
		res = 0 - (d->y == -1U);
		break;
	case DT_SPFL_N_MON:
		d->m = strtoui_lim(sp, &sp, 0, GREG_MONTHS_P_YEAR);
		res = 0 - (d->m == -1U);
		break;
	case DT_SPFL_N_DCNT_MON:
		/* ymd mode? */
		if (LIKELY(!s.bizda)) {
			d->d = strtoui_lim(sp, &sp, 0, 31);
			res = 0 - (d->d == -1U);
		} else {
			d->b = strtoui_lim(sp, &sp, 0, 23);
			res = 0 - (d->b == -1U);
		}
		break;
	case DT_SPFL_N_DCNT_WEEK:
		/* ymcw mode? */
		d->w = strtoui_lim(sp, &sp, 0, GREG_DAYS_P_WEEK);
		res = 0 - (d->w == -1U);
		break;
	case DT_SPFL_N_WCNT_MON:
		/* ymcw mode? */
		d->c = strtoui_lim(sp, &sp, 0, 5);
		res = 0 - (d->c == -1U);
		break;
	case DT_SPFL_S_WDAY:
		/* ymcw mode? */
		switch (s.abbr) {
		case DT_SPMOD_NORM:
			d->w = strtoarri(
				sp, &sp,
				__abbr_wday, countof(__abbr_wday));
			break;
		case DT_SPMOD_LONG:
			d->w = strtoarri(
				sp, &sp,
				__long_wday,
				countof(__long_wday));
			break;
		case DT_SPMOD_ABBR: {
			const char *pos;
			if ((pos = strchr(__abab_wday, *sp++))) {
				d->w = pos - __abab_wday;
			} else {
				d->w = -1;
			}
			break;
		}
		case DT_SPMOD_ILL:
		default:
			break;
		}
		res = 0 - (d->w == -1U);
		break;
	case DT_SPFL_S_MON:
		switch (s.abbr) {
		case DT_SPMOD_NORM:
			d->m = strtoarri(
				sp, &sp,
				__abbr_mon,
				countof(__abbr_mon));
			break;
		case DT_SPMOD_LONG:
			d->m = strtoarri(
				sp, &sp,
				__long_mon,
				countof(__long_mon));
			break;
		case DT_SPMOD_ABBR: {
			const char *pos;
			if ((pos = strchr(__abab_mon, *sp++))) {
				d->m = pos - __abab_mon;
			} else {
				d->m = -1U;
			}
			break;
		}
		case DT_SPMOD_ILL:
		default:
			break;
		}
		res = 0 - (d->m == -1U);
		break;
	case DT_SPFL_S_QTR:
		if (*sp++ != 'Q') {
			break;
		}
	case DT_SPFL_N_QTR:
		if (d->m == 0) {
			unsigned int q;
			if ((q = strtoui_lim(sp, &sp, 1, 4)) < -1U) {
				d->m = q * 3 - 2;
				res = 0;
			}
		}
		break;

	case DT_SPFL_LIT_PERCENT:
		if (*sp++ == '%') {
			res = 0;
		}
		break;
	case DT_SPFL_LIT_TAB:
		if (*sp++ == '\t') {
			res = 0;
		}
		break;
	case DT_SPFL_LIT_NL:
		if (*sp++ == '\n') {
			res = 0;
		}
		break;
	case DT_SPFL_N_DCNT_YEAR:
		/* was %D and %j, cannot be used at the moment */
		if ((d->d = strtoui_lim(sp, &sp, 1, 366)) < -1U) {
			res = 0;
			d->flags.d_dcnt_p = 1;
		}
		break;
	case DT_SPFL_N_WCNT_YEAR:
		/* was %C, cannot be used at the moment */
		(void)strtoui_lim(sp, &sp, 0, 53);
		break;
	}
	/* assign end pointer */
	if (ep) {
		*ep = (char*)sp;
	}
	return res;
}

static int
__strpd_rom(struct strpd_s *d, const char *sp, struct dt_spec_s s, char **ep)
{
	int res = -1;

	switch (s.spfl) {
	default:
	case DT_SPFL_UNK:
		break;

	case DT_SPFL_N_YEAR:
		if (s.abbr == DT_SPMOD_NORM) {
			d->y = romstrtoui_lim(
				sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR);
		} else if (s.abbr == DT_SPMOD_ABBR) {
			d->y = romstrtoui_lim(sp, &sp, 0, 99);
			if (UNLIKELY(d->y == -1U)) {
				;
			} else if ((d->y += 2000) > 2068) {
				d->y -= 100;
			}
		}
		res = 0 - (d->y == -1U);
		break;
	case DT_SPFL_N_MON:
		d->m = romstrtoui_lim(sp, &sp, 0, GREG_MONTHS_P_YEAR);
		res = 0 - (d->m == -1U);
		break;
	case DT_SPFL_N_DCNT_MON:
		d->d = romstrtoui_lim(sp, &sp, 0, 31);
		res = 0 - (d->d == -1U);
		break;
	case DT_SPFL_N_WCNT_MON:
		d->c = romstrtoui_lim(sp, &sp, 0, 5);
		res = 0 - (d->c == -1U);
		break;
	}
	if (ep) {
		*ep = (char*)sp;
	}
	return res;
}

static size_t
__strfd_card(
	char *buf, size_t bsz, struct dt_spec_s s,
	struct strpd_s *d, struct dt_d_s that)
{
	size_t res = 0;

	switch (s.spfl) {
	default:
	case DT_SPFL_UNK:
		break;
	case DT_SPFL_N_DSTD:
		d->d = d->d ?: (unsigned int)dt_get_mday(that);
		if (LIKELY(bsz >= 10)) {
			ui32tostr(buf + 0, bsz, d->y, 4);
			buf[4] = '-';
			res = ui32tostr(buf + 5, bsz, d->m, 2);
			buf[7] = '-';
			ui32tostr(buf + 8, bsz, d->d, 2);
			res = 10;
		}
		break;
	case DT_SPFL_N_YEAR:
		if (s.abbr == DT_SPMOD_NORM) {
			res = ui32tostr(buf, bsz, d->y, 4);
		} else if (s.abbr == DT_SPMOD_ABBR) {
			res = ui32tostr(buf, bsz, d->y, 2);
		}
		break;
	case DT_SPFL_N_MON:
		res = ui32tostr(buf, bsz, d->m, 2);
		break;
	case DT_SPFL_N_DCNT_MON:
		/* ymd mode check? */
		if (LIKELY(!s.bizda)) {
			d->d = d->d ?: (unsigned int)dt_get_mday(that);
			res = ui32tostr(buf, bsz, d->d, 2);
		} else {
			int bd = dt_get_bday_q(
				that, __make_bizda_param(s.ab, BIZDA_ULTIMO));
			res = ui32tostr(buf, bsz, bd, 2);
		}
		break;
	case DT_SPFL_N_DCNT_WEEK:
		/* ymcw mode check */
		d->w = d->w ?: dt_get_wday(that);
		res = ui32tostr(buf, bsz, d->w, 2);
		break;
	case DT_SPFL_N_WCNT_MON:
		/* ymcw mode check? */
		d->c = d->c ?: (unsigned int)dt_get_count(that);
		res = ui32tostr(buf, bsz, d->c, 2);
		break;
	case DT_SPFL_S_WDAY:
		/* get the weekday in ymd mode!! */
		d->w = d->w ?: dt_get_wday(that);
		switch (s.abbr) {
		case DT_SPMOD_NORM:
			res = arritostr(
				buf, bsz, d->w,
				__abbr_wday, countof(__abbr_wday));
			break;
		case DT_SPMOD_LONG:
			res = arritostr(
				buf, bsz, d->w,
				__long_wday, countof(__long_wday));
			break;
		case DT_SPMOD_ABBR:
			/* super abbrev'd wday */
			if (d->w < countof(__abab_wday)) {
				buf[res++] = __abab_wday[d->w];
			}
			break;
		case DT_SPMOD_ILL:
		default:
			break;
		}
		break;
	case DT_SPFL_S_MON:
		switch (s.abbr) {
		case DT_SPMOD_NORM:
			res = arritostr(
				buf, bsz, d->m,
				__abbr_mon, countof(__abbr_mon));
			break;
		case DT_SPMOD_LONG:
			res = arritostr(
				buf, bsz, d->m,
				__long_mon, countof(__long_mon));
			break;
		case DT_SPMOD_ABBR:
			/* super abbrev'd month */
			if (d->m < countof(__abab_mon)) {
				buf[res++] = __abab_mon[d->m];
			}
			break;
		case DT_SPMOD_ILL:
		default:
			break;
		}
		break;
	case DT_SPFL_S_QTR:
		buf[res++] = 'Q';
		buf[res++] = (char)(dt_get_quarter(that) + '0');
		break;
	case DT_SPFL_N_QTR:
		buf[res++] = '0';
		buf[res++] = (char)(dt_get_quarter(that) + '0');
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

	case DT_SPFL_N_DCNT_YEAR:
		if (that.typ == DT_YMD || that.typ == DT_BIZDA) {
			/* %j */
			int yd;
			if (LIKELY(!s.bizda)) {
				yd = __ymd_get_yday(that.ymd);
			} else {
				yd = __bizda_get_yday(
					that.bizda, __get_bizda_param(that));
			}
			if (yd >= 0) {
				res = ui32tostr(buf, bsz, yd, 3);
			} else {
				buf[res++] = '0';
				buf[res++] = '0';
				buf[res++] = '0';
			}
		}
		break;
	case DT_SPFL_N_WCNT_YEAR: {
		int yw;
		/* %C/%W week count */
		switch (that.typ) {
		case DT_YMD:
			if (s.cnt_weeks_iso) {
				yw = __ymd_get_wcnt_iso(that.ymd);
			} else {
				yw = __ymd_get_wcnt(that.ymd, s.cnt_wdays_from);
			}
			break;
		case DT_YMCW:
			yw = __ymcw_get_yday(that.ymcw);
			break;
		default:
			yw = 0;
			break;
		}
		res = ui32tostr(buf, bsz, yw, 2);
		break;
	}
	}
	return res;
}

static size_t
__strfd_rom(
	char *buf, size_t bsz, struct dt_spec_s s,
	struct strpd_s *d, struct dt_d_s that)
{
	size_t res = 0;

	if (that.typ != DT_YMD) {
		/* not supported for non-ymds */
		return res;
	}

	switch (s.spfl) {
	default:
	case DT_SPFL_UNK:
		break;
	case DT_SPFL_N_YEAR:
		if (s.abbr == DT_SPMOD_NORM) {
			res = ui32tostrrom(buf, bsz, d->y);
			break;
		} else if (s.abbr == DT_SPMOD_ABBR) {
			res = ui32tostrrom(buf, bsz, d->y % 100);
			break;
		}
		break;
	case DT_SPFL_N_MON:
		res = ui32tostrrom(buf, bsz, d->m);
		break;
	case DT_SPFL_N_DCNT_MON:
		res = ui32tostrrom(buf, bsz, d->d);
		break;
	case DT_SPFL_N_WCNT_MON:
		d->c = d->c ?: (unsigned int)dt_get_count(that);
		res = ui32tostrrom(buf, bsz, d->c);
		break;
	}
	return res;
}

static size_t
__strfd_dur(
	char *buf, size_t bsz, struct dt_spec_s s,
	struct strpd_s *d, struct dt_d_s UNUSED(that))
{
	size_t res = 0;

	switch (s.spfl) {
	default:
	case DT_SPFL_UNK:
		break;
	case DT_SPFL_N_DSTD:
	case DT_SPFL_N_DCNT_MON:
		res = snprintf(buf, bsz, "%d", d->sd);
		break;
	case DT_SPFL_N_YEAR:
		if (!d->y) {
			/* fill in for a mo, hack hack hack
			 * we'll think about the consequences later */
			d->y = __uidiv(d->m, GREG_MONTHS_P_YEAR);
			d->m = __uimod(d->m, GREG_MONTHS_P_YEAR);
		}
		res = snprintf(buf, bsz, "%u", d->y);
		break;
	case DT_SPFL_N_MON:
		res = snprintf(buf, bsz, "%u", d->m);
		break;
	case DT_SPFL_N_DCNT_WEEK:
		if (!d->w) {
			/* hack hack hack
			 * we'll think about the consequences later */
			d->w = __uidiv(d->d, GREG_DAYS_P_WEEK);
			d->d = __uimod(d->d, GREG_DAYS_P_WEEK);
		}
		res = snprintf(buf, bsz, "%u", d->w);
		break;
	case DT_SPFL_N_WCNT_MON:
		res = snprintf(buf, bsz, "%u", d->c);
		break;
	case DT_SPFL_S_WDAY:
	case DT_SPFL_S_MON:
	case DT_SPFL_S_QTR:
	case DT_SPFL_N_QTR:
	case DT_SPFL_N_DCNT_YEAR:
	case DT_SPFL_N_WCNT_YEAR:
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
DEFUN struct dt_d_s
dt_strpd(const char *str, const char *fmt, char **ep)
{
	struct dt_d_s res = dt_d_initialiser();
	struct strpd_s d = {0};
	const char *sp = str;
	const char *fp = fmt;

	if (UNLIKELY(fmt == NULL)) {
		return __strpd_std(str, ep);
	}
	/* translate high-level format names */
	__trans_dfmt(&fmt);

	while (*fp && *sp) {
		const char *fp_sav = fp;
		struct dt_spec_s spec = __tok_spec(fp_sav, (char**)&fp);

		if (spec.spfl == DT_SPFL_UNK) {
			/* must be literal */
			if (*fp_sav != *sp++) {
				sp = str;
				goto out;
			}
		} else if (LIKELY(!spec.rom)) {
			const char *sp_sav = sp;
			if (__strpd_card(&d, sp, spec, (char**)&sp) < 0) {
				sp = str;
				goto out;
			}
			if (spec.ord &&
			    __ordinalp(sp_sav, sp - sp_sav, (char**)&sp) < 0) {
				;
			}
			if (spec.bizda) {
				switch (*sp++) {
				case 'B':
					d.flags.ab = BIZDA_BEFORE;
				case 'b':
					d.flags.bizda = 1;
					break;
				default:
					/* it's a bizda anyway */
					d.flags.bizda = 1;
					sp--;
					break;
				}
			}
		} else if (UNLIKELY(spec.rom)) {
			if (__strpd_rom(&d, sp, spec, (char**)&sp) < 0) {
				sp = str;
				goto out;
			}
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
	struct strpd_s d = {0};
	const char *fp;
	char *bp;

	if (UNLIKELY(buf == NULL || bsz == 0)) {
		return 0;
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
		d.c = that.ymcw.c;
		d.w = that.ymcw.w;
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
	case DT_BIZDA: {
		dt_bizda_param_t bparam = __get_bizda_param(that);
		d.y = that.bizda.y;
		d.m = that.bizda.m;
		d.b = that.bizda.bd;
		if (LIKELY(bparam.ab == BIZDA_AFTER)) {
			d.flags.ab = BIZDA_AFTER;
		} else {
			d.flags.ab = BIZDA_BEFORE;
		}
		d.flags.bizda = 1;
		if (fmt == NULL) {
			fmt = bizda_dflt;
		}
		break;
	}
	default:
	case DT_DUNK:
		bp = buf;
		goto out;
	}
	/* translate high-level format names */
	__trans_dfmt(&fmt);

	/* assign and go */
	bp = buf;
	fp = fmt;
	for (char *const eo = buf + bsz; *fp && bp < eo;) {
		const char *fp_sav = fp;
		struct dt_spec_s spec = __tok_spec(fp_sav, (char**)&fp);

		if (spec.spfl == DT_SPFL_UNK) {
			/* must be literal then */
			*bp++ = *fp_sav;
		} else if (LIKELY(!spec.rom)) {
			bp += __strfd_card(bp, eo - bp, spec, &d, that);
			if (spec.ord) {
				bp += __ordtostr(bp, eo - bp);
			} else if (spec.bizda) {
				/* don't print the b after an ordinal */
				if (spec.ab == BIZDA_AFTER) {
					*bp++ = 'b';
				} else {
					*bp++ = 'B';
				}
			}
		} else if (UNLIKELY(spec.rom)) {
			bp += __strfd_rom(bp, eo - bp, spec, &d, that);
		}
	}
	if (bp < buf + bsz) {
	out:
		*bp = '\0';
	}
	return bp - buf;
}

DEFUN struct dt_d_s
dt_strpddur(const char *str, char **ep)
{
/* at the moment we allow only one format */
	struct dt_d_s res = {DT_DUNK};
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
	if (d.b && ((d.m >= 1 && d.m <= GREG_MONTHS_P_YEAR) || d.y)) {
		res.typ = DT_BIZDA;
		res.bizda.y = d.y;
		res.bizda.m = d.q * 3 + d.m;
		res.bizda.bd = d.b + d.w * DUWW_BDAYS_P_WEEK;
	} else if (LIKELY((d.m >= 1 && d.m <= GREG_MONTHS_P_YEAR) || d.y)) {
		res.typ = DT_YMD;
		res.ymd.y = d.y;
		res.ymd.m = d.q * 3 + d.m;
		res.ymd.d = d.d + d.w * GREG_DAYS_P_WEEK;
	} else if (d.d) {
		res.typ = DT_DAISY;
		res.daisydur = d.w * GREG_DAYS_P_WEEK + d.d;
	} else if (d.b) {
		res.typ = DT_BIZSI;
		res.bizsidur = d.w * DUWW_BDAYS_P_WEEK + d.b;
	} else {
		/* we leave out YMCW diffs simply because YMD diffs
		 * cover them better
		 * anything that doesn't fit shall be mapped to MD durs
		 * using the definitions of MONTHS_P_YEAR and DAYS_P_WEEK */
		res.typ = DT_MD;
		res.md.d = d.w * GREG_DAYS_P_WEEK + d.d;
		res.md.m = d.y * GREG_MONTHS_P_YEAR + d.m;
	}
out:
	if (ep) {
		*ep = (char*)sp;
	}
	return res;
}

DEFUN size_t
dt_strfddur(char *restrict buf, size_t bsz, const char *fmt, struct dt_d_s that)
{
	struct strpd_s d = {0};
	const char *fp;
	char *bp;

	if (UNLIKELY(buf == NULL || bsz == 0 || !that.dur)) {
		return 0;
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
		d.c = that.ymcw.c;
		d.w = that.ymcw.w;
		if (fmt == NULL) {
			fmt = ymcw_dflt;
		}
		break;
	case DT_DAISY:
		if (that.daisydur >= 0) {
			d.d = that.daisydur;
		} else {
			d.d = -that.daisydur;
			/* make sure the neg bit doesn't bite us */
			that.neg = 1;
		}
		if (fmt == NULL) {
			/* subject to change */
			fmt = daisy_dflt;
		}
		break;
	case DT_BIZSI:
		if (that.bizsidur >= 0) {
			d.d = that.bizsidur;
		} else {
			d.d = -that.bizsidur;
			/* make sure the neg bit doesn't bite us */
			that.neg = 1;
		}
		if (fmt == NULL) {
			/* subject to change */
			fmt = bizsi_dflt;
		}
		break;
	case DT_BIZDA: {
		dt_bizda_param_t bparam = __get_bizda_param(that);
		d.y = that.bizda.y;
		d.m = that.bizda.m;
		d.b = that.bizda.bd;
		if (LIKELY(bparam.ab == BIZDA_AFTER)) {
			d.flags.ab = BIZDA_AFTER;
		} else {
			d.flags.ab = BIZDA_BEFORE;
		}
		d.flags.bizda = 1;
		if (fmt == NULL) {
			fmt = bizda_dflt;
		}
		break;
	}
	case DT_MD:
		d.y = __uidiv(that.md.m, GREG_MONTHS_P_YEAR);
		d.m = __uimod(that.md.m, GREG_MONTHS_P_YEAR);
		d.w = __uidiv(that.md.d, GREG_DAYS_P_WEEK);
		d.d = __uimod(that.md.d, GREG_DAYS_P_WEEK);
		break;
	default:
	case DT_DUNK:
		bp = buf;
		goto out;
	}
	/* translate high-level format names */
	__trans_dfmt(&fmt);

	/* assign and go */
	bp = buf;
	fp = fmt;
	if (that.neg) {
		*bp++ = '-';
	}
	for (char *const eo = buf + bsz; *fp && bp < eo;) {
		const char *fp_sav = fp;
		struct dt_spec_s spec = __tok_spec(fp_sav, (char**)&fp);

		if (spec.spfl == DT_SPFL_UNK) {
			/* must be literal then */
			*bp++ = *fp_sav;
		} else if (LIKELY(!spec.rom)) {
			bp += __strfd_dur(bp, eo - bp, spec, &d, that);
			if (spec.bizda) {
				/* don't print the b after an ordinal */
				if (d.flags.ab == BIZDA_AFTER) {
					*bp++ = 'b';
				} else {
					*bp++ = 'B';
				}
			}
		}
	}
	if (bp < buf + bsz) {
	out:
		*bp = '\0';
	}
	return bp - buf;
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
		default:
			break;
		}
		break;
	}
	case DT_DAISY:
		/* time_t's base is 1970-01-01, which is daisy 19359 */
		res.daisy = t / 86400 + 19359;
		break;
	default:
	case DT_MD:
		/* doesn't make sense */
	case DT_DUNK:
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
	case DT_DUNK:
	default:
		res.typ = DT_DUNK;
		break;
	}
	return res;
}

DEFUN struct dt_d_s
dt_dadd(struct dt_d_s d, struct dt_d_s dur)
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

	case DT_DUNK:
	default:
		d.typ = DT_DUNK;
		d.u = 0;
		break;
	}
	return d;
}

DEFUN struct dt_d_s
dt_neg_dur(struct dt_d_s dur)
{
	dur.neg = (uint16_t)(~dur.neg & 0x01);
	switch (dur.typ) {
	case DT_DAISY:
		dur.daisydur = -dur.daisydur;
		break;
	case DT_BIZSI:
		dur.bizsidur = -dur.bizsidur;
		break;
	default:
		break;
	}
	return dur;
}

DEFUN int
dt_dur_neg_p(struct dt_d_s dur)
{
	switch (dur.typ) {
	case DT_DAISY:
		return dur.daisydur < 0;
	case DT_BIZSI:
		return dur.bizsidur < 0;
	default:
		return dur.neg;
	}
}

DEFUN struct dt_d_s
dt_ddiff(dt_dtyp_t tgttyp, struct dt_d_s d1, struct dt_d_s d2)
{
	struct dt_d_s res = {.typ = DT_DUNK};
	dt_dtyp_t tmptyp = tgttyp;

	if (tgttyp == DT_MD) {
		if (d1.typ == DT_YMD || d2.typ == DT_YMD) {
			tmptyp = DT_YMD;
		} else if (d1.typ == DT_YMCW || d2.typ == DT_YMCW) {
			tmptyp = DT_YMCW;
		} else {
			tmptyp = DT_DAISY;
		}
	}

	switch (tmptyp) {
	case DT_BIZSI:
	case DT_DAISY: {
		dt_daisy_t tmp1 = dt_conv_to_daisy(d1);
		dt_daisy_t tmp2 = dt_conv_to_daisy(d2);
		res = __daisy_diff(tmp1, tmp2);

		/* fix up result in case it's bizsi, i.e. kick weekends */
		if (tgttyp == DT_BIZSI) {
			dt_dow_t wdb = __daisy_get_wday(tmp2);
			res.bizsidur = __get_nbdays(res.daisy, wdb);
		}
		break;
	}
	case DT_YMD: {
		dt_ymd_t tmp1 = dt_conv_to_ymd(d1);
		dt_ymd_t tmp2 = dt_conv_to_ymd(d2);
		res = __ymd_diff(tmp1, tmp2);
		break;
	}
	case DT_YMCW: {
		dt_ymcw_t tmp1 = dt_conv_to_ymcw(d1);
		dt_ymcw_t tmp2 = dt_conv_to_ymcw(d2);
		res = __ymcw_diff(tmp1, tmp2);
		break;
	}
	case DT_BIZDA:
	case DT_DUNK:
	default:
		res.typ = DT_DUNK;
		res.u = 0;
		/* @fallthrough@ */
	case DT_MD:
		/* md is handled later */
		break;
	}
	/* check if we had DT_MD as tgttyp */
	if (tgttyp == DT_MD) {
		/* convert res back to DT_MD */
		struct dt_d_s tmp = {.typ = DT_MD};

		switch (tmptyp) {
		case DT_YMD:
			tmp.md.m = res.ymd.y * GREG_MONTHS_P_YEAR + res.ymd.m;
			tmp.md.d = res.ymd.d;
			break;
		case DT_YMCW:
			tmp.md.m = res.ymcw.y * GREG_MONTHS_P_YEAR + res.ymcw.m;
			tmp.md.d = res.ymcw.w * GREG_DAYS_P_WEEK + res.ymcw.c;
			break;
		case DT_DAISY:
			tmp.md.m = 0;
			tmp.md.d = res.daisy;
			break;
		default:
			break;
		}
		res = tmp;
	}
	return res;
}

DEFUN int
dt_dcmp(struct dt_d_s d1, struct dt_d_s d2)
{
/* for the moment D1 and D2 have to be of the same type. */
	if (UNLIKELY(d1.typ != d2.typ)) {
		/* always the left one */
		return -2;
	}
	switch (d1.typ) {
	case DT_DUNK:
	default:
		return -2;
	case DT_YMD:
	case DT_DAISY:
	case DT_BIZDA:
		/* use arithmetic comparison */
		if (d1.u == d2.u) {
			return 0;
		} else if (d1.u < d2.u) {
			return -1;
		} else /*if (d1.u > d2.u)*/ {
			return 1;
		}
	case DT_YMCW:
		/* use designated thing since ymcw dates aren't
		 * increasing */
		return __ymcw_cmp(d1.ymcw, d2.ymcw);
	}
}

DEFUN int
dt_d_in_range_p(struct dt_d_s d, struct dt_d_s d1, struct dt_d_s d2)
{
	return dt_dcmp(d, d1) >= 0 && dt_dcmp(d, d2) <= 0;
}

#if defined __INTEL_COMPILER
# pragma warning (default:2203)
#elif defined __GNUC__
# pragma GCC diagnostic warning "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

#endif	/* INCLUDED_date_core_c_ */
/* date-core.c ends here */
