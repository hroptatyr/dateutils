/*** date-core.c -- our universe of dates
 *
 * Copyright (C) 2011-2022 Sebastian Freundt
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
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include "date-core.h"
#include "date-core-private.h"
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

#if !defined DEFUN
# define DEFUN
#endif	/* !DEFUN */
#if !defined DEFVAR
# define DEFVAR
#endif	/* !DEFVAR */

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
#include "gmtime.h"

/* bizda definitions, reference dates */
static __attribute__((unused)) const char *bizda_ult[] = {"ultimo", "ult"};

/* arithmetic helpers */
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

static inline __attribute__((const, pure)) int
__sgn(signed int x)
{
	return (x > 0) - (x < 0);
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
		/* should have come through the GETTERS aspect */
		return __ywd_get_md(that.ywd);
	case DT_YD:
		return __yd_get_md(that.yd);
	}
}

/* too exotic to be public, nope bug #81 needs it */
int
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

DEFUN int
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
			dt_dow_t from;

			switch (wkcnt_convention) {
			case YWD_MONWK_CNT:
				from = DT_MONDAY;
				break;
			case YWD_SUNWK_CNT:
				from = DT_SUNDAY;
				break;
			default:
				/* huh? */
				from = DT_MIRACLEDAY;
				break;
			}
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
DEFUN dt_daisy_t
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
	case DT_LDN:
		return __ldn_to_daisy(that.ldn);
	case DT_JDN:
		return __jdn_to_daisy(that.jdn);
	case DT_MDN:
		return __mdn_to_daisy(that.mdn);
	case DT_YD:
		return __yd_to_daisy(that.yd);
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
	case DT_JDN:
		that.daisy = __jdn_to_daisy(that.jdn);
		goto daisy;
	case DT_LDN:
		that.daisy = __ldn_to_daisy(that.ldn);
		goto daisy;
	case DT_MDN:
		that.daisy = __mdn_to_daisy(that.mdn);
		goto daisy;
	case DT_DAISY:
	daisy:
		return __daisy_to_ymd(that.daisy);
	case DT_BIZDA:
		return __bizda_to_ymd(that.bizda);
	case DT_YWD:
		return __ywd_to_ymd(that.ywd);
	case DT_YD:
		return __yd_to_ymd(that.yd);
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
	case DT_JDN:
		that.daisy = __jdn_to_daisy(that.jdn);
		goto daisy;
	case DT_LDN:
		that.daisy = __ldn_to_daisy(that.ldn);
		goto daisy;
	case DT_MDN:
		that.daisy = __mdn_to_daisy(that.mdn);
		goto daisy;
	case DT_DAISY:
	daisy:
		return __daisy_to_ymcw(that.daisy);
	case DT_BIZDA:
		return __bizda_to_ymcw(that.bizda, __get_bizda_param(that));
	case DT_YWD:
		return __ywd_to_ymcw(that.ywd);
	case DT_YD:
		return __yd_to_ymcw(that.yd);
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
	case DT_YD:
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
	case DT_JDN:
		this.daisy = __jdn_to_daisy(this.jdn);
		goto daisy;
	case DT_LDN:
		this.daisy = __ldn_to_daisy(this.ldn);
		goto daisy;
	case DT_MDN:
		this.daisy = __mdn_to_daisy(this.mdn);
		goto daisy;
	case DT_DAISY:
	daisy:
		return __daisy_to_ywd(this.daisy);
	case DT_BIZDA:
		return __bizda_to_ywd(this.bizda, __get_bizda_param(this));
	case DT_YD:
		return __yd_to_ywd(this.yd);
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
	case DT_JDN:
		this.daisy = __jdn_to_daisy(this.jdn);
		goto daisy;
	case DT_LDN:
		this.daisy = __ldn_to_daisy(this.ldn);
		goto daisy;
	case DT_MDN:
		this.daisy = __mdn_to_daisy(this.mdn);
		goto daisy;
	case DT_DAISY:
	daisy:
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

#define ASPECT_CMP
#include "yd.c"
#include "ymd.c"
#include "ymcw.c"
#include "ywd.c"
#include "bizda.c"
#include "daisy.c"
#undef ASPECT_CMP


/* guessing parsers */
#include "strops.h"
#include "token.h"
#include "date-core-strpf.c"
#if !defined SKIP_LEAP_ARITH
/* we assume this file is in the dist, it's gen'd from fmt-special.gperf */
# include "fmt-special.c"
#endif	/* SKIP_LEAP_ARITH */

DEFVAR const char ymd_dflt[] = "%F";
DEFVAR const char ymcw_dflt[] = "%Y-%m-%c-%w";
DEFVAR const char ywd_dflt[] = "%G-W%V-%u";
DEFVAR const char yd_dflt[] = "%Y-%D";
DEFVAR const char daisy_dflt[] = "%d";
DEFVAR const char bizsi_dflt[] = "%db";
DEFVAR const char bizda_dflt[] = "%Y-%m-%db";

DEFVAR const char ymddur_dflt[] = "%Y-%0m-%0d";
DEFVAR const char ymcwdur_dflt[] = "%Y-%0m-%0w-%0d";
DEFVAR const char ywddur_dflt[] = "%G-W%0w-%d";
DEFVAR const char yddur_dflt[] = "%Y-%0d";
DEFVAR const char daisydur_dflt[] = "%d";
DEFVAR const char bizsidur_dflt[] = "%db";
DEFVAR const char bizdadur_dflt[] = "%Y-%0m-%0db";

DEFUN dt_dtyp_t
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

DEFUN dt_dtyp_t
__trans_dfmt(const char **fmt)
{
	if (UNLIKELY(*fmt == NULL)) {
		/* great, standing ovations to the user */
		;
	} else if (LIKELY(**fmt == '%')) {
		/* don't worry about it */
		;
	} else {
		const dt_dtyp_t tmp = __trans_dfmt_special(*fmt);

		switch (tmp) {
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
		case DT_YD:
			*fmt = yd_dflt;
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

		return tmp;
	}
	return DT_DUNK;
}

DEFUN dt_durtyp_t
__trans_ddurfmt(const char **fmt)
{
	if (UNLIKELY(*fmt == NULL)) {
		/* great, standing ovations to the user */
		;
	} else if (LIKELY(**fmt == '%')) {
		/* don't worry about it */
		;
	} else {
		unsigned int tmp = __trans_dfmt_special(*fmt);

		switch (tmp) {
		default:
			break;
		case DT_YMD:
			*fmt = ymddur_dflt;
			tmp = DT_DURYMD;
			break;
		case DT_YMCW:
			*fmt = ymcwdur_dflt;
			tmp = DT_DURYMCW;
			break;
		case DT_YWD:
			*fmt = ywddur_dflt;
			tmp = DT_DURYWD;
			break;
		case DT_YD:
			*fmt = yddur_dflt;
			tmp = DT_DURYD;
			break;
		case DT_BIZDA:
			*fmt = bizdadur_dflt;
			tmp = DT_DURBIZDA;
			break;
		case DT_DAISY:
			*fmt = daisydur_dflt;
			tmp = DT_DURD;
			break;
		case DT_BIZSI:
			*fmt = bizsidur_dflt;
			tmp = DT_DURBD;
			break;
		}

		return (dt_durtyp_t)tmp;
	}
	return DT_DURUNK;
}

/* strpf glue */
DEFUN struct dt_d_s
__guess_dtyp(struct strpd_s d)
{
	struct dt_d_s res = {DT_DUNK};

	if (LIKELY(d.y > 0 && d.c <= 0 && !d.flags.c_wcnt_p && !d.flags.bizda)) {
		/* nearly all goes to ymd */
		res.typ = DT_YMD;
		res.ymd.y = d.y;
		if (LIKELY(!d.flags.d_dcnt_p)) {
#if defined WITH_FAST_ARITH
			res.ymd.d = d.d;
#else  /* !WITH_FAST_ARITH */
			unsigned int md = __get_mdays(d.y, d.m);
			/* check for illegal dates, like 31st of April */
			if ((res.ymd.d = d.d) > md) {
				res.ymd.d = md;
				res.fix = 1U;
			}
#endif	/* !WITH_FAST_ARITH */
			/* month is always pertained */
			res.ymd.m = d.m;
		} else {
			/* produce yd dates */
			res.typ = DT_YD;
			res.yd.y = d.y;
#if defined WITH_FAST_ARITH
			res.yd.d = d.d;
#else  /* !WITH_FAST_ARITH */
			with (int maxd = __get_ydays(d.y)) {
				if (UNLIKELY((res.yd.d = d.d) > maxd)) {
					res.yd.d = maxd;
				}
			}
#endif	/* WITH_FAST_ARITH */
		}
	} else if (d.y > 0 && d.m <= 0 && !d.flags.bizda) {
		res.typ = DT_YWD;
		res.ywd = __make_ywd_c(d.y, d.c, (dt_dow_t)d.w, d.flags.wk_cnt);
	} else if (d.y > 0 && !d.flags.bizda) {
		/* its legit for d.w to be naught */
		res.typ = DT_YMCW;
		res.ymcw.y = d.y;
		res.ymcw.m = d.m;
#if defined WITH_FAST_ARITH
		res.ymcw.c = d.c;
#else  /* !WITH_FAST_ARITH */
		if (UNLIKELY((res.ymcw.c = d.c) >= 5)) {
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
#if defined WITH_FAST_ARITH
		res.bizda.bd = d.b;
#else  /* !WITH_FAST_ARITH */
		unsigned int bd = __get_bdays(d.y, d.m);
		if ((res.bizda.bd = d.b) > bd) {
			res.bizda.bd = bd;
			res.fix = 1U;
		}
#endif	/* WITH_FAST_ARITH */
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
	struct dt_d_s res = {DT_DUNK};
	struct strpd_s d = {0};
	const char *sp = str;
	const char *fp;

	if (UNLIKELY(fmt == NULL)) {
		return __strpd_std(str, ep);
	}
	/* translate high-level format names */
	__trans_dfmt(&fmt);

	fp = fmt;
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
	struct strpd_s d = {0};
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
		case DT_YD:
			fmt = yd_dflt;
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
	case DT_YD:
		d.y = that.yd.y;
		d.d = that.yd.d;
		d.flags.d_dcnt_p = 1U;
		break;
	case DT_JDN:
		that.typ = DT_DAISY;
		that.daisy = __jdn_to_daisy(that.jdn);
		goto daisy_prep;
	case DT_LDN:
		that.typ = DT_DAISY;
		that.daisy = __ldn_to_daisy(that.ldn);
		goto daisy_prep;
	case DT_MDN:
		that.typ = DT_DAISY;
		that.daisy = __mdn_to_daisy(that.mdn);
		goto daisy_prep;
	case DT_DAISY:
	daisy_prep:
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

DEFUN struct dt_ddur_s
dt_strpddur(const char *str, char **ep)
{
/* at the moment we allow only one format */
	struct dt_ddur_s res = dt_make_ddur(DT_DURUNK, 0);
	const char *sp = str;
	long int tmp;

	if (str == NULL) {
		goto out;
	}
	/* read off co-class indicator */
	if (*sp == '/') {
		res.cocl = 1U;
		sp++;
	}
	/* read just one component, use rudi's errno trick */
	errno = 0;
	if ((tmp = strtol(str, (char**)&sp, 10)) == 0 && str == sp) {
		/* didn't work aye? */
		goto out;
	} else if (tmp > INT_MAX || errno) {
		errno = ERANGE;
		goto out;
	}

	switch (*sp++) {
	case '\0':
		/* must have been day then */
		res.durtyp = DT_DURD;
		sp--;
		break;
	case 'd':
	case 'D':
		res.durtyp = DT_DURD;
		break;
	case 'y':
	case 'Y':
		res.durtyp = DT_DURYR;
		break;
	case 'm':
	case 'M':
		res.durtyp = DT_DURMO;
		break;
	case 'w':
	case 'W':
		res.durtyp = DT_DURWK;
		break;
	case 'b':
	case 'B':
		res.durtyp = DT_DURBD;
		break;
	case 'q':
	case 'Q':
		res.durtyp = DT_DURQU;
		break;
	default:
		sp = str;
		goto out;
	}
	/* no further checks on TMP */
	res.dv = tmp;
out:
	if (ep != NULL) {
		*ep = (char*)sp;
	}
	return res;
}

DEFUN size_t
dt_strfddur(char *restrict buf, size_t bsz, const char *fmt, struct dt_ddur_s that)
{
	struct strpd_s d = {0};
	const char *fp;
	char *bp;

	if (UNLIKELY(buf == NULL || bsz == 0)) {
		return 0;
	}

	switch (that.durtyp) {
	case DT_DURYMD:
		d.y = that.ymd.y;
		d.m = that.ymd.m;
		d.d = that.ymd.d;
		if (fmt == NULL) {
			fmt = ymddur_dflt;
		}
		break;
	case DT_DURYMCW:
		d.y = that.ymcw.y;
		d.m = that.ymcw.m;
		d.c = that.ymcw.c;
		d.d = that.ymcw.w;
		if (fmt == NULL) {
			fmt = ymcwdur_dflt;
		}
		break;
	case DT_DURYWD:
		d.y = that.ywd.y;
		d.c = that.ywd.c;
		d.d = that.ywd.w;
		if (fmt == NULL) {
			fmt = ywddur_dflt;
		}
		break;
	case DT_DURYD:
		d.y = that.yd.y;
		d.d = that.yd.d;
		if (fmt == NULL) {
			fmt = yddur_dflt;
		}
		break;
	case DT_DURBIZDA:;
		dt_bizda_param_t bparam;

		bparam.bs = that.param;
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
			fmt = bizdadur_dflt;
		}
		break;

	case DT_DURD:
	case DT_DURBD:
	case DT_DURWK:
	case DT_DURMO:
	case DT_DURQU:
	case DT_DURYR:
		if (that.dv >= 0) {
			/* make sure the neg bit doesn't bite us */
			that.dv = -that.dv;
			that.neg = 1U;
		}

		switch (that.durtyp) {
		case DT_DURD:
			d.d = that.dv;
			break;
		case DT_DURBD:
			d.b = that.dv;
			break;
		case DT_DURWK:
			d.d = that.dv * GREG_DAYS_P_WEEK;
			break;
		case DT_DURMO:
			d.m = that.dv;
			break;
		case DT_DURQU:
			d.m = that.dv * 3U;
			break;
		case DT_DURYR:
			d.y = that.dv;
			break;
		}
		break;

	default:
	case DT_DUNK:
		bp = buf;
		goto out;
	}
	/* translate high-level format names */
	__trans_ddurfmt(&fmt);

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
	case DT_YMCW:
	case DT_YWD:
	case DT_YD: {
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
		case DT_YD:
			res.yd.y = tm.tm_year;
			res.yd.d = tm.tm_yday;
			break;
		case DT_YWD:
			/* use ordinary conversion to ywd */
			res.typ = DT_YMD;
			res.ymd.y = tm.tm_year;
			res.ymd.m = tm.tm_mon;
			res.ymd.d = tm.tm_mday;
			res = dt_dconv(DT_YWD, res);
			break;

		default:
			break;
		}
		break;
	}

	case DT_DAISY:
		/* time_t's base is 1970-01-01, which is daisy 19359 */
		res.daisy = t / 86400 + 19359;
		break;

		/* the rest doesn't make sense I say */
	default:
	case DT_DUNK:
		res.u = 0;
	}
	return res;
}

DEFUN struct dt_d_s
dt_dconv(dt_dtyp_t tgttyp, struct dt_d_s d)
{
	struct dt_d_s res = {DT_DUNK};

	/* fix up before conversion */
	d = dt_dfixup(d);

	switch ((res.typ = tgttyp)) {
	case DT_YMD:
		res.ymd = dt_conv_to_ymd(d);
		break;
	case DT_YMCW:
		res.ymcw = dt_conv_to_ymcw(d);
		break;
	case DT_DAISY:
	case DT_JDN:
	case DT_LDN:
	case DT_MDN: {
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
		case DT_MDN:
			res.mdn = __daisy_to_mdn(tmp);
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
	case DT_YD:
		res.yd = dt_conv_to_yd(d);
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
	if (UNLIKELY(!n)) {
		/* can't use short-cut return here, it'd upset the IPO/LTO */
		goto out;
	}
	switch (d.typ) {
	case DT_JDN:
		d.daisy = __jdn_to_daisy(d.jdn);
		goto daisy_add_d;

	case DT_LDN:
		d.daisy = __ldn_to_daisy(d.ldn);
		goto daisy_add_d;

	case DT_MDN:
		d.daisy = __mdn_to_daisy(d.mdn);
		goto daisy_add_d;

	case DT_DAISY:
	daisy_add_d:
		d.daisy = __daisy_add_d(d.daisy, n);

		/* transform back (maybe) */
		switch (d.typ) {
		case DT_DAISY:
		default:
			break;
		case DT_LDN:
			d.ldn = __daisy_to_ldn(d.daisy);
			break;
		case DT_JDN:
			d.jdn = __daisy_to_jdn(d.daisy);
			break;
		case DT_MDN:
			d.mdn = __daisy_to_mdn(d.daisy);
			break;
		}
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

	case DT_YD:
		d.yd = __yd_add_d(d.yd, n);
		break;

	case DT_DUNK:
	default:
		d.typ = DT_DUNK;
		d.u = 0;
		break;
	}
out:
	return d;
}

DEFUN struct dt_d_s
dt_dadd_b(struct dt_d_s d, int n)
{
/* add N business days to D */
	if (UNLIKELY(!n)) {
		/* cacn't use short-cut return here, it'd upset the IPO/LTO */
		goto out;
	}
	switch (d.typ) {
	case DT_JDN:
		d.daisy = __jdn_to_daisy(d.jdn);
		goto daisy_add_b;

	case DT_LDN:
		d.daisy = __ldn_to_daisy(d.ldn);
		goto daisy_add_b;

	case DT_MDN:
		d.daisy = __mdn_to_daisy(d.mdn);
		goto daisy_add_b;

	case DT_DAISY:
	daisy_add_b:
		d.daisy = __daisy_add_b(d.daisy, n);

		/* transform back (maybe) */
		switch (d.typ) {
		case DT_DAISY:
		default:
			break;
		case DT_LDN:
			d.ldn = __daisy_to_ldn(d.daisy);
			break;
		case DT_JDN:
			d.jdn = __daisy_to_jdn(d.daisy);
			break;
		case DT_MDN:
			d.mdn = __daisy_to_mdn(d.daisy);
			break;
		}
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

	case DT_YD:
		d.yd = __yd_add_b(d.yd, n);
		break;

	case DT_DUNK:
	default:
		d.typ = DT_DUNK;
		d.u = 0;
		break;
	}
out:
	return d;
}

DEFUN struct dt_d_s
dt_dadd_w(struct dt_d_s d, int n)
{
/* add N weeks to D */
	if (UNLIKELY(!n)) {
		/* cacn't use short-cut return here, it'd upset the IPO/LTO */
		goto out;
	}
	switch (d.typ) {
	case DT_JDN:
		d.daisy = __jdn_to_daisy(d.jdn);
		goto daisy_add_w;

	case DT_LDN:
		d.daisy = __ldn_to_daisy(d.ldn);
		goto daisy_add_w;

	case DT_MDN:
		d.daisy = __mdn_to_daisy(d.mdn);
		goto daisy_add_w;

	case DT_DAISY:
	daisy_add_w:
		d.daisy = __daisy_add_w(d.daisy, n);

		/* transform back (maybe) */
		switch (d.typ) {
		case DT_DAISY:
		default:
			break;
		case DT_LDN:
			d.ldn = __daisy_to_ldn(d.daisy);
			break;
		case DT_JDN:
			d.jdn = __daisy_to_jdn(d.daisy);
			break;
		case DT_MDN:
			d.mdn = __daisy_to_mdn(d.daisy);
			break;
		}
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

	case DT_YD:
		d.yd = __yd_add_w(d.yd, n);
		break;

	case DT_DUNK:
	default:
		d.typ = DT_DUNK;
		d.u = 0;
		break;
	}
out:
	return d;
}

DEFUN struct dt_d_s
dt_dadd_m(struct dt_d_s d, int n)
{
/* add N months to D */
	if (UNLIKELY(!n)) {
		goto out;
	}
	switch (d.typ) {
	case DT_LDN:
	case DT_JDN:
	case DT_MDN:
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

	case DT_YD:
		/* yd have no notion of months */
		break;

	case DT_DUNK:
	default:
		d.typ = DT_DUNK;
		d.u = 0;
		break;
	}
out:
	return d;
}

DEFUN struct dt_d_s
dt_dadd_y(struct dt_d_s d, int n)
{
/* add N years to D */
	if (UNLIKELY(!n)) {
		/* cacn't use short-cut return here, it'd upset the IPO/LTO */
		goto out;
	}
	switch (d.typ) {
	case DT_LDN:
	case DT_JDN:
	case DT_MDN:
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

	case DT_YD:
		d.yd = __yd_add_y(d.yd, n);
		break;

	case DT_DUNK:
	default:
		d.typ = DT_DUNK;
		d.u = 0;
		break;
	}
out:
	return d;
}

DEFUN struct dt_d_s
dt_dadd(struct dt_d_s d, struct dt_ddur_s dur)
{
	if (UNLIKELY(!dur.dv)) {
		/* can't use short-cut return here, it'd upset the IPO/LTO */
		goto out;
	}
	switch (dur.durtyp) {
	case DT_DURD:
		d = dt_dadd_d(d, dur.dv);
		break;
	case DT_DURBD:
		d = dt_dadd_b(d, dur.dv);
		break;
	case DT_DURWK:
		d = dt_dadd_w(d, dur.dv);
		break;
	case DT_DURQU:
		/* just use the month adder */
		dur.dv *= 3U;
	case DT_DURMO:
		d = dt_dadd_m(d, dur.dv);
		break;
	case DT_DURYR:
		d = dt_dadd_y(d, dur.dv);
		break;
	default:
	case DT_DURUNK:
		/* huh? */
		break;
	}
out:
	return d;
}

DEFUN struct dt_ddur_s
dt_neg_dur(struct dt_ddur_s dur)
{
	dur.neg = (uint16_t)(~dur.neg & 0x01);
	switch (dur.durtyp) {
	case DT_DURD:
	case DT_DURBD:
	case DT_DURWK:
	case DT_DURMO:
	case DT_DURQU:
	case DT_DURYR:
		dur.dv = -dur.dv;
		break;

	default:
		break;
	}
	return dur;
}

DEFUN int
dt_dur_neg_p(struct dt_ddur_s dur)
{
	switch (dur.durtyp) {
	case DT_DURD:
	case DT_DURBD:
	case DT_DURWK:
	case DT_DURMO:
	case DT_DURQU:
	case DT_DURYR:
		return dur.dv < 0;
	default:
		break;
	}
	return dur.neg;
}

DEFUN struct dt_ddur_s
dt_ddiff(dt_durtyp_t tgttyp, struct dt_d_s d1, struct dt_d_s d2, int carry)
{
/* carry of 0 doesn't change anything
 * carry <0 adds -1 to d2 iff d1 <= d2
 * carry >0 adds 1 to d2 iff d1 >= d2 */
	struct dt_ddur_s res = dt_make_ddur(DT_DURUNK, 0);

	switch (tgttyp) {
	case DT_DURD:
	case DT_DURBD: {
		dt_daisy_t tmp1 = dt_conv_to_daisy(d1);
		dt_daisy_t tmp2 = dt_conv_to_daisy(d2);

		res = __daisy_diff(tmp1, tmp2);

		/* fix up result in case it's bizsi, i.e. kick weekends */
		if (tgttyp == DT_DURBD) {
			dt_dow_t wdb = __daisy_get_wday(tmp2);
			res.dv = __get_nbdays(res.dv, wdb);
		}
		break;
	}
	case DT_DURYMD: {
		dt_ymd_t tmp1 = dt_conv_to_ymd(d1);
		dt_ymd_t tmp2 = dt_conv_to_ymd(d2);
		int fix = __sgn(carry) == -__sgn(tmp2.u - tmp1.u);

		if (UNLIKELY(fix)) {
			/* add -1 iff tmp2 > tmp1 && carry < 0
			 * add 1 iff tmp2 < tmp1 && carry > 0, i.e.
			 * add sgn(carry) iff sgn(tmp2-tmp1) = -sgn(carry) */
			tmp2 = __ymd_add_d(tmp2, __sgn(carry));
		}
		res = __ymd_diff(tmp1, tmp2);
		res.fix = fix;
		break;
	}
	case DT_DURYMCW: {
		dt_ymcw_t tmp1 = dt_conv_to_ymcw(d1);
		dt_ymcw_t tmp2 = dt_conv_to_ymcw(d2);
		int fix = carry && __sgn(carry) == -__ymcw_cmp(tmp1, tmp2);

		if (UNLIKELY(fix)) {
			/* add -1 iff tmp2 > tmp1 && carry < 0
			 * add 1 iff tmp2 < tmp1 && carry > 0, i.e.
			 * add sgn(carry) iff sgn(tmp2-tmp1) = -sgn(carry) */
			tmp2 = __ymcw_add_d(tmp2, __sgn(carry));
		}
		res = __ymcw_diff(tmp1, tmp2);
		res.fix = fix;
		break;
	}
	case DT_DURYD: {
		dt_yd_t tmp1 = dt_conv_to_yd(d1);
		dt_yd_t tmp2 = dt_conv_to_yd(d2);
		int fix = __sgn(carry) == -__sgn(tmp2.u - tmp1.u);

		if (UNLIKELY(fix)) {
			/* add -1 iff tmp2 > tmp1 && carry < 0
			 * add 1 iff tmp2 < tmp1 && carry > 0, i.e.
			 * add sgn(carry) iff sgn(tmp2-tmp1) = -sgn(carry) */
			tmp2 = __yd_add_d(tmp2, __sgn(carry));
		}
		res = __yd_diff(tmp1, tmp2);
		res.fix = fix;
		break;
	}
	case DT_DURYWD: {
		dt_ywd_t tmp1 = dt_conv_to_ywd(d1);
		dt_ywd_t tmp2 = dt_conv_to_ywd(d2);
		int fix = __sgn(carry) == -__sgn(tmp2.u - tmp1.u);

		if (UNLIKELY(fix)) {
			/* add -1 iff tmp2 > tmp1 && carry < 0
			 * add 1 iff tmp2 < tmp1 && carry > 0, i.e.
			 * add sgn(carry) iff sgn(tmp2-tmp1) = -sgn(carry) */
			tmp2 = __ywd_add_d(tmp2, __sgn(carry));
		}
		res = __ywd_diff(tmp1, tmp2);
		res.fix = fix;
		break;
	}
	case DT_DURBIZDA:
	case DT_DURUNK:
	default:
		break;
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
	case DT_YWD:
	case DT_YD:
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
/* use the following multiplication table
 *
 * |d,d2|v |d,d1|>  -2 -1  0  1
 *      -2          -1 -1 -1 -1
 *      -1          -1  0  1  1
 *       0          -1  0  1  1
 *       1          -1  0  0  0
 *
 * encoded in a 32bit uint */
	static const uint32_t m = 0b10010111100101111010101111111111U;
	const unsigned int i = (dt_dcmp(d, d1) + 2) & 0b11U;
	const unsigned int j = (dt_dcmp(d, d2) + 2) & 0b11U;
	return 2 - ((m >> (i * 8U + j * 2U)) & 0b11U);
}

DEFUN __attribute__((const)) struct dt_d_s
dt_dfixup(struct dt_d_s d)
{
	switch (d.typ) {
	case DT_YMD:
		d.ymd = __ymd_fixup(d.ymd);
		break;
	case DT_YMCW:
		d.ymcw = __ymcw_fixup(d.ymcw);
		break;
	case DT_YWD:
		d.ywd = __ywd_fixup(d.ywd);
		break;
	case DT_YD:
		d.yd = __yd_fixup(d.yd);
		break;
	case DT_BIZDA:
		d.bizda = __bizda_fixup(d.bizda);
		break;

		/* these can't be buggered */
	case DT_DAISY:
	case DT_JDN:
	case DT_LDN:
	case DT_MDN:
	default:
		break;
	}
	return d;
}

#endif	/* INCLUDED_date_core_c_ */
/* date-core.c ends here */
