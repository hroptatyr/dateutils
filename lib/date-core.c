/*** date-core.c -- our universe of dates
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
#include "nifty.h"
/* parsers and formatters */
#include "date-core-strpf.h"

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

/* algo for jan01 wd determination */
#if defined GET_JAN01_WDAY_FULL_LOOKUP
#elif defined GET_JAN01_WDAY_28Y_LOOKUP
#elif defined GET_JAN01_WDAY_28Y_SWITCH
#elif defined GET_JAN01_WDAY_SAKAMOTO
#else
# define GET_JAN01_WDAY_28Y_LOOKUP
#endif


/* helpers */
#if defined GET_JAN01_WDAY_FULL_LOOKUP
# define M	(unsigned int)(DT_MONDAY)
# define T	(unsigned int)(DT_TUESDAY)
# define W	(unsigned int)(DT_WEDNESDAY)
# define R	(unsigned int)(DT_THURSDAY)
# define F	(unsigned int)(DT_FRIDAY)
# define A	(unsigned int)(DT_SATURDAY)
# define S	(unsigned int)(DT_SUNDAY)

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
# undef M
# undef T
# undef W
# undef R
# undef F
# undef A
# undef S
#endif	/* GET_JAN01_WDAY_FULL_LOOKUP */

#if 1
static uint16_t __mon_yday[] = {
/* this is \sum ml, first element is a bit set of leap days to add */
	0xfff8, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};
#endif

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
DEFVAR const char **dut_long_wday = __long_wday;
DEFVAR const size_t dut_nlong_wday = countof(__long_wday);

static const char *__abbr_wday[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Mir",
};
DEFVAR const char **dut_abbr_wday = __abbr_wday;
DEFVAR const size_t dut_nabbr_wday = countof(__abbr_wday);

static const char __abab_wday[] = "SMTWRFAX";
DEFVAR const char *dut_abab_wday = __abab_wday;
DEFVAR const size_t dut_nabab_wday = countof(__abab_wday);

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
DEFVAR const char **dut_long_mon = __long_mon;
DEFVAR const size_t dut_nlong_mon = countof(__long_mon);

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
DEFVAR const char **dut_abbr_mon = __abbr_mon;
DEFVAR const size_t dut_nabbr_mon = countof(__abbr_mon);

/* futures expiry codes, how convenient */
static const char __abab_mon[] = "_FGHJKMNQUVXZ";
DEFVAR const char *dut_abab_mon = __abab_mon;
DEFVAR const size_t dut_nabab_mon = countof(__abab_mon);

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

/* UTC has a constant day length */
#define UTC_SECS_PER_DAY	(86400)

static void
ffff_gmtime(struct tm *tm, const time_t t)
{
	register int days;
	register unsigned int yy;
	const uint16_t *ip;

	/* just go to day computation */
	days = (int)(t / UTC_SECS_PER_DAY);
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
#if defined GET_JAN01_WDAY_FULL_LOOKUP

static inline __attribute__((pure)) __jan01_wday_block_t
__get_jan01_block(unsigned int year)
{
	return __jan01_wday[(year - __JAN01_WDAY_BEG) / __JAN01_Y_PER_B];
}

static inline __attribute__((pure)) dt_dow_t
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

#elif defined GET_JAN01_WDAY_28Y_LOOKUP
# define M	(DT_MONDAY)
# define T	(DT_TUESDAY)
# define W	(DT_WEDNESDAY)
# define R	(DT_THURSDAY)
# define F	(DT_FRIDAY)
# define A	(DT_SATURDAY)
# define S	(DT_SUNDAY)

static const dt_dow_t __jan01_28y_wday[] = {
	/* 1904 - 1910 */
	F, S, M, T, W, F, A,
	/* 1911 - 1917 */
	S, M, W, R, F, A, M,
	/* 1918 - 1924 */
	T, W, R, A, S, M, T,
	/* 1925 - 1931 */
	R, F, A, S, T, W, R,
};

# undef M
# undef T
# undef W
# undef R
# undef F
# undef A
# undef S

static __attribute__((pure)) unsigned int
__get_28y_year_equiv(unsigned year)
{
/* the 28y cycle works for 1901 to 2100, for other years find an equivalent */
	year = year % 400U;

	if (year > 300U) {
		return year + 1600U;
	} else if (year > 200U) {
		return year + 1724U;
	} else if (year > 100U) {
		return year + 1820U;
	}
	return year + 2000;
}

static inline __attribute__((pure)) dt_dow_t
__get_jan01_wday(unsigned int year)
{
/* get the weekday of jan01 in YEAR
 * using the 28y cycle thats valid till the year 2399
 * 1920 = 16 mod 28 */
#if !defined WITH_FAST_ARITH
	if (UNLIKELY(year > 2100U)) {
		year = __get_28y_year_equiv(year);
	}
#endif	/* !WITH_FAST_ARITH */
	return __jan01_28y_wday[year % 28U];
}

#elif defined GET_JAN01_WDAY_28Y_SWITCH

static inline __attribute__((pure)) dt_dow_t
__get_jan01_wday(unsigned int year)
{
/* get the weekday of jan01 in YEAR
 * using the 28y cycle thats valid till the year 2399
 * 1920 = 16 mod 28
 * switch variant */
# define M	(DT_MONDAY)
# define T	(DT_TUESDAY)
# define W	(DT_WEDNESDAY)
# define R	(DT_THURSDAY)
# define F	(DT_FRIDAY)
# define A	(DT_SATURDAY)
# define S	(DT_SUNDAY)
	switch (year % 28) {
	case 0:
		return F;
	case 1:
		return S;
	case 2:
		return M;
	case 3:
		return T;
	case 4:
		return W;
	case 5:
		return F;
	case 6:
		return A;
	case 7:
		return S;
	case 8:
		return M;
	case 9:
		return W;
	case 10:
		return R;
	case 11:
		return F;
	case 12:
		return A;
	case 13:
		return M;
	case 14:
		return T;
	case 15:
		return W;
	case 16:
		return R;
	case 17:
		return A;
	case 18:
		return S;
	case 19:
		return M;
	case 20:
		return T;
	case 21:
		return R;
	case 22:
		return F;
	case 23:
		return A;
	case 24:
		return S;
	case 25:
		return T;
	case 26:
		return W;
	case 27:
		return R;
	default:
		return DT_MIRACLEDAY;
	}
# undef M
# undef T
# undef W
# undef R
# undef F
# undef A
# undef S
}

#elif defined GET_JAN01_WDAY_SAKAMOTO

static inline __attribute__((pure)) dt_dow_t
__get_jan01_wday(unsigned int year)
{
	unsigned int res;

	year--,
		res = year + year / 4 - year / 100 + year / 400 + 1;
	return (dt_dow_t)(res % GREG_DAYS_P_WEEK);
}

#endif	/* GET_JAN01_WDAY_* */

static inline __attribute__((pure)) unsigned int
__md_get_yday(unsigned int year, unsigned int mon, unsigned int dom)
{
#if 1
	return __mon_yday[mon] + dom + UNLIKELY(__leapp(year) && mon >= 3);
#else  /* !1 */
#define SL(x)	((x) * 5)
#define GET_REM(x)	((rem >> SL(x)) & 0x1f)
	static const uint64_t rem =
		(19ULL << SL(0)) |
		(18ULL << SL(1)) |
		(14ULL << SL(2)) |
		(13ULL << SL(3)) |
		(11ULL << SL(4)) |
		(10ULL << SL(5)) |
		(8ULL << SL(6)) |
		(7ULL << SL(7)) |
		(6ULL << SL(8)) |
		(4ULL << SL(9)) |
		(3ULL << SL(10)) |
		(1ULL << SL(11)) |
		(0ULL << SL(12));
	return (mon - 1) * 32 + GET_REM(mon - 1) - 19 + dom +
		UNLIKELY(__leapp(year) && mon >= 3);
#undef GET_REM
#undef SL
#endif	/* 1 */
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
	off = __md_get_yday(year, mon, 0);
	return (dt_dow_t)((cand + off) % GREG_DAYS_P_WEEK);
}

DEFUN __attribute__((pure)) inline unsigned int
__get_mdays(unsigned int y, unsigned int m)
{
/* get the number of days in Y-M */
	unsigned int res;

	if (UNLIKELY(m < 1 || m > GREG_MONTHS_P_YEAR)) {
		return 0;
	}

	/* use our cumulative yday array */
	res = __md_get_yday(y, m + 1, 0);
	return res - __md_get_yday(y, m, 0);
}

static __attribute__((pure)) unsigned int
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

struct __md_s {
	unsigned int m;
	unsigned int d;
};

#if 1
/* Freundt's 32-adic algo */
static struct __md_s
__yday_get_md(unsigned int year, unsigned int doy)
{
/* Given a year and the day of the year, return gregorian month + dom
 * we use some clever maths to invert __mon_yday[] above
 * you see __mon_yday[] + 19 divrem 32 is
 *   F   0  0 19
 *   G  31  1 18
 *   H  59  2 14
 *   J  90  3 13
 *   K 120  4 11
 *   M 151  5 10
 *   N 181  6  8
 *   Q 212  7  7
 *   U 243  8  6
 *   V 273  9  4
 *   X 304 10  3
 *   Z 334 11  1
 *     365 12  0
 *
 * Now the key is to store only the remainders in a clever way.
 *
 * So in total the table we store is 5bit remainders of
 * __mon_yday[] + 19 % 32 */
#define GET_REM(x)	(rem[x])
	static const uint8_t rem[] = {
		19, 19, 18, 14, 13, 11, 10, 8, 7, 6, 4, 3, 1, 0

	};
	unsigned int m;
	unsigned int d;
	unsigned int beef;
	unsigned int cake;

	/* get 32-adic doys */
	m = (doy + 19) / 32U;
	d = (doy + 19) % 32U;
	beef = GET_REM(m);
	cake = GET_REM(m + 1);

	/* put leap years into cake */
	if (UNLIKELY(__leapp(year) && cake < 16U)) {
		/* note how all leap-affected cakes are < 16 */
		beef += beef < 16U;
		cake++;
	}

	if (d <= cake) {
		d = doy - ((m - 1) * 32 - 19 + beef);
	} else {
		d = doy - (m++ * 32 - 19 + cake);
	}
	return (struct __md_s){.m = m, .d = d};
#undef GET_REM
}

#elif 0
/* Trimester algo, we can't figure out which one's faster */
static struct __md_s
__yday_get_md(unsigned int year, unsigned int yday)
{
/* The idea here is that the year can be divided into trimesters:
 * Mar  31   Aug  31   Jan 31
 * Apr  30   Sep  30   Feb 28/29
 * May  31   Oct  31
 * Jun  30   Nov  30
 * Jul  31   Dec  31
 *
 * The first two trimesters have 153 days each, and given one of those
 * the m/d calculation is simply:
 *   m,d <- yday divrem 30.5
 *
 * where 30.5 is achieved by:
 *   m,d <- 2 * yday divrem 61
 * of course.
 *
 * Normally given the trimester and the m,d within the trimester you
 * could assemble the m,d of a gregorian year as:
 *   m <- 5 * trimstr + m + 3
 *   d <- d
 * given 0-counting.  And wrap around months 13 and 14 to 01 and 02;
 * so from this the subtrahend in the 2nd trimester case should be
 * 153 (as opposed to -30 now).
 *
 * We employ a simpler version here that ``inserts'' 2 days after February,
 * yday 60 is 29 Feb, yday 61 is 30 Feb, then proceed as per usual until
 * the end of July where another (unnamed) 30-day month is inserted that
 * goes seamlessly with the 31,30,31,30... cycle */
	int m;
	int d;

	if ((yday -= 1 + __leapp(year)) < 59) {
		/* 3rd trimester */
		yday += __leapp(year);
	} else if ((yday += 2) < 153 + 61) {
		/* 1st trimester */
		;
	} else {
		/* 2nd trimester */
		yday += 30;
	}

	m = 2 * yday / 61;
	d = 2 * yday % 61;
	return (struct __md_s){.m = m + 1 - (m >= 7), .d = d / 2 + 1};
}
#endif	/* 0 */

/* helpers from the calendar files, don't define any aspect, so only
 * the helpers should get included */
#include "ymd.c"
#include "ymcw.c"
#include "ywd.c"
#include "bizda.c"
#include "daisy.c"


#define ASPECT_GETTERS
#include "ymd.c"
#include "ymcw.c"
#include "ywd.c"
#include "bizda.c"
#include "daisy.c"
#undef ASPECT_GETTERS

#define ASPECT_CONV
#include "ymd.c"
#include "ymcw.c"
#include "ywd.c"
#include "bizda.c"
#include "daisy.c"
#undef ASPECT_CONV

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
	case DT_YWD:
		return __ywd_get_mon(that.ywd);
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
	case DT_YWD:
		return __ywd_get_wday(that.ywd);
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

static struct __md_s
dt_get_md(struct dt_d_s that)
{
	switch (that.typ) {
	default:
		return (struct __md_s){.m = 0, .d = 0};
	case DT_YMD:
		return (struct __md_s){.m = that.ymd.m, .d = that.ymd.d};
	case DT_YMCW: {
		unsigned int d = __ymcw_get_mday(that.ymcw);
		return (struct __md_s){.m = that.ymcw.m, .d = d};
	}
	case DT_YWD:
		/* should have come throught the GETTERS aspect */
		return __ywd_get_md(that.ywd);
	}
}

/* too exotic to be public */
static int
dt_get_wcnt_mon(struct dt_d_s that)
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
	case DT_YWD:
		return __ywd_get_wcnt_mon(that.ywd);
	default:
	case DT_DUNK:
		return 0;
	}
}

static int
dt_get_wcnt_year(struct dt_d_s this, unsigned int wkcnt_convention)
{
	int res;

	switch (this.typ) {
	case DT_YMD:
		switch (wkcnt_convention) {
		default:
		case YWD_ABSWK_CNT:
			res = __ymd_get_wcnt_abs(this.ymd);
			break;
		case YWD_ISOWK_CNT:
			res = __ymd_get_wcnt_iso(this.ymd);
			break;
		case YWD_MONWK_CNT:
		case YWD_SUNWK_CNT: {
			/* using monwk_cnt is a minor trick
			 * from = 1 = Mon or 0 = Sun */
			int from = wkcnt_convention == YWD_MONWK_CNT;
			res = __ymd_get_wcnt(this.ymd, from);
			break;
		}
		}
		break;
	case DT_YMCW:
		res = __ymcw_get_yday(this.ymcw);
		break;
	case DT_YWD:
		res = __ywd_get_wcnt_year(this.ywd, wkcnt_convention);
		break;
	default:
		res = 0;
		break;
	}
	return res;
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
	case DT_YWD:
		return __ywd_get_yday(that.ywd);
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
	switch (that.typ) {
	case DT_DAISY:
		return that.daisy;
	case DT_YMD:
		return __ymd_to_daisy(that.ymd);
	case DT_YMCW:
		return __ymcw_to_daisy(that.ymcw);
	case DT_YWD:
	case DT_BIZDA:
	case DT_DUNK:
	default:
		break;
	}
	return (dt_daisy_t)0;
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
		return __bizda_to_ymd(that.bizda);
	case DT_YWD:
		return __ywd_to_ymd(that.ywd);
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
	case DT_YWD:
		return __ywd_to_ymcw(that.ywd);
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

static dt_ywd_t
dt_conv_to_ywd(struct dt_d_s this)
{
	switch (this.typ) {
	case DT_YWD:
		/* yay, that was quick */
		return this.ywd;
	case DT_YMD:
		return __ymd_to_ywd(this.ymd);
	case DT_YMCW:
		return __ymcw_to_ywd(this.ymcw);
	case DT_DAISY:
		return __daisy_to_ywd(this.daisy);
	case DT_BIZDA:
	case DT_DUNK:
	default:
		break;
	}
	return (dt_ywd_t){.u = 0};
}


/* arithmetic */
#define ASPECT_ADD
#include "ymd.c"
#include "ymcw.c"
#include "ywd.c"
#include "bizda.c"
#include "daisy.c"
#undef ASPECT_ADD

#define ASPECT_DIFF
#include "ymd.c"
#include "ymcw.c"
#include "ywd.c"
#include "bizda.c"
#include "daisy.c"
#undef ASPECT_DIFF


/* guessing parsers */
#include "token.c"
#include "strops.c"
#include "date-core-strpf.c"

#if defined HAVE_GPERF
#include "fmt-special.c"
#endif	/* HAVE_GPERF */

static const char ymd_dflt[] = "%F";
static const char ymcw_dflt[] = "%Y-%m-%c-%w";
static const char ywd_dflt[] = "%rY-W%V-%u";
static const char daisy_dflt[] = "%d";
static const char bizsi_dflt[] = "%db";
static const char bizda_dflt[] = "%Y-%m-%db";

static dt_dtyp_t
__trans_dfmt_special(const char *fmt)
{
#if defined HAVE_GPERF
	size_t len = strlen(fmt);
	const struct dt_fmt_special_s *res;

	if (UNLIKELY((res = __fmt_special(fmt, len)) != NULL)) {
		return res->e;
	}
#else  /* !HAVE_GPERF */
	if (0) {
		;
	} else if (strcasecmp(fmt, "ymd") == 0) {
		return DT_YMD;
	} else if (strcasecmp(fmt, "ymcw") == 0) {
		return DT_YMCW;
	} else if (strcasecmp(fmt, "bizda") == 0) {
		return DT_BIZDA;
	} else if (strcasecmp(fmt, "ywd") == 0) {
		return DT_YWD;
	} else if (strcasecmp(fmt, "daisy") == 0) {
		return DT_DAISY;
# if defined DT_SEXY_BASE_YEAR
	} else if (strcasecmp(fmt, "sexy") == 0) {
		return (dt_dtyp_t)DT_SEXY;
# endif	 /* DT_SEXY_BASE_YEAR */
	} else if (strcasecmp(fmt, "bizsi") == 0) {
		return DT_BIZSI;
	}
#endif	/* HAVE_GPERF */
	return DT_DUNK;
}

DEFUN void
__trans_dfmt(const char **fmt)
{
	if (UNLIKELY(*fmt == NULL)) {
		/* great, standing ovations to the user */
		;
	} else if (LIKELY(**fmt == '%')) {
		/* don't worry about it */
		;
	} else {
		switch (__trans_dfmt_special(*fmt)) {
		default:
			break;
		case DT_YMD:
			*fmt = ymd_dflt;
			break;
		case DT_YMCW:
			*fmt = ymcw_dflt;
			break;
		case DT_YWD:
			*fmt = ywd_dflt;
			break;
		case DT_BIZDA:
			*fmt = bizda_dflt;
			break;
		case DT_DAISY:
			*fmt = daisy_dflt;
			break;
		case DT_BIZSI:
			*fmt = bizsi_dflt;
			break;
		}
	}
	return;
}

/* strpf glue */
DEFUN struct dt_d_s
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
	if (UNLIKELY(d.c == -1U)) {
		d.c = 0;
	}

	if (LIKELY(d.y && d.c == 0 && !d.flags.c_wcnt_p && !d.flags.bizda)) {
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
	} else if (d.y && d.m == 0 && !d.flags.bizda) {
		res.typ = DT_YWD;
		res.ywd = __make_ywd(d.y, d.c, d.w, d.flags.wk_cnt);
	} else if (d.y && !d.flags.bizda) {
		/* its legit for d.w to be naught */
		res.typ = DT_YMCW;
		res.ymcw.y = d.y;
		res.ymcw.m = d.m;
#if defined WITH_FAST_ARITH
		res.ymcw.c = d.c;
#else  /* !WITH_FAST_ARITH */
		if ((res.ymcw.c = d.c) >= 5) {
			/* the user meant the LAST wday actually */
			res.ymcw.c = __get_mcnt(d.y, d.m, (dt_dow_t)d.w);
		}
#endif	/* WITH_FAST_ARITH */
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


/* parser implementations */
#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

DEFUN struct dt_d_s
dt_strpd(const char *str, const char *fmt, char **ep)
{
	struct dt_d_s res = dt_d_initialiser();
	struct strpd_s d = strpd_initialiser();
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
	if (ep != NULL) {
		*ep = (char*)sp;
	}
	return res;
}

#define ASPECT_STRF
#include "ymd.c"
#include "ymcw.c"
#include "ywd.c"
#include "bizda.c"
#include "daisy.c"
#undef ASPECT_STRF

DEFUN size_t
dt_strfd(char *restrict buf, size_t bsz, const char *fmt, struct dt_d_s that)
{
	struct strpd_s d = strpd_initialiser();
	const char *fp;
	char *bp;
	dt_dtyp_t tgttyp;
	int set_fmt = 0;

	if (UNLIKELY(buf == NULL || bsz == 0)) {
		return 0;
	}

	if (LIKELY(fmt == NULL)) {
		/* um, great */
		set_fmt = 1;
	} else if (LIKELY(*fmt == '%')) {
		/* don't worry about it */
		;
	} else if ((tgttyp = __trans_dfmt_special(fmt)) != DT_DUNK) {
		that = dt_dconv(tgttyp, that);
		set_fmt = 1;
	}

	if (set_fmt) {
		switch (that.typ) {
		case DT_YMD:
			fmt = ymd_dflt;
			break;
		case DT_YMCW:
			fmt = ymcw_dflt;
			break;
		case DT_YWD:
			fmt = ywd_dflt;
			break;
		case DT_DAISY:
			/* subject to change */
			fmt = ymd_dflt;
			break;
		case DT_BIZDA:
			fmt = bizda_dflt;
			break;
		default:
			/* fuck */
			abort();
			break;
		}
	}

	switch (that.typ) {
	case DT_YMD:
		d.y = that.ymd.y;
		d.m = that.ymd.m;
		d.d = that.ymd.d;
		break;
	case DT_YMCW:
		d.y = that.ymcw.y;
		d.m = that.ymcw.m;
		d.c = that.ymcw.c;
		d.w = that.ymcw.w;
		break;
	case DT_DAISY:
		__prep_strfd_daisy(&d, that.daisy);
		break;
	case DT_BIZDA:
		__prep_strfd_bizda(&d, that.bizda, __get_bizda_param(that));
		break;
	case DT_YWD:
		__prep_strfd_ywd(&d, that.ywd);
		break;
	default:
	case DT_DUNK:
		bp = buf;
		goto out;
	}

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
	struct strpd_s d = strpd_initialiser();

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
	if (ep != NULL) {
		*ep = (char*)sp;
	}
	return res;
}

DEFUN size_t
dt_strfddur(char *restrict buf, size_t bsz, const char *fmt, struct dt_d_s that)
{
	struct strpd_s d = strpd_initialiser();
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

#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (default:2203)
#elif defined __GNUC__
# pragma GCC diagnostic warning "-Wcast-qual"
#endif	/* __INTEL_COMPILER */


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
#if defined HAVE_ANON_STRUCTS_INIT
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
dt_dconv(dt_dtyp_t tgttyp, struct dt_d_s d)
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
	case DT_YWD:
		res.ywd = dt_conv_to_ywd(d);
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
	struct strpdi_s durcch = strpdi_initialiser();

	__fill_strpdi(&durcch, dur);

	switch (d.typ) {
	case DT_DAISY:
		if (durcch.d) {
			d.daisy = __daisy_add_d(d.daisy, durcch.d);
		} else if (durcch.b) {
			d.daisy = __daisy_add_b(d.daisy, durcch.b);
		}
		if (durcch.w) {
			d.daisy = __daisy_add_w(d.daisy, durcch.w);
		}
		if (durcch.m) {
			d.daisy = __daisy_add_m(d.daisy, durcch.m);
		}
		if (durcch.y) {
			d.daisy = __daisy_add_y(d.daisy, durcch.y);
		}
		break;

	case DT_YMD:
		if (durcch.d) {
			d.ymd = __ymd_add_d(d.ymd, durcch.d);
		} else if (durcch.b) {
			d.ymd = __ymd_add_b(d.ymd, durcch.b);
		}
		if (durcch.w) {
			d.ymd = __ymd_add_w(d.ymd, durcch.w);
		}
		if (durcch.m) {
			d.ymd = __ymd_add_m(d.ymd, durcch.m);
		}
		if (durcch.y) {
			d.ymd = __ymd_add_y(d.ymd, durcch.y);
		}
		break;

	case DT_YMCW:
		if (durcch.d) {
			d.ymcw = __ymcw_add_d(d.ymcw, durcch.d);
		} else if (durcch.b) {
			d.ymcw = __ymcw_add_b(d.ymcw, durcch.b);
		}
		if (durcch.w) {
			d.ymcw = __ymcw_add_w(d.ymcw, durcch.w);
		}
		if (durcch.m) {
			d.ymcw = __ymcw_add_m(d.ymcw, durcch.m);
		}
		if (durcch.y) {
			d.ymcw = __ymcw_add_y(d.ymcw, durcch.y);
		}
		break;

	case DT_BIZDA:
		if (durcch.d) {
			d.bizda = __bizda_add_d(d.bizda, durcch.d);
		} else if (durcch.b) {
			d.bizda = __bizda_add_b(d.bizda, durcch.b);
		}
		if (durcch.w) {
			d.bizda = __bizda_add_w(d.bizda, durcch.w);
		}
		if (durcch.m) {
			d.bizda = __bizda_add_m(d.bizda, durcch.m);
		}
		if (durcch.y) {
			d.bizda = __bizda_add_y(d.bizda, durcch.y);
		}
		break;

	case DT_YWD:
		if (durcch.d) {
			d.ywd = __ywd_add_d(d.ywd, durcch.d);
		} else if (durcch.b) {
			d.ywd = __ywd_add_b(d.ywd, durcch.b);
		}
		if (durcch.w) {
			d.ywd = __ywd_add_w(d.ywd, durcch.w);
		}
		if (durcch.m) {
			d.ywd = __ywd_add_m(d.ywd, durcch.m);
		}
		if (durcch.y) {
			d.ywd = __ywd_add_y(d.ywd, durcch.y);
		}
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
			res.bizsidur = __get_nbdays(res.daisydur, wdb);
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
	case DT_YWD:
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

		case DT_BIZDA:
		case DT_BIZSI:
		case DT_MD:
		case DT_DUNK:
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

#endif	/* INCLUDED_date_core_c_ */
/* date-core.c ends here */
