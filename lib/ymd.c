/*** ymd.c -- guts for ymd dates
 *
 * Copyright (C) 2010-2022 Sebastian Freundt
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
/* set aspect temporarily */
#define ASPECT_YMD
/* permanent aspect, to be read as have we ever seen aspect_ymd */
#if !defined ASPECT_YMD_
#define ASPECT_YMD_
#endif	/* !ASPECT_YMD_ */

#include "nifty.h"

/* some algorithmic choices */
#if defined YMD_GET_WDAY_LOOKUP
#elif defined YMD_GET_WDAY_ZELLER
#elif defined YMD_GET_WDAY_SAKAMOTO
#elif defined YMD_GET_WDAY_7YM_ALL
#elif defined YMD_GET_WDAY_7YM_REP
#else
/* default algo */
# define YMD_GET_WDAY_LOOKUP
#endif

#if !defined DEFUN
# define DEFUN
#endif	/* !DEFUN */


#if !defined YMD_ASPECT_HELPERS_
#define YMD_ASPECT_HELPERS_
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

DEFUN __attribute__((pure)) dt_ymd_t
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

static dt_dow_t
__get_m01_wday(unsigned int year, unsigned int mon)
{
/* get the weekday of the first of MONTH in YEAR */
	unsigned int off;
	dt_dow_t cand;

	if (UNLIKELY(mon < 1 || mon > GREG_MONTHS_P_YEAR)) {
		return DT_MIRACLEDAY;
	}
	cand = __get_jan01_wday(year);
	off = __md_get_yday(year, mon, 0);
	off = (cand + off) % GREG_DAYS_P_WEEK;
	return (dt_dow_t)(off ?: DT_SUNDAY);
}

#if defined YMD_GET_WDAY_LOOKUP
/* lookup version */
static dt_dow_t
__get_dom_wday(unsigned int year, unsigned int mon, unsigned int dom)
{
	unsigned int yd;
	unsigned int j01_wd;

	if ((yd = __md_get_yday(year, mon, dom)) > 0 &&
	    (j01_wd = __get_jan01_wday(year)) != DT_MIRACLEDAY) {
		unsigned int wd = (yd - 1 + j01_wd) % GREG_DAYS_P_WEEK;
		return (dt_dow_t)(wd ?: DT_SUNDAY);
	}
	return DT_MIRACLEDAY;
}

#elif defined YMD_GET_WDAY_ZELLER
/* Zeller algorithm */
static dt_dow_t
__get_dom_wday(int year, int mon, int dom)
{
/* this is Zeller's method, but there's a problem when we use this for
 * the bizda calendar. */
	int w;
	int c, x;
	int d, y;
	unsigned int wd;

	if ((mon -= 2) <= 0) {
		mon += 12;
		year--;
	}

	d = year / 100;
	c = year % 100;
	x = c / 4;
	y = d / 4;

	w = (13 * mon - 1) / 5;
	wd = (w + x + y + dom + c - 2 * d) % GREG_DAYS_P_WEEK;
	return (dt_dow_t)(wd ?: DT_SUNDAY);
}
#elif defined YMD_GET_WDAY_SAKAMOTO
/* Sakamoto method */
static dt_dow_t
__get_dom_wday(int year, int mon, int dom)
{
	static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
	int res;
	unsigned int wd;

	year -= mon < 3;
	res = year + year / 4 - year / 100 + year / 400;
	res += t[mon - 1] + dom;
	wd = (unsigned int)res % GREG_DAYS_P_WEEK;
	return (dt_dow_t)(wd ?: DT_SUNDAY);
}

#elif defined YMD_GET_WDAY_7YM_ALL || defined YMD_GET_WDAY_7YM_REP
/* this one's work in progress
 * first observe the 7 patterns in months FG and H..Z and how leap years
 * correspond to ordinary years.
 *
 * Use:
 *   dseq 1917-01-01 1mo 2417-12-01 -f '%Y %m %_a' |           \
 *     awk 'BEGIN{FS=" "}                                      \
 *          {a[$1] = "" a[$1] "" ($2=="03" ? " " : "") "" $3;} \
 *          END{for (i in a) print(i "\t" a[i]);}' |           \
 *     sort | sort -k3 -k2 -u
 *
 * for instance to get this table (months FG and months H..Z):
 *   1924	TF ATRSTFMWAM
 *   1919	WA ATRSTFMWAM
 *   1940	MR FMWAMRSTFS
 *   1918	TF FMWAMRSTFS
 *   1926	FM MRATRSWFMW
 *   1920	RS MRATRSWFMW
 *   1917	MR RSTFSWAMRA
 *   1928	SW RSTFSWAMRA
 *   1925	RS SWFMWATRST
 *   1936	WA SWFMWATRST
 *   1921	AT TFSWFMRATR
 *   1932	FM TFSWFMRATR
 *   1944	AT WAMRATFSWF
 *   1922	SW WAMRATFSWF
 *
 * and note how leap years and ordinary years pair up.
 *
 * After a bit of number crunching (consider all years mod 28) we'll find:
 *           H..Z
 *  0 -> 6 -> 17 -> 23
 *  4 -> 10 -> 21 -> 27
 *  8 -> 14 -> 25 -> 3
 * 12 -> 18 -> 1 -> 7
 * 16 -> 22 -> 5 -> 11
 * 20 -> 26 -> 9 -> 15
 * 24 -> 2 -> 13 -> 19
 *
 * where obviously the FG years are a permutation (13 14 15 21 22 17 18)^-1
 * of the H..Z years.
 *
 * It's quite easy to see how this system forms an orbit through C28 via:
 *
 *   [0]C4 + 0 = [4]C4 + 1 = [1]C4 + 2 = [5]C4 + 3
 *   [1]C4 + 0 = [5]C4 + 1 = [2]C4 + 2 = [6]C4 + 3
 *   [2]C4 + 0 = [6]C4 + 1 = [3]C4 + 2 = [0]C4 + 3
 *   [3]C4 + 0 = [0]C4 + 1 = [4]C4 + 2 = [1]C4 + 3
 *   [4]C4 + 0 = [1]C4 + 1 = [5]C4 + 2 = [2]C4 + 3
 *   [5]C4 + 0 = [2]C4 + 1 = [6]C4 + 2 = [3]C4 + 3
 *   [6]C4 + 0 = [3]C4 + 1 = [0]C4 + 2 = [4]C4 + 3
 *
 * and so by decomposing a year mod 28 and into C7*C4 we can map it to the
 * weekday.
 *
 * Here's the algo:
 * input: year
 * output: idx, weekday series index for H..
 *
 * year <- year mod 28
 * x,y  <- decompose year as [x]C4 + y
 * idx  <- x - 0, if y = 0
 *         x - 4, if y = 1
 *         x - 1, if y = 2
 *         x - 5, if y = 3
 *
 * Proceed similar for FG series.
 * That's how the formulas below came into being.
 *
 * For the shortened version (REP for representant) you have observe
 * that from leap year to leap year the weekday difference for each
 * respective month is 5. */
static unsigned int
__get_widx_H(unsigned int y)
{
/* return the equivalence class number for H.. weekdays for year Y. */
	static unsigned int add[] = {0, 3, 6, 2};
	unsigned int idx, res;

	y = y % 28U;
	idx = y / 4U;
	res = y % 4U;
	return (idx + add[res]) % GREG_DAYS_P_WEEK;
}

static unsigned int
__get_widx_FG(unsigned int y)
{
/* return the equivalence class number for FG weekdays for year Y. */
	static unsigned int add[] = {0, 6, 2, 5};
	unsigned int idx, res;

	y = y % 28U;
	idx = y / 4U;
	res = y % 4U;
	return (idx + add[res]) % GREG_DAYS_P_WEEK;
}

#if !defined WITH_FAST_ARITH
static inline __attribute__((pure)) unsigned int
__get_28y_year_equiv_H(unsigned year)
{
/* like __get_28y_year_equiv() but for months H..Z
 * needed for 7YM algo */
	year = year % 400U;

	if (year >= 300U) {
		return year + 1600U;
	} else if (year >= 200U) {
		return year + 1724U;
	} else if (year >= 100U) {
		return year + 1820U;
	}
	return year + 2000;
}
#endif	/* !WITH_FAST_ARITH */

#if defined YMD_GET_WDAY_7YM_ALL
static inline __attribute__((pure)) unsigned int
__get_ser(unsigned int class, unsigned int mon)
{
# define M	(DT_MONDAY)
# define T	(DT_TUESDAY)
# define W	(DT_WEDNESDAY)
# define R	(DT_THURSDAY)
# define F	(DT_FRIDAY)
# define A	(DT_SATURDAY)
# define S	(DT_SUNDAY)
	static uint8_t ser[][12] = {
		{
			/* 1932 = [[0]C4 + 0]C28 */
			F, M,  T, F, S, W, F, M, R, A, T, R,
		}, {
			/* 1936 = [[1]C4 + 0]C28 */
			W, A,  S, W, F, M, W, A, T, R, S, T,
		}, {
			/* 1940 = [[2]C4 + 0]C28 */
			M, R,  F, M, W, A, M, R, S, T, F, S,
		}, {
			/* 1944 = [[3]C4 + 0]C28 */
			A, T,  W, A, M, R, A, T, F, S, W, F,
		}, {
			/* 1920 = [[4]C4 + 0]C28 */
			R, S,  M, R, A, T, R, S, W, F, M, W,
		}, {
			/* 1924 = [[5]C4 + 0]C28 */
			T, F,  A, T, R, S, T, F, M, W, A, M,
		}, {
			/* 1928 = [[6]C4 + 0]C28 */
			S, W,  R, S, T, F, S, W, A, M, R, A,
		},
	};
# undef M
# undef T
# undef W
# undef R
# undef F
# undef A
# undef S
	return ser[class][mon];
}
#elif defined YMD_GET_WDAY_7YM_REP
static inline __attribute__((pure)) unsigned int
__get_ser(unsigned int class, unsigned int mon)
{
# define M	(DT_MONDAY)
# define T	(DT_TUESDAY)
# define W	(DT_WEDNESDAY)
# define R	(DT_THURSDAY)
# define F	(DT_FRIDAY)
# define A	(DT_SATURDAY)
# define S	(DT_SUNDAY)
	static unsigned int ser[12] = {
		/* 1932 = [[0]C4 + 0]C28 */
		F, M,  T, F, S, W, F, M, R, A, T, R,
	};
# undef M
# undef T
# undef W
# undef R
# undef F
# undef A
# undef S
	return ser[mon] + 5 * class;
}
#endif	/* 7YM_ALL | 7YM_RES */

static dt_dow_t
__get_dom_wday(unsigned int year, unsigned int mon, unsigned int dom)
{
	unsigned int bm = mon - 1;
	unsigned int bd = dom - 1;
	unsigned int idx;
	unsigned int wd;

	if (bm < 2U) {
		/* FG */
#if defined WITH_FAST_ARITH
		unsigned int by = year;
#else  /* !WITH_FAST_ARITH */
		unsigned int by = __get_28y_year_equiv(year);
#endif	/* !WITH_FAST_ARITH */
		idx = __get_widx_FG(by);
	} else {
#if defined WITH_FAST_ARITH
		unsigned int by = year;
#else  /* !WITH_FAST_ARITH */
		unsigned int by = __get_28y_year_equiv_H(year);
#endif  /* !WITH_FAST_ARITH */
		idx = __get_widx_H(by);
	}

	wd = (__get_ser(idx, bm) + bd) % GREG_DAYS_P_WEEK;
	return (dt_dow_t)(wd ?: DT_SUNDAY);
}

#endif	/* 0 */

/* try to get helpers like __get_d_equiv() et al */
#include "bizda.c"
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

static dt_dow_t
__ymd_get_wday(dt_ymd_t that)
{
	return __get_dom_wday(that.y, that.m, that.d);
}

DEFUN unsigned int
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

static int
__ymd_get_wcnt_abs(dt_ymd_t d)
{
/* absolutely count the n-th occurrence of WD regardless what WD
 * the year started with
 * generally the path ymd->yd->get_wcnt_abs() is preferred */
	int yd = __ymd_get_yday(d);

	/* and now express yd as 7k + n relative to jan01 */
	return (yd - 1) / 7 + 1;
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

static dt_ywd_t
__ymd_to_ywd(dt_ymd_t d)
{
	dt_dow_t w = __ymd_get_wday(d);
	unsigned int c = __ymd_get_wcnt_abs(d);
	return __make_ywd_c(d.y, c, w, YWD_ABSWK_CNT);
}

static dt_daisy_t
__ymd_to_daisy(dt_ymd_t d)
{
	dt_daisy_t res;
#if 1
/* cassio neri eaf */
#define s	(82U)
#define K	(584693U + 146097U * s)
#define L	(400U * s)
	const unsigned int J = d.m <= 2;
	const unsigned int Y = (uint32_t)(d.y + L) - J;
	const unsigned int M = J ? d.m + 12U : d.m;
	const unsigned int D = d.d - 1U;
	const unsigned int C = Y / 100U;

	/* Rata die.*/
	const unsigned int y_star = 1461U * Y / 4U - C + C / 4U;
	const unsigned int m_star = (979U * M - 2919U) / 32U;
	res = y_star + m_star + D;

	/* shift */
	res -= K;
#undef s
#undef K
#undef L
#else
	unsigned int sy = d.y;
	unsigned int sm = d.m;
	unsigned int sd;

	if (UNLIKELY((signed int)TO_BASE(sy) < 0)) {
		return 0;
	}

#if !defined WITH_FAST_ARITH || defined OMIT_FIXUPS
	/* the non-fast arith has done the fixup already */
	sd = d.d;
#else  /* WITH_FAST_ARITH && !OMIT_FIXUPS */
	{
		unsigned int tmp = __get_mdays(sy, sm);
		if (UNLIKELY((sd = d.m) > tmp)) {
			sd = tmp;
		}
	}
#endif	/* !WITH_FAST_ARITH || OMIT_FIXUPS */

	res = __jan00_daisy(sy);
	res += __md_get_yday(sy, sm, sd);
#endif
	return res;
}

static dt_yd_t
__ymd_to_yd(dt_ymd_t d)
{
	int yd = __ymd_get_yday(d);
#if defined HAVE_ANON_STRUCTS_INIT
	return (dt_yd_t){.y = d.y, .d = yd};
#else
	dt_yd_t res;
	res.y = d.y;
	res.d = yd;
	return res;
#endif
}
#endif	/* ASPECT_CONV */


#if defined ASPECT_ADD && !defined YMD_ASPECT_ADD_
#define YMD_ASPECT_ADD_
static __attribute__((pure)) dt_ymd_t
__ymd_fixup_d(unsigned int y, signed int m, signed int d)
{
	dt_ymd_t res = {0};

	m += !m;
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
	signed int tgtd;
	d = __ymd_fixup(d);
	tgtd = d.d + n + (n < 0 && !d.d);

	/* fixup the day */
	return __ymd_fixup_d(d.y, d.m, tgtd);
}

static dt_ymd_t
__ymd_add_b(dt_ymd_t d, int n)
{
/* add N business days to D */
	dt_dow_t wd;
	signed int tgtd;

	d = __ymd_fixup(d);
	d.d ^= n < 0 && !d.d;
	wd = __ymd_get_wday(d);
	tgtd = d.d + __get_d_equiv(wd, n);

	/* fixup the day, i.e. 2012-01-34 -> 2012-02-03 */
	return __ymd_fixup_d(d.y, d.m, tgtd);
}

static dt_ymd_t
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
	d.m = tgtm + (n < 0 && !d.m);
	return d;
}

static dt_ymd_t
__ymd_add_y(dt_ymd_t d, int n)
{
/* add N years to D */
	d.y += n;
	return d;
}
#endif	/* ASPECT_ADD */


#if defined ASPECT_DIFF && !defined YMD_ASPECT_DIFF_
#define YMD_ASPECT_DIFF_
static struct dt_ddur_s
__ymd_diff(dt_ymd_t d1, dt_ymd_t d2)
{
/* compute d2 - d1 entirely in terms of ymd */
	struct dt_ddur_s res = dt_make_ddur(DT_DURYMD, 0);
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
	if ((tgtd = d2.d - d1.d) < 0 && tgtm != 0) {
		/* if tgtm is 0 it remains 0 and tgtd remains negative */
		/* get the target month's mdays */
		unsigned int d2m = d2.m;
		unsigned int d2y = d2.y;

		if (--d2m < 1) {
			d2m = GREG_MONTHS_P_YEAR;
			d2y--;
		}
		tgtm--;
		if (UNLIKELY((tgtd += __get_mdays(d2y, d2m)) < 0)) {
			/* super special case when d2m is feb and the
			 * difference is >= 28 or 29,
			 * unrolled because it cannot happen again */
			if (--d2m < 1) {
				d2m = GREG_MONTHS_P_YEAR;
				d2y--;
			}
			tgtm--;
			tgtd += __get_mdays(d2y, d2m);
		}
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

#undef ASPECT_YMD

/* ymd.c ends here */
