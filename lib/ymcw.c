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


#if defined ASPECT_STRF && !defined YMCW_ASPECT_STRF_
#define YMCW_ASPECT_STRF_

#endif	/* ASPECT_STRF */

/* ymcw.c ends here */
