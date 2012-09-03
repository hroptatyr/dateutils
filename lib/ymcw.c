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
	struct strpdi_s durcch = strpdi_initialiser();

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
		/* @fallthrough@ */
	case DT_DAISY:
	case DT_BIZSI: {
		signed int q;
		signed int mc;

		switch (dur.typ) {
		case DT_DAISY:
		case DT_BIZSI:
			/* get the trivial bits */
			tgty = d.y;
			tgtm = d.m;
		default:
			break;
		}

		/* factorise 7d.c + d.w + durcch.d into 7q + p, 0 <= p < 7
		 * we need the fact that p cannot be negative further down */
		mc = (d.c - 1) * GREG_DAYS_P_WEEK + d.w + durcch.d;
		q = __uidiv(mc, GREG_DAYS_P_WEEK);
		{
			/* just so we don't mix enum types and ints */
			unsigned int tmp = __uimod(mc, GREG_DAYS_P_WEEK);
			/* final week day in tmp, so ass it */
			tgtw = (dt_dow_t)tmp;
		}

		/* fixup q */
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

		/* re-instantiate the count within the month */
		tgtc = q + 1;
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
