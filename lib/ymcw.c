/*** ymcw.c -- guts for ymcw dates
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
#define ASPECT_YMCW

#if !defined DEFUN
# define DEFUN
#endif	/* !DEFUN */


#if !defined YMCW_ASPECT_HELPERS_
#define YMCW_ASPECT_HELPERS_

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

DEFUN dt_ymcw_t
__ymcw_fixup(dt_ymcw_t d)
{
/* given dates like 2012-02-05-01 this returns 2012-02-04-01 */
	int mc;

	if (LIKELY(d.c <= 4)) {
		/* every month has 4 occurrences of any weekday */
		;
	} else if (d.m == 0 || d.w == DT_MIRACLEDAY) {
		/* um */
		;
	} else if (d.c > (mc = __get_mcnt(d.y, d.m, (dt_dow_t)d.w))) {
		d.c = mc;
	}
	return d;
}
#endif	/* YMCW_ASPECT_HELPERS_ */


#if defined ASPECT_GETTERS && !defined YMCW_ASPECT_GETTERS_
#define YMCW_ASPECT_GETTERS_
static dt_dow_t
__ymcw_get_wday(dt_ymcw_t that)
{
	return (dt_dow_t)that.w;
}

DEFUN unsigned int
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
 * so go back to the last W, and compute its number instead
 *
 * Here's the full schema:
 * For W+0
 * 5 4 4 5 4 4  5 4 4 5 4 5  non-leap
 * 5 4 4 5 4 4  5 4 5 4 4 5  leap  flip9
 *
 * For W+1
 * 5 4 4 4 5 4  5 4 4 5 4 4  non-leap
 * 5 4 4 5 4 4  5 4 4 5 4 5  leap  flip4  flip12
 *
 * For W+2
 * 5 4 4 4 5 4  4 5 4 5 4 4  non-leap
 * 5 4 4 4 5 4  5 4 4 5 4 4  leap  flip7
 *
 * For W+3
 * 4 4 5 4 5 4  4 5 4 4 5 4  non-leap
 * 4 5 4 4 5 4  4 5 4 5 4 4  leap  flip2  flip10
 *
 * For W+4
 * 4 4 5 4 4 5  4 5 4 4 5 4  non-leap
 * 4 4 5 4 5 4  4 5 4 4 5 4  leap  flip5
 *
 * For W+5
 * 4 4 5 4 4 5  4 4 5 4 4 5  non-leap
 * 4 4 5 4 4 5  4 5 4 4 5 4  leap  flip8  flip11
 *
 * For W+6
 * 4 4 4 5 4 4  5 4 5 4 4 5  non-leap
 * 4 4 5 4 4 5  4 4 5 4 4 5  leap  flip3  flip6
 *
 * flipN denotes which month in a leap year becomes 5 where the
 * month in the non-leap year equivalent has value 4.
 *
 * The flips for W+1 W+2, W+4, W+5, W+6 can be presented through
 * non-leap rules:
 *
 * 544544544545544544544545
 * 544544545445544544545445
 *
 * 544454544544544454544544
 * 544544544545544544544545 = non-leap W+0
 *
 * 544454454544544454454544
 * 544454544544544454544544 = non-leap W+1
 *
 * 445454454454445454454454
 * 454454454544454454454544
 *
 * 445445454454445445454454
 * 445454454454445454454454 = non-leap W+3
 *
 * 445445445445445445445445
 * 445445454454445445454454 = non-leap W+4
 *
 * 444544545445444544545445
 * 445445445445445445445445 = non-leap W+5
 */
	static uint8_t ycum[][12] = {
		{
			/* W+0 */
			0, 5, 9, 13, 18, 22, 26,  31, 35, 39, 44, 48, /*53*/
		}, {
			/* W+1 */
			0, 5, 9, 13, 17, 22, 26,  31, 35, 39, 44, 48, /*52*/
		}, {
			/* W+2 */
			0, 5, 9, 13, 17, 22, 26,  30, 35, 39, 44, 48, /*52*/
		}, {
			/* W+3 */
			0, 4, 8, 13, 17, 22, 26,  30, 35, 39, 43, 48, /*52*/
		}, {
			/* W+4 */
			0, 4, 8, 13, 17, 21, 26,  30, 35, 39, 43, 48, /*52*/
		}, {
			/* W+5 */
			0, 4, 8, 13, 17, 21, 26,  30, 34, 39, 43, 47, /*52*/
		}, {
			/* W+6 */
			0, 4, 8, 12, 17, 21, 25,  30, 34, 39, 43, 47, /*52*/
		}, {
			/* leap-year rule W+0 */
			0, 5, 9, 13, 18, 22, 26,  31, 35, 40, 44, 48, /*53*/
			/* leap-year rule W+3 = W+0 + 1mo + 4 */
		},
	};
	dt_dow_t j01w = __get_jan01_wday(that.y);
	dt_dow_t w = (dt_dow_t)(that.w ?: DT_THURSDAY);
	unsigned int diff = j01w <= w ? w - j01w : w + 7 - j01w;

	if (UNLIKELY(__leapp(that.y))) {
		switch (diff) {
		case 3:
			if (UNLIKELY(that.m < 2)) {
				return that.c;
			}
			return that.c + (ycum[7])[that.m - 2] + 4;
		case 0:
			return that.c + (ycum[7])[that.m - 1];
		default:
		case 1:
		case 2:
		case 4:
		case 5:
		case 6:
			diff--;
			break;
		}
	}
	return that.c + (ycum[diff])[that.m - 1];
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
#endif	/* YMCW_ASPECT_GETTERS_ */


#if defined ASPECT_CONV && !defined YMCW_ASPECT_CONV_
#define YMCW_ASPECT_CONV_
/* we need some getter stuff, so get it */
#define ASPECT_GETTERS
#include "ymcw.c"
#undef ASPECT_GETTERS

static dt_ymd_t
__ymcw_to_ymd(dt_ymcw_t d)
{
	unsigned int md = __ymcw_get_mday(d);
#if defined HAVE_ANON_STRUCTS_INIT
	return (dt_ymd_t){.y = d.y, .m = d.m, .d = md};
#else  /* !HAVE_ANON_STRUCTS_INIT */
	dt_ymd_t res;
	res.y = d.y;
	res.m = d.m;
	res.d = md;
	return res;
#endif	/* HAVE_ANON_STRUCTS_INIT */
}

static dt_ywd_t
__ymcw_to_ywd(dt_ymcw_t d)
{
	unsigned int y = d.y;
	dt_dow_t w = (dt_dow_t)d.w;
	unsigned int c = __ymcw_get_yday(d);
	return __make_ywd_c(y, c, w, YWD_ABSWK_CNT);
}

static dt_daisy_t
__ymcw_to_daisy(dt_ymcw_t d)
{
	dt_daisy_t res;
	unsigned int sy = d.y;
	unsigned int sm = d.m;
	unsigned int sd;

	if (UNLIKELY((signed int)TO_BASE(sy) < 0)) {
		return 0;
	}

	sd = __ymcw_get_mday(d);
	res = __jan00_daisy(sy);
	res += __md_get_yday(sy, sm, sd);
	return res;
}

static dt_yd_t
__ymcw_to_yd(dt_ymcw_t d)
{
	unsigned int sd = __ymcw_get_mday(d);
	unsigned int sm = d.m;
	unsigned int sy = d.y;

#if defined HAVE_ANON_STRUCTS_INIT
	return (dt_yd_t){.y = sy, .d = __md_get_yday(sy, sm, sd)};
#else
	dt_yd_t res;
	res.y = sy;
	res.d = __md_get_yday(sy, sm, sd);
	return res;
#endif
}
#endif	/* ASPECT_CONV */


#if defined ASPECT_ADD && !defined YMCW_ASPECT_ADD_
#define YMCW_ASPECT_ADD_

static dt_ymcw_t
__ymcw_fixup_c(unsigned int y, signed int m, signed int c, dt_dow_t w)
{
	dt_ymcw_t res = {0};

	/* fixup q */
	if (LIKELY(c >= 1 && c <= 4)) {
		/* all months in our design range have 4 occurrences of
		 * any weekday, so YAAAY*/
		;
	} else if (c < 1) {
		int mc;

		do {
			if (UNLIKELY(--m < 1)) {
				--y;
				m = GREG_MONTHS_P_YEAR;
			}
			mc = __get_mcnt(y, m, w);
			c += mc;
		} while (c < 1);
	} else {
		int mc;

		while (c > (mc = __get_mcnt(y, m, w))) {
			c -= mc;
			if (UNLIKELY(++m > (signed int)GREG_MONTHS_P_YEAR)) {
				++y;
				m = 1;
			}
		}
	}

	/* final assignment */
	res.y = y;
	res.m = m;
	res.c = c;
	res.w = w;
	return res;
}

static dt_ymcw_t
__ymcw_add_w(dt_ymcw_t d, int n)
{
/* add N weeks to D */
	signed int tgtc = d.c + n;

	return __ymcw_fixup_c(d.y, d.m, tgtc, (dt_dow_t)d.w);
}

static dt_ymcw_t
__ymcw_add_d(dt_ymcw_t d, int n)
{
/* add N days to D
 * we reduce this to __ymcw_add_w() */
	signed int aw = n / (signed int)GREG_DAYS_P_WEEK;
	signed int ad = n % (signed int)GREG_DAYS_P_WEEK;

	if ((ad += d.w) > (signed int)GREG_DAYS_P_WEEK) {
		ad -= GREG_DAYS_P_WEEK;
		aw++;
	} else if (ad <= 0) {
		ad += GREG_DAYS_P_WEEK;
		aw--;
	}

	/* fixup for abswk count, m01 may be any wd */
	{
		dt_dow_t m01 = __get_m01_wday(d.y, d.m);

		if ((dt_dow_t)d.w < m01 && (dt_dow_t)ad >= m01) {
			aw++;
		} else if ((dt_dow_t)d.w >= m01 && (dt_dow_t)ad < m01) {
			aw--;
		}
	}

	d.w = (dt_dow_t)ad;
	return __ymcw_add_w(d, aw);
}

static dt_ymcw_t
__ymcw_add_b(dt_ymcw_t d, int n)
{
/* add N business days to D */
#if 0
/* trivial trait, reduce to _add_d() problem and dispatch */
	dt_dow_t wd = __ymcw_get_wday(d);
	return __ymcw_add_d(d, __get_d_equiv(wd, n));
#else
	signed int aw = n / (signed int)DUWW_BDAYS_P_WEEK;
	signed int ad = n % (signed int)DUWW_BDAYS_P_WEEK;

	if ((ad += d.w) > (signed int)DUWW_BDAYS_P_WEEK) {
		ad -= DUWW_BDAYS_P_WEEK;
		aw++;
	} else if (ad <= 0) {
		ad += DUWW_BDAYS_P_WEEK;
		aw--;
	}

	/* fixup for abswk count, m01 may be any wd */
	{
		dt_dow_t m01 = __get_m01_wday(d.y, d.m);

		if ((dt_dow_t)d.w < m01 && (dt_dow_t)ad >= m01) {
			aw++;
		} else if ((dt_dow_t)d.w >= m01 && (dt_dow_t)ad < m01) {
			aw--;
		}
	}

	d.w = (dt_dow_t)ad;
	return __ymcw_add_w(d, aw);
#endif
}

static dt_ymcw_t
__ymcw_add_m(dt_ymcw_t d, int n)
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
	return d;
}

static dt_ymcw_t
__ymcw_add_y(dt_ymcw_t d, int n)
{
/* add N years to D */
	d.y += n;
	return d;
}
#endif	/* ASPECT_ADD */


#if defined ASPECT_DIFF && !defined YMCW_ASPECT_DIFF_
#define YMCW_ASPECT_DIFF_

static struct dt_ddur_s
__ymcw_diff(dt_ymcw_t d1, dt_ymcw_t d2)
{
/* compute d2 - d1 entirely in terms of ymd */
	struct dt_ddur_s res = dt_make_ddur(DT_DURYMCW, 0);
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
#endif	/* ASPECT_DIFF */


#if defined ASPECT_CMP && !defined YMCW_ASPECT_CMP_
#define YMCW_ASPECT_CMP_
DEFUN int
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
#endif	/* ASPECT_CMP */


#if defined ASPECT_STRF && !defined YMCW_ASPECT_STRF_
#define YMCW_ASPECT_STRF_

#endif	/* ASPECT_STRF */

/* ymcw.c ends here */
