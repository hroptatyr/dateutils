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

struct __md_s {
	unsigned int m;
	unsigned int d;
};


/* helpers */
#if 1
static uint16_t __mon_yday[] = {
/* this is \sum ml, first element is a bit set of leap days to add */
	0xfff8, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};
#endif

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


/* helpers from the calendar files, don't define any aspect, so only
 * the helpers should get included */
#include "yd.c"
#include "ymd.c"
#include "ymcw.c"
#include "ywd.c"
#include "bizda.c"
#include "daisy.c"


#define ASPECT_GETTERS
#include "yd.c"
#include "ymd.c"
#include "ymcw.c"
#include "ywd.c"
#include "bizda.c"
#include "daisy.c"
#undef ASPECT_GETTERS

#define ASPECT_CONV
#include "yd.c"
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

/* forward decl */
static dt_yd_t dt_conv_to_yd(struct dt_d_s this);

static int
dt_get_wcnt_year(struct dt_d_s this, unsigned int wkcnt_convention)
{
	int res;

	switch (this.typ) {
	case DT_YMD:
	case DT_DAISY:
	case DT_YD: {
		dt_yd_t yd = dt_conv_to_yd(this);

		switch (wkcnt_convention) {
		default:
		case YWD_ABSWK_CNT:
			res = __yd_get_wcnt_abs(yd);
			break;
		case YWD_ISOWK_CNT:
			res = __yd_get_wcnt_iso(yd);
			break;
		case YWD_MONWK_CNT:
		case YWD_SUNWK_CNT: {
			/* using monwk_cnt is a minor trick
			 * from = 1 = Mon or 0 = Sun */
			int from = wkcnt_convention == YWD_MONWK_CNT;
			res = __yd_get_wcnt(yd, from);
			break;
		}
		}
		break;
	}
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
		return __ywd_to_daisy(that.ywd);
	case DT_BIZDA:
		return __bizda_to_daisy(that.bizda, __get_bizda_param(that));
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
		return __bizda_to_ymcw(that.bizda, __get_bizda_param(that));
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
/* the problem with this conversion is that not all dates can be mapped
 * to a bizda date, so we need a policy first what to do in case things
 * go massively pear-shaped. */
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
		return __bizda_to_ywd(this.bizda, __get_bizda_param(this));
	case DT_DUNK:
	default:
		break;
	}
	return (dt_ywd_t){.u = 0};
}

static dt_yd_t
dt_conv_to_yd(struct dt_d_s this)
{
	switch (this.typ) {
	case DT_YD:
		/* yay, that was quick */
		return this.yd;
	case DT_YMD:
		return __ymd_to_yd(this.ymd);
	case DT_DAISY:
		return __daisy_to_yd(this.daisy);
	case DT_YMCW:
		return __ymcw_to_yd(this.ymcw);
	case DT_YWD:
		return __ywd_to_yd(this.ywd);
	default:
		break;
	}
	return (dt_yd_t){.u = 0};
}


/* arithmetic */
#define ASPECT_ADD
#include "yd.c"
#include "ymd.c"
#include "ymcw.c"
#include "ywd.c"
#include "bizda.c"
#include "daisy.c"
#undef ASPECT_ADD

#define ASPECT_DIFF
#include "yd.c"
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
#if !defined SKIP_LEAP_ARITH
/* we assume this file is in the dist, it's gen'd from fmt-special.gperf */
# include "fmt-special.c"
#endif	/* SKIP_LEAP_ARITH */

static const char ymd_dflt[] = "%F";
static const char ymcw_dflt[] = "%Y-%m-%c-%w";
static const char ywd_dflt[] = "%rY-W%V-%u";
static const char daisy_dflt[] = "%d";
static const char bizsi_dflt[] = "%db";
static const char bizda_dflt[] = "%Y-%m-%db";

static dt_dtyp_t
__trans_dfmt_special(const char *fmt)
{
#if !defined SKIP_LEAP_ARITH
	size_t len = strlen(fmt);
	const struct dt_fmt_special_s *res;

	if (UNLIKELY((res = __fmt_special(fmt, len)) != NULL)) {
		return res->e;
	}
#else  /* SKIP_LEAP_ARITH */
	(void)fmt;
#endif	/* !SKIP_LEAP_ARITH */
	return DT_DUNK;
}

static void
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

	if (LIKELY(d.y > 0 && d.c <= 0 && !d.flags.c_wcnt_p && !d.flags.bizda)) {
		/* nearly all goes to ymd */
		res.typ = DT_YMD;
		res.ymd.y = d.y;
		if (LIKELY(!d.flags.d_dcnt_p)) {
			res.ymd.m = d.m;
#if defined WITH_FAST_ARITH
			res.ymd.d = d.d;
#else  /* !WITH_FAST_ARITH */
			unsigned int md = __get_mdays(d.y, d.m);
			/* check for illegal dates, like 31st of April */
			if ((res.ymd.d = d.d) > md) {
				res.ymd.d = md;
			}
		} else {
			/* convert dcnt to m + d */
			struct __md_s r = __yday_get_md(d.y, d.d);
			res.ymd.m = r.m;
			res.ymd.d = r.d;
		}
#endif	/* !WITH_FAST_ARITH */
	} else if (d.y > 0 && d.m <= 0 && !d.flags.bizda) {
		res.typ = DT_YWD;
		res.ywd = __make_ywd_c(d.y, d.c, d.w, d.flags.wk_cnt);
	} else if (d.y > 0 && !d.flags.bizda) {
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
	} else if (d.y > 0 && d.flags.bizda) {
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
		struct dt_spec_s spec = __tok_spec(fp_sav, &fp);

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
	res = __guess_dtyp(d);
out:
	/* set the end pointer */
	if (ep != NULL) {
		*ep = (char*)sp;
	}
	return res;
}

#define ASPECT_STRF
#include "yd.c"
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
		struct dt_spec_s spec = __tok_spec(fp_sav, &fp);

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
	struct dt_d_s res = dt_d_initialiser();
	struct strpd_s d = strpd_initialiser();
	const char *sp = str;
	int tmp;

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
	if (d.b && (d.y || d.m >= 1)) {
		res.typ = DT_BIZDA;
		res.bizda.y = d.y;
		res.bizda.m = d.q * 3 + d.m;
		res.bizda.bd = d.b + d.w * DUWW_BDAYS_P_WEEK;
	} else if (LIKELY(d.y || d.m >= 1)) {
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
		struct dt_spec_s spec = __tok_spec(fp_sav, &fp);

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
	struct dt_d_s res = dt_d_initialiser();

	switch ((res.typ = tgttyp)) {
	case DT_YMD:
		res.ymd = dt_conv_to_ymd(d);
		break;
	case DT_YMCW:
		res.ymcw = dt_conv_to_ymcw(d);
		break;
	case DT_DAISY:
	case DT_JDN:
	case DT_LDN: {
		dt_daisy_t tmp = dt_conv_to_daisy(d);
		switch (tgttyp) {
		case DT_DAISY:
			res.daisy = tmp;
			break;
		case DT_LDN:
			res.ldn = __daisy_to_ldn(tmp);
			break;
		case DT_JDN:
			res.jdn = __daisy_to_jdn(tmp);
			break;
		default:
			/* nice one gcc */
			;
		}
		break;
	}
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
dt_dadd_d(struct dt_d_s d, int n)
{
/* add N (gregorian) days to D */
	switch (d.typ) {
	case DT_DAISY:
		d.daisy = __daisy_add_d(d.daisy, n);
		break;

	case DT_YMD:
		d.ymd = __ymd_add_d(d.ymd, n);
		break;

	case DT_YMCW:
		d.ymcw = __ymcw_add_d(d.ymcw, n);
		break;

	case DT_BIZDA:
		d.bizda = __bizda_add_d(d.bizda, n);
		break;

	case DT_YWD:
		d.ywd = __ywd_add_d(d.ywd, n);
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
dt_dadd_b(struct dt_d_s d, int n)
{
/* add N business days to D */
	switch (d.typ) {
	case DT_DAISY:
		d.daisy = __daisy_add_b(d.daisy, n);
		break;

	case DT_YMD:
		d.ymd = __ymd_add_b(d.ymd, n);
		break;

	case DT_YMCW:
		d.ymcw = __ymcw_add_b(d.ymcw, n);
		break;

	case DT_BIZDA:
		d.bizda = __bizda_add_b(d.bizda, n);
		break;

	case DT_YWD:
		d.ywd = __ywd_add_b(d.ywd, n);
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
dt_dadd_w(struct dt_d_s d, int n)
{
/* add N weeks to D */
	switch (d.typ) {
	case DT_DAISY:
		d.daisy = __daisy_add_w(d.daisy, n);
		break;

	case DT_YMD:
		d.ymd = __ymd_add_w(d.ymd, n);
		break;

	case DT_YMCW:
		d.ymcw = __ymcw_add_w(d.ymcw, n);
		break;

	case DT_BIZDA:
		d.bizda = __bizda_add_w(d.bizda, n);
		break;

	case DT_YWD:
		d.ywd = __ywd_add_w(d.ywd, n);
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
dt_dadd_m(struct dt_d_s d, int n)
{
/* add N months to D */
	switch (d.typ) {
	case DT_DAISY:
		/* daisy objects have no notion of months */
		break;

	case DT_YMD:
		d.ymd = __ymd_add_m(d.ymd, n);
		break;

	case DT_YMCW:
		d.ymcw = __ymcw_add_m(d.ymcw, n);
		break;

	case DT_BIZDA:
		d.bizda = __bizda_add_m(d.bizda, n);
		break;

	case DT_YWD:
		/* ywd have no notion of months */
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
dt_dadd_y(struct dt_d_s d, int n)
{
/* add N years to D */
	switch (d.typ) {
	case DT_DAISY:
		/* daisy objects have no notion of years */
		break;

	case DT_YMD:
		d.ymd = __ymd_add_y(d.ymd, n);
		break;

	case DT_YMCW:
		d.ymcw = __ymcw_add_y(d.ymcw, n);
		break;

	case DT_BIZDA:
		d.bizda = __bizda_add_y(d.bizda, n);
		break;

	case DT_YWD:
		d.ywd = __ywd_add_y(d.ywd, n);
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
dt_dadd(struct dt_d_s d, struct dt_d_s dur)
{
	struct strpdi_s durcch = strpdi_initialiser();

	__fill_strpdi(&durcch, dur);

	if (durcch.d) {
		d = dt_dadd_d(d, durcch.d);
	} else if (durcch.b) {
		d = dt_dadd_b(d, durcch.b);
	}
	if (durcch.w) {
		d = dt_dadd_w(d, durcch.w);
	}
	if (durcch.m) {
		d = dt_dadd_m(d, durcch.m);
	}
	if (durcch.y) {
		d = dt_dadd_y(d, durcch.y);
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
