/*** daisy.c -- guts for daisy dates
 *
 * Copyright (C) 2010-2015 Sebastian Freundt
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

#if !defined DEFUN
# define DEFUN
#endif	/* !DEFUN */


#if !defined DAISY_ASPECT_HELPERS_
#define DAISY_ASPECT_HELPERS_
#define TO_BASE(x)	((x) - DT_DAISY_BASE_YEAR)
#define TO_YEAR(x)	((x) + DT_DAISY_BASE_YEAR)

static inline __attribute__((const, pure)) dt_daisy_t
__jan00_daisy(unsigned int year)
{
/* daisy's base year is both 1 mod 4 and starts on a monday, so ... */
	unsigned int by = TO_BASE(year);

#if defined WITH_FAST_ARITH
	return by * 365U + by / 4U;
#else  /* !WITH_FAST_ARITH */
	by = by * 365U + by / 4U;
#if DT_DAISY_BASE_YEAR == 1917
	if (UNLIKELY(year > 2100U)) {
		by -= (year - 2001U) / 100U;
		by += (year - 2001U) / 400U;
	}
#elif DT_DAISY_BASE_YEAR == 1753
	if (LIKELY(year > 1800U)) {
		by -= (year - 1701U) / 100U;
		by += (year - 1601U) / 400U;
	}
#elif DT_DAISY_BASE_YEAR == 1601
	by -= (year - 1601U) / 100U;
	by += (year - 1601U) / 400U;
#endif
	return by;
#endif	/* WITH_FAST_ARITH */
}
#endif	/* DAISY_ASPECT_HELPERS_ */


#if defined ASPECT_GETTERS && !defined DAISY_ASPECT_GETTERS_
#define DAISY_ASPECT_GETTERS_

static __attribute__((const, pure)) dt_dow_t
__daisy_get_wday(dt_daisy_t d)
{
/* daisy wdays are simple because the base year is chosen so that day 0
 * in the daisy calendar is a sunday */
	return (dt_dow_t)((d % GREG_DAYS_P_WEEK) ?: DT_SUNDAY);
}

static __attribute__((const, pure)) unsigned int
__daisy_get_year(dt_daisy_t d)
{
/* given days since 1917-01-01 (Mon), compute a year */
	unsigned int by;

	if (UNLIKELY(d == 0)) {
		return 0;
	}
	/* get an estimate for the year and readjust */
	by = d / 365U;
	if (UNLIKELY(__jan00_daisy(TO_YEAR(by)) >= d)) {
		by--;
#if !defined WITH_FAST_ARITH
		if (UNLIKELY(__jan00_daisy(TO_YEAR(by)) >= d)) {
			by--;
		}
#endif	/* WITH_FAST_ARITH */
	}
	return TO_YEAR(by);
}

static __attribute__((const, pure)) unsigned int
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

#if DT_DAISY_BASE_YEAR == 1917
# define DT_LDN_BASE	(122068U/*lilian's 1917-01-00*/)
# define DT_JDN_BASE	(2421228.5f/*julian's 1917-01-00*/)
#elif DT_DAISY_BASE_YEAR == 1753
# define DT_LDN_BASE	(62169U/*lilian's 1753-01-00*/)
# define DT_JDN_BASE	(2361329.5f/*julian's 1753-01-00*/)
#elif DT_DAISY_BASE_YEAR == 1601
# define DT_LDN_BASE	(6652U/*lilian's 1601-01-00*/)
# define DT_JDN_BASE	(2305812.5f/*julian's 1601-01-00*/)
#else
# error cannot convert to ldn, unknown base year
#endif

static __attribute__((const, pure)) dt_ldn_t
__daisy_to_ldn(dt_daisy_t d)
{
	return d + DT_LDN_BASE;
}

static __attribute__((const, pure)) dt_jdn_t
__daisy_to_jdn(dt_daisy_t d)
{
	return (dt_jdn_t)d + DT_JDN_BASE;
}

static __attribute__((const, pure)) dt_daisy_t
__ldn_to_daisy(dt_ldn_t d)
{
	dt_sdaisy_t tmp;

	if ((tmp = d - DT_LDN_BASE) > 0) {
		return (dt_daisy_t)tmp;
	}
	return 0U;
}

static __attribute__((const, pure)) dt_daisy_t
__jdn_to_daisy(dt_jdn_t d)
{
	float tmp;
	if ((tmp = d - DT_JDN_BASE) > 0.0f) {
		return (dt_daisy_t)tmp;
	}
	return 0U;
}

DEFUN __attribute__((const, pure)) dt_ymd_t
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

static __attribute__((const, pure)) dt_ymcw_t
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

static __attribute__((const, pure)) dt_ywd_t
__daisy_to_ywd(dt_daisy_t that)
{
	const unsigned int wd = (that + GREG_DAYS_P_WEEK - 1) % GREG_DAYS_P_WEEK;
	dt_dow_t dow = (dt_dow_t)(wd + 1U);
	unsigned int y = __daisy_get_year(that);
	int yd = that - __jan00_daisy(y);

	return __make_ywd_yd_dow(y, yd, dow);
}

static __attribute__((const, pure)) dt_yd_t
__daisy_to_yd(dt_daisy_t d)
{
	int yd = __daisy_get_yday(d);
	unsigned int y = __daisy_get_year(d);

#if defined HAVE_ANON_STRUCTS_INIT
	return (dt_yd_t){.y = y, .d = yd};
#else
	dt_yd_t res;
	res.y = y;
	res.d = yd;
	return res;
#endif
}
#endif	/* ASPECT_CONV */


#if defined ASPECT_ADD && !defined DAISY_ASPECT_ADD_
#define DAISY_ASPECT_ADD_
#define ASPECT_GETTERS
#include "daisy.c"
#undef ASPECT_GETTERS

static __attribute__((const, pure)) dt_daisy_t
__daisy_add_d(dt_daisy_t d, int n)
{
/* add N days to D */
	d += n;
	return d;
}

static __attribute__((const, pure)) dt_daisy_t
__daisy_add_b(dt_daisy_t d, int n)
{
/* add N business days to D */
	dt_dow_t dow = __daisy_get_wday(d);
	int equ = __get_d_equiv(dow, n);
	d += equ;
	return d;
}

static __attribute__((const, pure)) dt_daisy_t
__daisy_add_w(dt_daisy_t d, int n)
{
/* add N weeks to D */
	return __daisy_add_d(d, GREG_DAYS_P_WEEK * n);
}
#endif	/* ASPECT_ADD */


#if defined ASPECT_DIFF && !defined DAISY_ASPECT_DIFF_
#define DAISY_ASPECT_DIFF_
static __attribute__((const, pure)) struct dt_d_s
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

DEFUN void
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
