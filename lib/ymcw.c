/*** ymcw.c -- guts for ymcw dates
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
#define ASPECT_YMCW


#if !defined YMCW_ASPECT_HELPERS_
#define YMCW_ASPECT_HELPERS_

static dt_ymcw_t
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
#else
	dt_ymd_t res;
	res.y = d.y;
	res.m = d.m;
	res.d = md;
	return res;
#endif
}
#endif	/* ASPECT_CONV */


#if defined ASPECT_ADD && !defined YMCW_ASPECT_ADD_
#define YMCW_ASPECT_ADD_

static dt_ymcw_t
__fixup_c(unsigned int y, signed int m, signed int c, dt_dow_t w)
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

	return __fixup_c(d.y, d.m, tgtc, (dt_dow_t)d.w);
}

static dt_ymcw_t
__ymcw_add_d(dt_ymcw_t d, int n)
{
/* add N days to D
 * we reduce this to __ymcw_add_w() */
	signed int aw = n / 7;
	signed int ad = n % 7;

	if ((ad += d.w) >= (signed int)GREG_DAYS_P_WEEK) {
		ad -= GREG_DAYS_P_WEEK;
		aw++;
	} else if (ad < 0) {
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
__ymcw_add_b(dt_ymcw_t d, int UNUSED(n))
{
/* add N business days to D */
	return d;
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
	return __ymcw_fixup(d);
}

static __attribute__((unused)) dt_ymcw_t
__ymcw_add_y(dt_ymcw_t d, int n)
{
/* add N years to D */
	d.y += n;
	return d;
}
#endif	/* ASPECT_ADD */


#if defined ASPECT_DIFF && !defined YMCW_ASPECT_DIFF_
#define YMCW_ASPECT_DIFF_

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
#endif	/* ASPECT_DIFF */


#if defined ASPECT_STRF && !defined YMCW_ASPECT_STRF_
#define YMCW_ASPECT_STRF_

#endif	/* ASPECT_STRF */

/* ymcw.c ends here */
