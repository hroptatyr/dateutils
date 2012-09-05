/*** ymd.c -- guts for ymd dates
 *
 * Copyright (C) 2010-2012 Sebastian Freundt
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
#define ASPECT_YMD

#include "nifty.h"


#if !defined YMD_ASPECT_HELPERS_
#define YMD_ASPECT_HELPERS_
static dt_ymd_t
__ymd_fixup(dt_ymd_t d)
{
/* given dates like 2012-02-32 this returns 2012-02-29 */
	int mdays;

	if (LIKELY(d.d <= 28)) {
		/* every month has 28 days in our range */
		;
	} else if (UNLIKELY(d.m == 0 || d.m > GREG_MONTHS_P_YEAR)) {
		;
	} else if (d.d > (mdays = __get_mdays(d.y, d.m))) {
		d.d = mdays;
	}
	return d;
}
#endif	/* YMD_ASPECT_HELPERS_ */


#if defined ASPECT_GETTERS && !defined YMD_ASPECT_GETTERS_
#define YMD_ASPECT_GETTERS_
static unsigned int
__ymd_get_yday(dt_ymd_t that)
{
	unsigned int res;

	if (UNLIKELY(that.y == 0 ||
		     that.m == 0 || that.m > GREG_MONTHS_P_YEAR)) {
		return 0;
	}
	/* process */
	res = __md_get_yday(that.y, that.m, that.d);
	return res;
}

#if 1
/* lookup version */
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

#elif 0
/* Zeller algorithm */
static dt_dow_t
__ymd_get_wday(dt_ymd_t that)
{
/* this is Zeller's method, but there's a problem when we use this for
 * the bizda calendar. */
	int ydm = that.m;
	int ydy = that.y;
	int ydd = that.d;
	int w;
	int c, x;
	int d, y;

	if ((ydm -= 2) <= 0) {
		ydm += 12;
		ydy--;
	}

	d = ydy / 100;
	c = ydy % 100;
	x = c / 4;
	y = d / 4;

	w = (13 * ydm - 1) / 5;
	return (dt_dow_t)((w + x + y + ydd + c - 2 * d) % GREG_DAYS_P_WEEK);
}
#elif 1
/* Sakamoto method */
static dt_dow_t
__ymd_get_wday(dt_ymd_t that)
{
	static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
	int y = that.y;
	int m = that.m;
	int d = that.d;
	int res;

	y -= m < 3;
	res = y + y / 4 - y / 100 + y / 400;
	res += t[m - 1] + d;
	return (dt_dow_t)(res % GREG_DAYS_P_WEEK);
}
#endif	/* 0 */

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

DEFUN int
__ymd_get_wcnt(dt_ymd_t d, int wdays_from)
{
	int yd = __ymd_get_yday(d);
	dt_dow_t y01 = __get_jan01_wday(d.y);
	int wk;

	/* yd of the FIRST week of the year */
	if ((wk = 8 - (int)y01 + wdays_from) > 7) {
		wk -= 7;
	}
	/* and now express yd as 7k + n relative to jan01 */
	return (yd - wk + 7) / 7;
}

DEFUN int
__ymd_get_wcnt_abs(dt_ymd_t d)
{
/* absolutely count the n-th occurrence of WD regardless what WD
 * the year started with */
	int yd = __ymd_get_yday(d);

	/* and now express yd as 7k + n relative to jan01 */
	return (yd - 1) / 7 + 1;
}

DEFUN int
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
	dt_dow_t y01dow = __get_jan01_wday(y);
	unsigned int y01 = (unsigned int)y01dow;
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
#endif	/* YMD_ASPECT_GETTERS_ */


#if defined ASPECT_CONV && !defined YMD_ASPECT_CONV_
#define YMD_ASPECT_CONV_
/* we need some getter stuff, so get it */
#define ASPECT_GETTERS
#include "ymd.c"
#undef ASPECT_GETTERS

static dt_ymcw_t
__ymd_to_ymcw(dt_ymd_t d)
{
	unsigned int c = __ymd_get_count(d);
	unsigned int w = __ymd_get_wday(d);
#if defined HAVE_ANON_STRUCTS_INIT
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
#endif	/* ASPECT_CONV */


#if defined ASPECT_ADD && !defined YMD_ASPECT_ADD_
#define YMD_ASPECT_ADD_
static dt_ymd_t
__fixup_d(unsigned int y, signed int m, signed int d)
{
	dt_ymd_t res = {0};

	if (LIKELY(d >= 1 && d <= 28)) {
		/* all months in our design range have at least 28 days */
		;
	} else if (d < 1) {
		int mdays;

		do {
			if (UNLIKELY(--m < 1)) {
				--y;
				m = GREG_MONTHS_P_YEAR;
			}
			mdays = __get_mdays(y, m);
			d += mdays;
		} while (d < 1);

	} else {
		int mdays;

		while (d > (mdays = __get_mdays(y, m))) {
			d -= mdays;
			if (UNLIKELY(++m > (signed int)GREG_MONTHS_P_YEAR)) {
				++y;
				m = 1;
			}
		}
	}

	res.y = y;
	res.m = m;
	res.d = d;
	return res;
}

static dt_ymd_t
__ymd_add_d(dt_ymd_t d, int n)
{
/* add N days to D */
	signed int tgtd = d.d + n;

	/* fixup the day */
	return __fixup_d(d.y, d.m, tgtd);
}

static dt_ymd_t
__ymd_add_b(dt_ymd_t d, int n)
{
/* add N business days to D */
	dt_dow_t wd = __ymd_get_wday(d);
	int tgtd = d.d + __get_d_equiv(wd, n);

	/* fixup the day, i.e. 2012-01-34 -> 2012-02-03 */
	return __fixup_d(d.y, d.m, tgtd);
}

static __attribute__((unused)) dt_ymd_t
__ymd_add_w(dt_ymd_t d, int n)
{
/* add N weeks to D */
	return __ymd_add_d(d, GREG_DAYS_P_WEEK * n);
}

static dt_ymd_t
__ymd_add_m(dt_ymd_t d, int n)
{
/* add N months to D */
	signed int tgtm = d.m + n;

	while (tgtm > (signed int)GREG_MONTHS_P_YEAR) {
		tgtm -= GREG_MONTHS_P_YEAR;
		++d.y;
	}
	while (tgtm < 1) {
		tgtm += GREG_MONTHS_P_YEAR;
		--d.y;
	}
	/* final assignment */
	d.m = tgtm;
	return __ymd_fixup(d);
}

static __attribute__((unused)) dt_ymd_t
__ymd_add_y(dt_ymd_t d, int n)
{
/* add N years to D */
	d.y += n;
	return d;
}
#endif	/* ASPECT_ADD */


#if defined ASPECT_DIFF && !defined YMD_ASPECT_DIFF_
#define YMD_ASPECT_DIFF_
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
#endif	/* ASPECT_DIFF */


#if defined ASPECT_STRF && !defined YMD_ASPECT_STRF_
#define YMD_ASPECT_STRF_

#endif	/* ASPECT_STRF */

/* ymd.c ends here */
