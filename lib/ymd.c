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


#if !defined YMD_ASPECT_HELPERS_
#define YMD_ASPECT_HELPERS_

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
__ymd_add(dt_ymd_t d, struct dt_d_s dur)
{
/* add DUR to D, doesn't check if DUR has the dur flag */
	unsigned int tgty = 0;
	unsigned int tgtm = 0;
	signed int tgtd = 0;
	struct strpdi_s durcch = strpdi_initialiser();

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
#endif	/* ASPECT_ADD */


#if defined ASPECT_STRF && !defined YMD_ASPECT_STRF_
#define YMD_ASPECT_STRF_

#endif	/* ASPECT_STRF */

/* ymd.c ends here */
