/*** daisy.c -- guts for daisy dates
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
#define ASPECT_DAISY

#include "nifty.h"


#if !defined DAISY_ASPECT_HELPERS_
#define DAISY_ASPECT_HELPERS_

static inline __attribute__((pure)) dt_daisy_t
__jan00_daisy(unsigned int year)
{
/* daisy's base year is both 1 mod 4 and starts on a monday, so ... */
#define TO_BASE(x)	((x) - DT_DAISY_BASE_YEAR)
#define TO_YEAR(x)	((x) + DT_DAISY_BASE_YEAR)
	unsigned int by = TO_BASE(year);

#if defined WITH_FAST_ARITH
	return by * 365U + by / 4U;
#else  /* !WITH_FAST_ARITH */
	by = by * 365U + by / 4U;
	if (UNLIKELY(year > 2100U)) {
		by -= (year - 2000U) / 100U;
		by += (year - 2000U) / 400U;
	}
	return by;
#endif	/* WITH_FAST_ARITH */
}
#endif	/* DAISY_ASPECT_HELPERS_ */


#if defined ASPECT_GETTERS && !defined DAISY_ASPECT_GETTERS_
#define DAISY_ASPECT_GETTERS_

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
#endif	/* DAISY_ASPECT_GETTERS_ */


#if defined ASPECT_CONV && !defined DAISY_ASPECT_CONV_
#define DAISY_ASPECT_CONV_

static dt_ymd_t
__daisy_to_ymd(dt_daisy_t that)
{
	dt_daisy_t j00;
	unsigned int doy;
	unsigned int y;
	struct __md_s md;

	if (UNLIKELY(that == 0)) {
		return (dt_ymd_t){.u = 0};
	}
	y = __daisy_get_year(that);
	j00 = __jan00_daisy(y);
	doy = that - j00;
	md = __yday_get_md(y, doy);
#if defined HAVE_ANON_STRUCTS_INIT
	return (dt_ymd_t){.y = y, .m = md.m, .d = md.d};
#else  /* !HAVE_ANON_STRUCTS_INIT */
	{
		dt_ymd_t res;
		res.y = y;
		res.m = md.m;
		res.d = md.d;
		return res;
	}
#endif	/* HAVE_ANON_STRUCTS_INIT */
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
#if defined HAVE_ANON_STRUCTS_INIT
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
#endif	/* ASPECT_CONV */


#if defined ASPECT_ADD && !defined DAISY_ASPECT_ADD_
#define DAISY_ASPECT_ADD_
#define ASPECT_GETTERS
#include "daisy.c"
#undef ASPECT_GETTERS

static dt_daisy_t
__daisy_add_d(dt_daisy_t d, int n)
{
/* add N days to D */
	d += n;
	return d;
}

static dt_daisy_t
__daisy_add_b(dt_daisy_t d, int n)
{
/* add N business days to D */
	dt_dow_t dow = __daisy_get_wday(d);
	int equ = __get_d_equiv(dow, n);
	d += equ;
	return d;
}

static dt_daisy_t
__daisy_add_w(dt_daisy_t d, int n)
{
/* add N weeks to D */
	return __daisy_add_d(d, GREG_DAYS_P_WEEK * n);
}

static dt_daisy_t
__daisy_add_m(dt_daisy_t d, int UNUSED(n))
{
/* daisies have no notion of months, so do fuckall */
	return d;
}

static dt_daisy_t
__daisy_add_y(dt_daisy_t d, int UNUSED(n))
{
/* daisies have no notion of years, do fuckall */
	return d;
}
#endif	/* ASPECT_ADD */


#if defined ASPECT_DIFF && !defined DAISY_ASPECT_DIFF_
#define DAISY_ASPECT_DIFF_
static struct dt_d_s
__daisy_diff(dt_daisy_t d1, dt_daisy_t d2)
{
/* compute d2 - d1 */
	struct dt_d_s res = {.typ = DT_DAISY, .dur = 1};
	int32_t diff = d2 - d1;

	res.daisydur = diff;
	return res;
}
#endif	/* ASPECT_DIFF */


#if defined ASPECT_STRF && !defined DAISY_ASPECT_STRF_
#define DAISY_ASPECT_STRF_

static void
__prep_strfd_daisy(struct strpd_s *tgt, dt_daisy_t d)
{
	dt_ymd_t tmp = __daisy_to_ymd(d);

	tgt->y = tmp.y;
	tgt->m = tmp.m;
	tgt->d = tmp.d;
	return;
}
#endif	/* ASPECT_STRF */

/* daisy.c ends here */
