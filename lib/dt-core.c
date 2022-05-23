/*** dt-core.c -- our universe of datetimes
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
/* implementation part of dt-core.h */
#if !defined INCLUDED_dt_core_c_
#define INCLUDED_dt_core_c_

#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include "date-core.h"
#include "time-core.h"
#include "strops.h"
#include "leaps.h"
#include "boops.h"
#include "nifty.h"
#include "dt-core.h"
#include "dt-core-private.h"
#include "date-core.h"
#include "date-core-private.h"
#include "time-core.h"
#include "time-core-private.h"
/* parsers and formatters */
#include "date-core-strpf.h"
#include "time-core-strpf.h"
#if defined SKIP_LEAP_ARITH
# undef WITH_LEAP_SECONDS
#endif	/* SKIP_LEAP_ARITH */
#if defined WITH_LEAP_SECONDS
# include "leap-seconds.h"
#endif	/* WITH_LEAP_SECONDS */

#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

#if !defined DEFUN
# define DEFUN
#endif	/* !DEFUN */

struct strpdt_s {
	struct strpd_s sd;
	struct strpt_s st;
	int64_t i;

	/* use 31 bits for the difference */
	int32_t zdiff:31;
	/* and 1 to indicate if it was specified */
	int32_t zngvn:1;
};

/* used for arithmetic */
struct strpdti_s {
	signed int m;
	signed int d;
	signed int w;
	signed int b;
	signed int S;
};


/* converters and stuff */
#if !defined DT_DAISY_BASE_YEAR
# error daisy base year cannot be obtained
#elif DT_DAISY_BASE_YEAR == 1917
# define DAISY_UNIX_BASE	(19359L)
# define DAISY_GPS_BASE		(23016L)
#elif DT_DAISY_BASE_YEAR == 1753
# define DAISY_UNIX_BASE	(79258L)
# define DAISY_GPS_BASE		(82915L)
#elif DT_DAISY_BASE_YEAR == 1601
# define DAISY_UNIX_BASE	(134775L)
# define DAISY_GPS_BASE		(138432L)
#else
# error unknown daisy base year
#endif	/* DT_DAISY_BASE_YEAR */
#if DAISY_GPS_BASE - DAISY_UNIX_BASE != 3657L
# error daisy unix and gps bases diverge
#endif	/* static assert */

static inline dt_ssexy_t
__to_unix_epoch(struct dt_dt_s dt)
{
/* daisy is competing with the prevalent unix epoch, this is the offset */
	struct dt_d_s dd;
	dt_ssexy_t res;

	if (dt.typ == DT_SEXY) {
		/* no way to find out, is there */
		return dt.sexy;
	} else if (dt_sandwich_p(dt) || dt_sandwich_only_d_p(dt)) {
		dd = dt.d;
	} else if (dt_sandwich_only_t_p(dt)) {
		dd = dt_get_base().d;
	} else {
		return 0;
	}
	res = (dt_conv_to_daisy(dd) - DAISY_UNIX_BASE) * SECS_PER_DAY;
	res += (dt.t.hms.h * 60 + dt.t.hms.m) * 60 + dt.t.hms.s;
	return res;
}

/* public version */
dt_ssexy_t
dt_to_unix_epoch(struct dt_dt_s dt)
{
	return __to_unix_epoch(dt);
}

static inline dt_ssexy_t
__to_gps_epoch(struct dt_dt_s dt)
{
	if (dt.typ == DT_SEXY) {
		/* no way to find out, is there */
		return dt.sexy;
	} else if (dt_sandwich_p(dt) || dt_sandwich_only_d_p(dt)) {
		dt_daisy_t d = dt_conv_to_daisy(dt.d);
		dt_ssexy_t res = (d - DAISY_GPS_BASE) * SECS_PER_DAY;
		if (dt_sandwich_p(dt)) {
			res += (dt.t.hms.h * 60 + dt.t.hms.m) * 60 + dt.t.hms.s;
		}
		return res;
	}
	return 0;
}

/* public version */
dt_ssexy_t
dt_to_gps_epoch(struct dt_dt_s dt)
{
	return __to_gps_epoch(dt);
}

static inline struct dt_dt_s
dt_conv_to_sexy(struct dt_dt_s dt)
{
	if (dt.typ == DT_SEXY) {
		return dt;
	} else if (dt_sandwich_only_t_p(dt)) {
		dt.sxepoch = (dt.t.hms.h * 60 + dt.t.hms.m) * 60 + dt.t.hms.s;
	} else if (dt_sandwich_p(dt) || dt_sandwich_only_d_p(dt)) {
		dt.sxepoch = __to_unix_epoch(dt);
	} else {
		dt = (struct dt_dt_s){DT_UNK};
	}
	/* make sure we hand out sexies */
	dt.typ = DT_SEXY;
	return dt;
}

static inline struct dt_dt_s
__sexy_to_daisy(dt_ssexy_t sx)
{
	struct dt_dt_s res = {DT_UNK};
	int s, m, h;

	s = sx % SECS_PER_MIN;
	sx /= SECS_PER_MIN;
	m = sx % MINS_PER_HOUR;
	sx /= MINS_PER_HOUR;
	h = sx % HOURS_PER_DAY;
	sx /= HOURS_PER_DAY;

	m -= s < 0;
	h -= m < 0;
	sx -= h < 0;

	s += s >= 0 ? 0 : SECS_PER_MIN;
	m += m >= 0 ? 0 : MINS_PER_HOUR;
	h += h >= 0 ? 0 : HOURS_PER_DAY;

	/* assign now */
	res.t.hms.s = s;
	res.t.hms.m = m;
	res.t.hms.h = h;

	/* rest is a day-count, move to daisy */
	res.d.daisy = sx + DAISY_UNIX_BASE;

	/* sandwichify */
	dt_make_sandwich(&res, DT_DAISY, DT_HMS);
	return res;
}

static inline struct dt_dt_s
__ymdhms_to_ymd(dt_ymdhms_t x)
{
	struct dt_dt_s res = {DT_UNK};

	res.t.hms.s = x.S;
	res.t.hms.m = x.M;
	res.t.hms.h = x.H;

	/* rest is a day-count, move to daisy */
	res.d.ymd.y = x.y;
	res.d.ymd.m = x.m;
	res.d.ymd.d = x.d;

	/* sandwichify */
	dt_make_sandwich(&res, DT_YMD, DT_HMS);
	return res;
}

static inline __attribute__((unused)) struct dt_dt_s
__epoch_to_ymd_sandwich(dt_ssexy_t sx)
{
	struct dt_dt_s res;

	res.t.hms.s = sx % SECS_PER_MIN;
	sx /= SECS_PER_MIN;
	res.t.hms.m = sx % MINS_PER_HOUR;
	sx /= MINS_PER_HOUR;
	res.t.hms.h = sx % HOURS_PER_DAY;
	sx /= HOURS_PER_DAY;

	res.d.ymd = __daisy_to_ymd(sx + DAISY_UNIX_BASE);
	dt_make_sandwich(&res, DT_YMD, DT_HMS);
	return res;
}

static inline dt_sexy_t
__sexy_add(dt_sexy_t sx, struct dt_dtdur_s dur)
{
/* sexy add
 * only works for continuous types (DAISY, etc.)
 * we need to take leap seconds into account here */
	dt_ssexy_t dv = dur.dv;

	switch (dur.durtyp) {
	case DT_DURH:
		dv *= MINS_PER_HOUR;
	case DT_DURM:
		dv *= SECS_PER_MIN;
	case DT_DURS:
		break;
	case DT_DURNANO:
		dv /= NANOS_PER_SEC;
		break;
	case DT_DURD:
	case DT_DURBD:
		dv = (dt_ssexy_t)dur.d.dv * SECS_PER_DAY;
		/*@fallthrough@*/
	case DT_DURUNK:
		dv += dur.t.sdur;
	default:
		break;
	}
	/* just go through with it */
	return sx + dv;
}

#if defined WITH_LEAP_SECONDS && defined SKIP_LEAP_ARITH
#error "bugger"
#endif


/* guessing parsers */
#include "token.h"
#include "strops.h"
#if defined WITH_LEAP_SECONDS && defined SKIP_LEAP_ARITH
#error "bugger"
#endif
#include "dt-core-strpf.c"

static const char ymdhms_dflt[] = "%FT%T";
static const char ymcwhms_dflt[] = "%Y-%m-%c-%wT%T";
static const char ywdhms_dflt[] = "%rY-W%V-%uT%T";
static const char ydhms_dflt[] = "%Y-%D";
static const char daisyhms_dflt[] = "%dT%T";
static const char sexy_dflt[] = "%s";
static const char bizsihms_dflt[] = "%dbT%T";
static const char bizdahms_dflt[] = "%Y-%m-%dbT%T";

static const char ymdhmsdur_dflt[] = "%0Y-%0m-%0dT%0H:%0M:%0S";
static const char ymcwhmsdur_dflt[] = "%Y-%0m-%0w-%0dT%0H:%0M:%0S";
static const char ywdhmsdur_dflt[] = "%rY-W%0w-%0dT%0H:%0M:%0S";
static const char ydhmsdur_dflt[] = "%Y-%0dT%0H:%0M:%0S";
static const char daisyhmsdur_dflt[] = "%dT%0H:%0M:%0S";
static const char sexydur_dflt[] = "%s";
static const char bizsihmsdur_dflt[] = "%dbT%0H:%0M:%0S";
static const char bizdahmsdur_dflt[] = "%Y-%0m-%0dbT%0H:%0M:%0S";

DEFUN dt_dttyp_t
__trans_dtfmt(const char **fmt)
{
	if (UNLIKELY(*fmt == NULL)) {
		/* um, great */
		;
	} else if (LIKELY(**fmt == '%')) {
		/* don't worry about it */
		;
	} else {
		const dt_dtyp_t tmp = __trans_dfmt_special(*fmt);

		/* thanks gcc for making me cast this :( */
		switch ((unsigned int)tmp) {
		default:
			break;
		case DT_YMD:
			*fmt = ymdhms_dflt;
			break;
		case DT_YMCW:
			*fmt = ymcwhms_dflt;
			break;
		case DT_BIZDA:
			*fmt = bizdahms_dflt;
			break;
		case DT_DAISY:
			*fmt = daisyhms_dflt;
			break;
		case DT_SEXY:
			*fmt = sexy_dflt;
			break;
		case DT_BIZSI:
			*fmt = bizsihms_dflt;
			break;
		case DT_YWD:
			*fmt = ywdhms_dflt;
			break;
		case DT_YD:
			*fmt = ydhms_dflt;
			break;
		}
		return (dt_dttyp_t)tmp;
	}
	return (dt_dttyp_t)DT_DUNK;
}

DEFUN dt_dtdurtyp_t
__trans_dtdurfmt(const char **fmt)
{
	if (UNLIKELY(*fmt == NULL)) {
		/* um, great */
		;
	} else if (LIKELY(**fmt == '%')) {
		/* don't worry about it */
		;
	} else {
		unsigned int tmp = __trans_dfmt_special(*fmt);

		/* thanks gcc for making me cast this :( */
		switch ((unsigned int)tmp) {
		default:
			break;
		case DT_YMD:
			*fmt = ymdhmsdur_dflt;
			tmp = DT_DURYMD;
			break;
		case DT_YMCW:
			*fmt = ymcwhmsdur_dflt;
			tmp = DT_DURYMCW;
			break;
		case DT_BIZDA:
			*fmt = bizdahmsdur_dflt;
			tmp = DT_DURBIZDA;
			break;
		case DT_DAISY:
			*fmt = daisyhmsdur_dflt;
			tmp = DT_DURD;
			break;
		case DT_SEXY:
			*fmt = sexydur_dflt;
			tmp = DT_DURS;
			break;
		case DT_BIZSI:
			*fmt = bizsihmsdur_dflt;
			tmp = DT_DURBD;
			break;
		case DT_YWD:
			*fmt = ywdhmsdur_dflt;
			tmp = DT_DURYWD;
			break;
		case DT_YD:
			*fmt = ydhmsdur_dflt;
			tmp = DT_DURYD;
			break;
		}
		return (dt_dtdurtyp_t)tmp;
	}
	return (dt_dtdurtyp_t)DT_DURUNK;
}

#define FFFF_GMTIME_SUBDAY
#include "gmtime.h"

static struct timeval
now_tv(void)
{
/* singleton, gives a consistent `now' throughout the whole run */
	static struct timeval tv;

	if (LIKELY(tv.tv_sec)) {
		/* perfect */
		;
	} else if (gettimeofday(&tv, NULL) < 0) {
		/* big cinema :( */
		tv = (struct timeval){0U, 0U};
	}
	return tv;
}

static struct tm
now_tm(void)
{
/* singleton, gives a consistent `now' throughout the whole run */
	static struct tm tm;
	struct timeval tv;

	if (LIKELY(tm.tm_year)) {
		/* sit back and relax */
		;
	} else if ((tv = now_tv()).tv_sec == 0U) {
		/* big cinema :( */
#if defined HAVE_SLOPPY_STRUCTS_INIT
		return (struct tm){0};
#else  /* !HAVE_SLOPPY_STRUCTS_INIT */
		memset(&tm, 0, sizeof(tm));
#endif	/* HAVE_SLOPPY_STRUCTS_INIT */
	} else {
		ffff_gmtime(&tm, tv.tv_sec);
	}
	return tm;
}

static struct strpdt_s
massage_strpdt(struct strpdt_s d)
{
/* the reason we do this separately is that we don't want to bother
 * the pieces of code that use the guesser for different reasons */
	if (UNLIKELY(d.sd.y == 0U)) {
		static const struct strpd_s d0 = {0};
		struct dt_dt_s now = dt_get_base();

		if (UNLIKELY(memcmp(&d.sd, &d0, sizeof(d0)) == 0U)) {
			goto msgg_time;
		}

		d.sd.y = now.d.ymd.y;
		if (LIKELY(d.sd.m)) {
			goto out;
		}
		d.sd.m = now.d.ymd.m;
		if (LIKELY(d.sd.d)) {
			goto out;
		}
		d.sd.d = now.d.ymd.d;

	msgg_time:
		/* same for time values, but obtain those through now_tv() */
		if (UNLIKELY(!d.st.flags.h_set)) {
			d.st.h = now.t.hms.h;
			if (LIKELY(d.st.flags.m_set)) {
				goto out;
			}
			d.st.m = now.t.hms.m;
			if (LIKELY(d.st.flags.s_set)) {
				goto out;
			}
			d.st.s = now.t.hms.s;
			d.st.ns = now.t.hms.ns;
		}
	}
out:
	return d;
}

#if defined WITH_LEAP_SECONDS
static zidx_t
leaps_before(struct dt_dt_s d)
{
	zidx_t res;
	bool on;

	switch (d.typ) {
	case DT_YMD:
		res = leaps_before_ui32(leaps_ymd, nleaps, d.d.ymd.u);
		on = res + 1 < nleaps && leaps_ymd[res + 1] == d.d.ymd.u;
		break;
	case DT_YMCW:
		res = leaps_before_ui32(leaps_ymcw, nleaps, d.d.ymcw.u);
		on = res + 1 < nleaps && leaps_ymcw[res + 1] == d.d.ymcw.u;
		break;
	case DT_DAISY:
		res = leaps_before_ui32(leaps_d, nleaps, d.d.daisy);
		on = res + 1 < nleaps && leaps_d[res + 1] == d.d.daisy;
		break;
	case DT_SEXY:
	case DT_SEXYTAI:
		res = leaps_before_si32(leaps_s, nleaps, (int32_t)d.sexy);
		on = (res + 1U < nleaps) &&
			(leaps_s[res + 1] == (int32_t)d.sexy);
		break;
	default:
		res = 0;
		on = false;
		break;
	}

	/* clang 3.3 will fuck the following up
	 * see http://llvm.org/bugs/show_bug.cgi?id=18028
	 * we have to access d.t.hms.u24 once (the failing access),
	 * then again and magically it'll work, thanks a bunch clang! */
	if (dt_sandwich_p(d) && on) {
#if defined __clang__ && __clang_major__ == 3 && __clang_minor__ >= 3
# warning clang bug! \
see http://llvm.org/bugs/show_bug.cgi?id=18028
		/* access d.t.hms.u24 once */
		volatile unsigned int hms = d.t.hms.u24;

		/* check the time part too */
		if ((hms & 0xffffffU) > leaps_hms[res + 1]) {
			res++;
		}
#else  /* !__clang__ 3.3 */
		/* check the time part too */
		if (d.t.hms.u24 > leaps_hms[res + 1]) {
			res++;
		}
#endif	/* __clang__ 3.3 */
	}
	return res;
}
#endif	/* WITH_LEAP_SECONDS */

static inline int32_t
zdiff_sec(struct dt_dt_s d)
{
/* obtain zdiff in signed seconds, instead of absolute ZDIFF_RES multiples */
	int32_t zdiff = d.zdiff * ZDIFF_RES;

	if (d.neg) {
		zdiff = -zdiff;
	}
	return zdiff;
}

static inline __attribute__((const)) bool
dt_dur_only_d_p(struct dt_dtdur_s d)
{
	return d.durtyp && d.d.durtyp < DT_NDURTYP && !d.t.sdur;
}

static bool
need_milfup_p(const char *fmt)
{
/* military midnights don't need decaying if %T or %H is present */
	for (const char *fp = fmt; *fp;) {
		const char *fp_sav = fp;
		struct dt_spec_s spec = __tok_spec(fp_sav, &fp);

		switch (spec.spfl) {
		case DT_SPFL_N_TSTD:
			return false;
		case DT_SPFL_N_HOUR:
			if (!spec.sc12) {
				return false;
			}
		default:
			break;
		}
	}
	return true;
}


/* parser implementations */
DEFUN struct dt_dt_s
dt_strpdt(const char *str, const char *fmt, char **ep)
{
	struct dt_dt_s res = {DT_UNK};
	struct strpdt_s d = {0};
	const char *sp = str;
	const char *fp;

	if (LIKELY(fmt == NULL)) {
		return __strpdt_std(str, ep);
	}
	/* translate high-level format names, for sandwiches */
	switch ((dt_dtyp_t)__trans_dtfmt(&fmt)) {
		char *on;
	default:
		break;

		/* special case julian/lilian dates as they have
		 * no format specifiers */

	case DT_JDN:
		/* we demand a float representation from start to finish */
		res.d.jdn = (dt_jdn_t)strtod(str, &on);

		if (UNLIKELY(*on < '\0' || *on > ' ')) {
			/* nah, that's not a distinguished float */
			goto fucked;
		}
		/* fix up const-ness problem */
		sp = on;
		/* don't worry about time slot or date/time sandwiches */
		dt_make_d_only(&res, DT_JDN);
		goto sober;

	case DT_LDN:
		res.d.ldn = (dt_ldn_t)strtoi32(str, &sp);
		if (*sp == '.') {
			/* oooh, a double it seems */
			double tmp = strtod(sp, &on);

			/* fix up const-ness problem */
			sp = on;
			/* convert to HMS */
			res.t.hms.h = (tmp *= HOURS_PER_DAY);
			tmp -= (double)res.t.hms.h;
			res.t.hms.m = (tmp *= MINS_PER_HOUR);
			tmp -= (double)res.t.hms.m;
			res.t.hms.s = (tmp *= SECS_PER_MIN);
			tmp -= (double)res.t.hms.s;
			res.t.hms.ns = (tmp *= NANOS_PER_SEC);
			dt_make_sandwich(&res, DT_LDN, DT_HMS);
		} else if (UNLIKELY(*sp < '\0' || *sp > ' ')) {
			/* not on my turf */
			goto fucked;
		} else {
			/* looking good */
			dt_make_d_only(&res, DT_LDN);
		}
		goto sober;

	case DT_MDN:
		res.d.mdn = (dt_ldn_t)strtoi32(str, &sp);
		if (*sp == '.') {
			/* oooh, a double it seems */
			double tmp = strtod(sp, &on);

			/* fix up const-ness problem */
			sp = on;
			/* convert to HMS */
			res.t.hms.h = (tmp *= HOURS_PER_DAY);
			tmp -= (double)res.t.hms.h;
			res.t.hms.m = (tmp *= MINS_PER_HOUR);
			tmp -= (double)res.t.hms.m;
			res.t.hms.s = (tmp *= SECS_PER_MIN);
			tmp -= (double)res.t.hms.s;
			res.t.hms.ns = (tmp *= NANOS_PER_SEC);
			dt_make_sandwich(&res, DT_MDN, DT_HMS);
		} else if (UNLIKELY(*sp < '\0' || *sp > ' ')) {
			/* not on my turf */
			goto fucked;
		} else {
			/* looking good */
			dt_make_d_only(&res, DT_MDN);
		}
		goto sober;
	}

	fp = fmt;
	while (*fp && *sp) {
		const char *fp_sav = fp;
		struct dt_spec_s spec = __tok_spec(fp_sav, &fp);

		if (spec.spfl == DT_SPFL_UNK) {
			/* must be literal */
			if (UNLIKELY(*fp_sav != *sp++)) {
				goto fucked;
			}
		} else if (LIKELY(!spec.rom)) {
			const char *sp_sav = sp;
			if (__strpdt_card(&d, sp, spec, (char**)&sp) < 0) {
				goto fucked;
			}
			if (spec.ord &&
			    __ordinalp(sp_sav, sp - sp_sav, (char**)&sp) < 0) {
				;
			}
			if (spec.bizda) {
				switch (*sp++) {
				case 'B':
					d.sd.flags.ab = BIZDA_BEFORE;
				case 'b':
					d.sd.flags.bizda = 1;
					break;
				default:
					/* it's a bizda anyway */
					d.sd.flags.bizda = 1;
					sp--;
					break;
				}
			}
		} else if (UNLIKELY(spec.rom)) {
			if (__strpd_rom(&d.sd, sp, spec, (char**)&sp) < 0) {
				goto fucked;
			}
		}
	}
	/* check suffix literal */
	if (*fp && *fp != *sp) {
		goto fucked;
	}
	/* check if it's a sexy type */
	if (d.i) {
		res.typ = DT_SEXY;
		res.sexy = d.i;
	} else {
		/* assign d and t types using date and time core routines */
		d = massage_strpdt(d);
		res.d = __guess_dtyp(d.sd);
		res.t = __guess_ttyp(d.st);

		if (res.d.typ > DT_DUNK && res.t.typ > DT_TUNK) {
			res.sandwich = 1;
		} else if (res.d.typ > DT_DUNK) {
			res.t.typ = DT_TUNK;
			res.sandwich = 0;
		} else if (res.t.typ > DT_TUNK) {
			res.d.typ = DT_DUNK;
			res.sandwich = 1;
		}
	}
	if (d.zdiff && dt_sandwich_p(res)) {
		res = __fixup_zdiff(res, d.zdiff);
	} else if (d.zngvn && dt_sandwich_p(res)) {
		res.znfxd = 1;
	}

sober:
	/* set the end pointer */
	if (ep != NULL) {
		*ep = (char*)sp;
	}
	return res;
fucked:
	if (ep != NULL) {
		*ep = (char*)str;
	}
	return (struct dt_dt_s){DT_UNK};
}

DEFUN size_t
dt_strfdt(char *restrict buf, size_t bsz, const char *fmt, struct dt_dt_s that)
{
	struct strpdt_s d = {0};
	const char *fp;
	char *bp;
	dt_dtyp_t tgttyp;
	int set_fmt = 0;

	if (UNLIKELY(buf == NULL || bsz == 0)) {
		bp = buf;
		goto out;
	}

	if (LIKELY(fmt == NULL)) {
		/* um, great */
		set_fmt = 1;
	} else if (LIKELY(*fmt == '%')) {
		/* don't worry about it */
		;
	} else if ((tgttyp = __trans_dfmt_special(fmt)) != (dt_dtyp_t)DT_UNK) {
		that = dt_dtconv((dt_dttyp_t)tgttyp, that);
		set_fmt = 1;
	}

	if (set_fmt && dt_sandwich_p(that)) {
		switch (that.typ) {
		case DT_YMD:
			fmt = ymdhms_dflt;
			break;
		case DT_YMCW:
			fmt = ymcwhms_dflt;
			break;
		case DT_YWD:
			fmt = ywdhms_dflt;
			break;
		case DT_YD:
			fmt = ydhms_dflt;
			break;
		case DT_DAISY:
			/* subject to change */
			fmt = ymdhms_dflt;
			break;
		case DT_JDN:
		case DT_LDN:
		case DT_MDN:
		strf_xian:
			/* short cut, just print the guy here */
			bp = buf + __strfdt_xdn(buf, bsz, that);
			goto out;
		case DT_BIZDA:
			fmt = bizdahms_dflt;
			break;
		case DT_SEXY:
		case DT_YMDHMS:
			fmt = ymdhms_dflt;
			break;
		default:
			/* fuck */
			abort();
			break;
		}
	} else if (set_fmt && dt_sandwich_only_d_p(that)) {
		switch (that.d.typ) {
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
		case DT_JDN:
		case DT_LDN:
		case DT_MDN:
			goto strf_xian;
		default:
			/* fuck */
			abort();
			break;
		}
	} else if (set_fmt && that.typ >= DT_PACK && that.typ < DT_NDTTYP) {
		/* must be sexy or ymdhms */
		fmt = ymdhms_dflt;
	} else if (dt_sandwich_only_t_p(that)) {
		/* transform time specs */
		__trans_tfmt(&fmt);
	}

	/* fix up before printing */
	if (LIKELY(dt_sandwich_p(that) || dt_sandwich_only_d_p(that))) {
		that.d = dt_dfixup(that.d);
	}
	/* make sure we always snarf the zdiff info */
	d.zdiff = zdiff_sec(that);

	if (dt_sandwich_p(that) && UNLIKELY(that.t.hms.h == 24U)) {
		/* military midnight fixup
		 * only when there's %H or %T in the flags, don't decay*/
		if (need_milfup_p(fmt)) {
			that = dt_milfup(that);
		}
	}

	switch (that.typ) {
	case DT_YMD:
	ymd_prep:
		d.sd.y = that.d.ymd.y;
		d.sd.m = that.d.ymd.m;
		d.sd.d = that.d.ymd.d;
		break;
	case DT_YMCW:
		d.sd.y = that.d.ymcw.y;
		d.sd.m = that.d.ymcw.m;
		d.sd.c = that.d.ymcw.c;
		d.sd.w = that.d.ymcw.w;
		break;
	case DT_YWD:
		__prep_strfd_ywd(&d.sd, that.d.ywd);
		break;
	case DT_YD:
		d.sd.y = that.d.yd.y;
		d.sd.d = that.d.yd.d;
		d.sd.flags.d_dcnt_p = 1U;
		break;
	case DT_JDN:
		that = dt_dtconv((dt_dttyp_t)DT_DAISY, that);
		goto daisy_prep;
	case DT_LDN:
		that = dt_dtconv((dt_dttyp_t)DT_DAISY, that);
		goto daisy_prep;
	case DT_MDN:
		that = dt_dtconv((dt_dttyp_t)DT_DAISY, that);
		goto daisy_prep;
	case DT_DAISY:
	daisy_prep:
		__prep_strfd_daisy(&d.sd, that.d.daisy);
		break;

	case DT_BIZDA:
		__prep_strfd_bizda(
			&d.sd, that.d.bizda, __get_bizda_param(that.d));
		break;

	case DT_SEXY:
		/* instead of leaving this as SEXY turn it into
		 * DAISY/HMS sandwich */
		that = dt_dtconv((dt_dttyp_t)DT_DAISY, that);
		/* prep d.sd */
		goto daisy_prep;
	case DT_YMDHMS:
		/* convert this to a YMD/HMS sandwich */
		that = dt_dtconv((dt_dttyp_t)DT_YMD, that);
		/* prep d.sd */
		goto ymd_prep;

	default:
	case DT_DUNK:
		if (!dt_sandwich_only_t_p(that)) {
			bp = buf;
			goto out;
		}
	}

	if (dt_sandwich_p(that) || dt_sandwich_only_t_p(that)) {
		/* cope with the time part */
		d.st.h = that.t.hms.h;
		d.st.m = that.t.hms.m;
		d.st.s = that.t.hms.s;
		d.st.ns = that.t.hms.ns;
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
			bp += __strfdt_card(bp, eo - bp, spec, &d, that);
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
			bp += __strfd_rom(bp, eo - bp, spec, &d.sd, that.d);
		}
	}
out:
	if (bp < buf + bsz) {
		*bp = '\0';
	}
	return bp - buf;
}

DEFUN struct dt_dtdur_s
dt_strpdtdur(const char *str, char **ep)
{
/* at the moment we allow only one format */
	struct dt_dtdur_s res = {(dt_dtdurtyp_t)DT_DURUNK};
	const char *sp;
	long int tmp;

	if ((sp = str) == NULL) {
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

sp:
	switch (*sp++) {
	case '\0':
		/* must have been day then */
		res.d.durtyp = DT_DURD;
		sp--;
		break;
	case 'd':
	case 'D':
		res.d.durtyp = DT_DURD;
		break;
	case 'y':
	case 'Y':
		res.d.durtyp = DT_DURYR;
		break;
	case 'w':
	case 'W':
		res.d.durtyp = DT_DURWK;
		break;
	case 'b':
	case 'B':
		res.d.durtyp = DT_DURBD;
		break;
	case 'q':
	case 'Q':
		res.d.durtyp = DT_DURQU;
		break;

	case 'h':
	case 'H':
		res.durtyp = DT_DURH;
		break;
	case 'm':
	case 'M':
		if (*sp == 'o') {
			/* that makes it a month */
			res.d.durtyp = DT_DURMO;
			sp++;
			break;
		}
		/*@fallthrough@*/
	case '\'':
		res.durtyp = DT_DURM;
		break;
	case 's':
	case 'S':
	case '"':
		res.durtyp = DT_DURS;
		break;

	case 'r':
		/* real seconds/hours/minutes */
		res.tai = 1;
		goto sp;

	case 'n':
		res.durtyp = DT_DURNANO;
		if (*sp == 's') {
			/* nanoseconds, my favourite */
			sp++;
		}
		break;
	default:
		sp = str;
		goto out;
	}
	/* no further checks on tmp */
	if (res.durtyp < (unsigned int)DT_NDURTYP) {
		res.d.dv = tmp;
	} else {
		res.dv = tmp;
	}
out:
	if (ep != NULL) {
		*ep = (char*)sp;
	}
	return res;
}

DEFUN size_t
dt_strfdtdur(
	char *restrict buf, size_t bsz, const char *fmt, struct dt_dtdur_s that)
{
	struct strpdt_s d = {0};
	const char *fp;
	char *bp;

	if (UNLIKELY(buf == NULL || bsz == 0)) {
		bp = buf;
		goto out;
	}

	switch (that.d.durtyp) {
	case DT_YMD:
		d.sd.y = that.d.ymd.y;
		d.sd.m = that.d.ymd.m;
		d.sd.d = that.d.ymd.d;
		if (fmt == NULL && !dt_dur_only_d_p(that)) {
			fmt = ymdhmsdur_dflt;
		} else if (fmt == NULL && dt_dur_only_d_p(that)) {
			fmt = ymddur_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;
	case DT_YMCW:
		d.sd.y = that.d.ymcw.y;
		d.sd.m = that.d.ymcw.m;
		d.sd.c = that.d.ymcw.c;
		d.sd.d = that.d.ymcw.w;
		if (fmt == NULL && !dt_dur_only_d_p(that)) {
			fmt = ymcwhmsdur_dflt;
		} else if (fmt == NULL && dt_dur_only_d_p(that)) {
			fmt = ymcwdur_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;
	case DT_YWD:
		d.sd.y = that.d.ywd.y;
		d.sd.c = that.d.ywd.c;
		d.sd.d = that.d.ywd.w;
		if (fmt == NULL && !dt_dur_only_d_p(that)) {
			fmt = ywdhmsdur_dflt;
		} else if (fmt == NULL && dt_dur_only_d_p(that)) {
			fmt = ywddur_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;
	case DT_YD:
		d.sd.y = that.d.yd.y;
		d.sd.d = that.d.yd.d;
		if (fmt == NULL && !dt_dur_only_d_p(that)) {
			fmt = ydhmsdur_dflt;
		} else if (fmt == NULL && dt_dur_only_d_p(that)) {
			fmt = yddur_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;
	case DT_DURD:
		d.sd.d = that.d.dv;
		if (fmt == NULL) {
			fmt = daisydur_dflt;
		}
		break;
	case DT_DURBD:
		d.sd.d = that.d.dv;
		if (fmt == NULL) {
			/* subject to change */
			fmt = bizsidur_dflt;
		}
		break;
	case DT_BIZDA:;
		dt_bizda_param_t bparam;

		bparam.bs = that.d.param;
		d.sd.y = that.d.bizda.y;
		d.sd.m = that.d.bizda.m;
		d.sd.b = that.d.bizda.bd;
		if (LIKELY(bparam.ab == BIZDA_AFTER)) {
			d.sd.flags.ab = BIZDA_AFTER;
		} else {
			d.sd.flags.ab = BIZDA_BEFORE;
		}
		d.sd.flags.bizda = 1;
		if (fmt == NULL && !dt_dur_only_d_p(that)) {
			fmt = bizdahmsdur_dflt;
		} else if (fmt == NULL && dt_dur_only_d_p(that)) {
			fmt = bizdadur_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;

	default:
	case DT_DUNK:
		break;
	}
	/* translate high-level format names */
	if (!dt_dur_only_d_p(that)) {
		__trans_dtdurfmt(&fmt);
	} else if (dt_dur_only_d_p(that)) {
		__trans_ddurfmt(&fmt);
	} else if (true) {
	try_time:
		fmt = "%S";
	} else {
		bp = buf;
		goto out;
	}

	/* assign and go */
	bp = buf;
	fp = fmt;
	if (that.d.neg) {
		*bp++ = '-';
	}
	for (char *const eo = buf + bsz; *fp && bp < eo;) {
		const char *fp_sav = fp;
		struct dt_spec_s spec = __tok_spec(fp_sav, &fp);

		if (spec.spfl == DT_SPFL_UNK) {
			/* must be literal then */
			*bp++ = *fp_sav;
		} else if (LIKELY(!spec.rom)) {
			bp += __strfdt_dur(bp, eo - bp, spec, &d, that);
			if (spec.bizda) {
				/* don't print the b after an ordinal */
				if (d.sd.flags.ab == BIZDA_AFTER) {
					*bp++ = 'b';
				} else {
					*bp++ = 'B';
				}
			}
		}
	}
out:
	if (bp < buf + bsz) {
		*bp = '\0';
	}
	return bp - buf;
}

DEFUN struct dt_dtdur_s
dt_neg_dtdur(struct dt_dtdur_s dur)
{
	dur.neg = (uint16_t)(~dur.neg & 0x01);
	dur.t.neg = (uint16_t)(~dur.t.neg & 0x01);

	switch (dur.durtyp) {
	case DT_DURD:
	case DT_DURBD:
	case DT_DURWK:
	case DT_DURMO:
	case DT_DURQU:
	case DT_DURYR:
		dur.d.dv = -dur.d.dv;
		dur.t.sdur = -dur.t.sdur;
		break;

	case DT_DURH:
	case DT_DURM:
	case DT_DURS:
	case DT_DURNANO:
		dur.dv = -dur.dv;
		break;

	default:
		break;
	}
	return dur;
}

DEFUN int
dt_dtdur_neg_p(struct dt_dtdur_s dur)
{
	switch (dur.durtyp) {
	case DT_DURD:
	case DT_DURBD:
	case DT_DURWK:
	case DT_DURMO:
	case DT_DURQU:
	case DT_DURYR:
		return dur.d.dv < 0 || (!dur.d.dv && dur.t.sdur < 0);

	case DT_DURH:
	case DT_DURM:
	case DT_DURS:
	case DT_DURNANO:
		return dur.dv < 0;

	default:
		break;
	}
	return dur.neg;
}


/* date getters, platform dependent */
DEFUN struct dt_dt_s
dt_datetime(dt_dttyp_t outtyp)
{
	struct dt_dt_s res = {DT_UNK};
	const dt_dtyp_t outdtyp = (dt_dtyp_t)outtyp;
	struct tm tm = now_tm();
	struct timeval tv = now_tv();

	switch (outdtyp) {
	case DT_YMD:
	case DT_YMCW:
	case DT_YD:
		switch (outdtyp) {
		case DT_YMD:
			res.d.ymd.y = tm.tm_year;
			res.d.ymd.m = tm.tm_mon;
			res.d.ymd.d = tm.tm_mday;
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
			res.d.ymcw.y = tm.tm_year;
			res.d.ymcw.m = tm.tm_mon;
			res.d.ymcw.c = __ymd_get_count(tmp);
			res.d.ymcw.w = tm.tm_wday;
			break;
		}
		case DT_YD:
			res.d.yd.y = tm.tm_year;
			res.d.yd.d = tm.tm_yday;
			break;
		default:
			/* grrrr */
			;
		}
		break;

	case DT_YWD:
		/* use ordinary conversion to ywd */
		res.d.typ = DT_YMD;
		res.d.ymd.y = tm.tm_year;
		res.d.ymd.m = tm.tm_mon;
		res.d.ymd.d = tm.tm_mday;
		res.d = dt_dconv(DT_YWD, res.d);
		break;

	case DT_DAISY:
		/* time_t's base is 1970-01-01, which is daisy 19359 */
		res.d.daisy = tv.tv_sec / (unsigned int)SECS_PER_DAY
			+ DAISY_UNIX_BASE;
		break;

	case DT_BIZDA:
	case DT_BIZSI:
		/* could be an idea to have those, innit? */

	default:
	case DT_DUNK:
		break;
	}

	/* time assignment */
	if (outdtyp <= DT_NDTYP) {
		res.t.hms.h = tm.tm_hour;
		res.t.hms.m = tm.tm_min;
		res.t.hms.s = tm.tm_sec;
		res.t.hms.ns = tv.tv_usec * 1000;
		dt_make_sandwich(&res, (dt_dtyp_t)outtyp, DT_HMS);
	} else {
		/* must be one of the sexies then, aye? */
		res.sexy = tv.tv_sec;
	}
	return res;
}

DEFUN struct dt_dt_s
dt_dtconv(dt_dttyp_t tgttyp, struct dt_dt_s d)
{
	if (dt_sandwich_p(d) || dt_sandwich_only_d_p(d)) {
		/* thanks gcc for making me cast tgttyp */
		switch ((unsigned int)tgttyp) {
			short unsigned int sw;
		case DT_YMD:
		case DT_YMCW:
		case DT_BIZDA:
		case DT_DAISY:
		case DT_BIZSI:
		case DT_YWD:
		case DT_YD:
		case DT_JDN:
		case DT_LDN:
		case DT_MDN:
			/* backup sandwich state */
			sw = d.sandwich;
			/* convert */
			d.d = dt_dconv((dt_dtyp_t)tgttyp, d.d);
			/* restore sandwich state */
			d.sandwich = sw;
			break;
		case DT_SEXY:
		case DT_SEXYTAI: {
			dt_daisy_t dd = dt_conv_to_daisy(d.d);
			unsigned int ss = __secs_since_midnight(d.t);

			switch (tgttyp) {
				int64_t sx;
#if defined WITH_LEAP_SECONDS
			case DT_SEXYTAI: {
				zidx_t zi;

				sx = (dd - DAISY_UNIX_BASE) * SECS_PER_DAY + ss;
				zi = leaps_before_si32(leaps_s, nleaps, sx);
				d.sexy = sx + leaps_corr[zi];
				break;
			}
#else  /* !WITH_LEAP_SECONDS */
			case DT_SEXYTAI:
#endif	/* WITH_LEAP_SECONDS */
			case DT_SEXY:
				sx = (dd - DAISY_UNIX_BASE) * SECS_PER_DAY + ss;
				d.sexy = sx;
				break;
			default:
				/* grrrr */
				;
			}
			d.sandwich = 0;
			d.typ = tgttyp;
			break;
		}
		case DT_YMDHMS:
			/* no support for this guy yet */

		case DT_DUNK:
		default:
			d = (struct dt_dt_s){DT_UNK};
			break;
		}
	} else if (dt_sandwich_only_t_p(d)) {
		/* ah, how good is that? */
		;
	} else if (!dt_separable_p(d)) {
		switch (d.typ) {
		case DT_SEXY:
		case DT_SEXYTAI:
			if (tgttyp > DT_UNK && tgttyp < DT_PACK) {
				/* go through daisy */
				d = __sexy_to_daisy(d.sxepoch);
				d.d = dt_dconv((dt_dtyp_t)tgttyp, d.d);
				d.sandwich = 1U;
			} else if (tgttyp == DT_YMDHMS) {
				;
			}
			break;
		case DT_YMDHMS:
			if (tgttyp > DT_UNK && tgttyp < DT_PACK) {
				/* go through ymd */
				d = __ymdhms_to_ymd(d.ymdhms);
				d.d = dt_dconv((dt_dtyp_t)tgttyp, d.d);
				d.sandwich = 1U;
			} else if (tgttyp == DT_SEXY) {
				;
			}
			break;
		default:
			d = (struct dt_dt_s){DT_UNK};
			break;
		}
	} else {
		/* great, what now? */
		;
	}
	return d;
}

DEFUN struct dt_dt_s
dt_dtadd(struct dt_dt_s d, struct dt_dtdur_s dur)
{
/* we decompose the problem like so:
 * carry <- dpart(dur);
 * tpart(res), carry <- tadd(d, tpart(dur), corr);
 * res <- dadd(dpart(res), carry); */
	dt_ssexy_t dv;
#if defined WITH_LEAP_SECONDS
	struct dt_dt_s orig;
	const bool tai_fixup_p = dur.tai && !dt_sandwich_only_d_p(d);

	if (UNLIKELY(tai_fixup_p)) {
		/* make a copy */
		orig = d;
	}
#endif	/* WITH_LEAP_SECONDS */

	if (d.typ == DT_SEXY) {
		d.sexy = __sexy_add(d.sexy, dur);
		return d;
	}

	dv = dur.dv;
	switch (dur.durtyp) {
	default:
	dadd:
		/* let date-core's dt_dadd() do the yakka */
		d.d = dt_dadd(d.d, dur.d);
		break;

	case DT_DURH:
		dv *= MINS_PER_HOUR;
		/*@fallthrough@*/
	case DT_DURM:
		dv *= SECS_PER_MIN;
		/*@fallthrough@*/
	case DT_DURS:;
		int carry;

	tadd:
		/* make sure we don't blow the carry slot */
		carry = dv / (signed int)SECS_PER_DAY;
		dv = dv % (signed int)SECS_PER_DAY;

		if (d.sandwich) {
			/* accept both t-onlies and sandwiches */
			d.t = dt_tadd_s(d.t, dv, 0);

			if ((carry += d.t.carry) && !dt_sandwich_only_t_p(d)) {
				/* add some days as well */
				dur.d.durtyp = DT_DURD;
				dur.d.dv = carry;
				goto dadd;
			}
		}
		break;

	case DT_DURNANO:;
		/* quickly do the addition ourselves */
		dv += d.t.hms.ns;
		carry = dv / (signed int)NANOS_PER_SEC;
		if ((dv = dv % (signed int)NANOS_PER_SEC) < 0) {
			dv += NANOS_PER_SEC;
			carry--;
		}

		if (d.sandwich) {
			d.t.hms.ns = dv;
		}

		/* the rest is normal second-wise tadd */
		dv = carry;
		goto tadd;
	}

#if defined WITH_LEAP_SECONDS
	if (UNLIKELY(tai_fixup_p)) {
		/* the reason we omitted SEXY is because there's simply
		 * no representation in there */
		zidx_t i_orig = leaps_before(orig);
		zidx_t i_d = leaps_before(d);

		if (UNLIKELY(i_orig != i_d)) {
			/* insert leaps */
			int nltr = leaps_corr[i_orig] - leaps_corr[i_d];

			/* save a copy of d again */
			orig = d;
			i_orig = i_d;
			if (nltr &&
			    /* get our special tadd with carry */
			    (d.t = dt_tadd_s(d.t, nltr, 0), d.t.carry)) {
				/* great, we need to sub/add again
				 * as there's been a wrap-around at
				 * midnight, spooky */
				dur.d.durtyp = DT_DURD;
				dur.d.dv = d.t.carry;
				d.d = dt_dadd(d.d, dur.d);
			}

			/* check if we transitioned again */
			if (d.typ == DT_SEXYTAI || (i_d = leaps_before(d), 0)) {
				/* don't have to */
				;
			} else if (UNLIKELY(i_d < i_orig)) {
				d.t.hms.s -= nltr;
			} else if (UNLIKELY(i_d > i_orig)) {
				d = orig;
				d.t.hms.s += nltr;
			}
		}
	}
#endif	/* WITH_LEAP_SECONDS */
	return d;
}

DEFUN struct dt_dtdur_s
dt_dtdiff(dt_dtdurtyp_t tgttyp, struct dt_dt_s d1, struct dt_dt_s d2)
{
	struct dt_dtdur_s res = {(dt_dtdurtyp_t)DT_DURUNK};
	int64_t dt = 0;

	if (!dt_sandwich_only_d_p(d1) && !dt_sandwich_only_d_p(d2)) {
		/* do the time portion difference right away */
		switch (tgttyp) {
		default:
			dt = dt_tdiff_s(d1.t, d2.t);
			break;
		case DT_DURNANO:
			dt = dt_tdiff_ns(d1.t, d2.t);
			break;
		}
	}
	/* now assess what else is to be done */
	if (dt_sandwich_only_t_p(d1) && dt_sandwich_only_t_p(d2)) {
		/* make t-only */
		res.durtyp = (dt_dtdurtyp_t)(DT_DURS + (tgttyp == DT_DURNANO));
		res.neg = (uint16_t)(dt < 0);
		res.dv = dt >= 0 ? dt : -dt;
	} else if (tgttyp && (dt_durtyp_t)tgttyp < DT_NDURTYP) {
		res.durtyp = tgttyp;
		res.d = dt_ddiff((dt_durtyp_t)tgttyp, d1.d, d2.d, dt);
		dt = !res.neg ? dt : -dt;
		dt = !res.d.fix ? dt
			: dt > 0 ? SECS_PER_DAY - dt
			: dt < 0 ? dt + SECS_PER_DAY
			: 0;
		res.t.sdur = dt;
		res.t.nsdur = 0;
	} else if ((dt_durtyp_t)tgttyp >= DT_NDURTYP) {
		int64_t sxdur;

		if (d1.typ < DT_PACK && d2.typ < DT_PACK) {
			/* go for tdiff and ddiff independently */
			res.d = dt_ddiff(DT_DURD, d1.d, d2.d, 0);

			if (UNLIKELY(tgttyp == DT_DURNANO)) {
				/* unfortunately we have to scale back */
				dt /= NANOS_PER_SEC;
			}
			/* since target type is SEXY do the conversion here */
			sxdur = dt + (int64_t)res.d.dv * SECS_PER_DAY;
		} else {
			/* oh we're in the sexy domain already,
			 * note, we can't diff ymdhms packs */
			d1 = dt_dtconv(DT_SEXY, d1);
			d2 = dt_dtconv(DT_SEXY, d2);

			/* now it's fuck-easy */
			sxdur = (int64_t)(d2.sexy - d1.sexy);
		}

		/* set up the output here */
		res.durtyp = DT_DURS;
		res.neg = 0U;
		if (LIKELY(tgttyp < DT_NDTDURTYP)) {
			res.tai = 0U;
		} else {
			/* just a hack to transfer the notion of TAIness */
			res.tai = 1U;
		}
		res.dv = sxdur;

#if defined WITH_LEAP_SECONDS
		if (tgttyp >= DT_NDTDURTYP) {
			/* just a hack to transfer the notion of TAIness
			 * check for transitions */
			zidx_t i_d1 = leaps_before(d1);
			zidx_t i_d2 = leaps_before(d2);

# if BYTE_ORDER == BIG_ENDIAN
			/* not needed on little-endians
			 * the little means just that */
			res.soft = sxdur;
# elif BYTE_ORDER == LITTLE_ENDIAN

# else
#  warning unknown byte order
# endif	/* BYTE_ORDER */

			if (UNLIKELY(i_d1 != i_d2)) {
				int nltr = leaps_corr[i_d2] - leaps_corr[i_d1];

				res.corr = nltr;
# if BYTE_ORDER == BIG_ENDIAN
			} else {
				/* always repack res.corr to remove clutter
				 * from the earlier res.sexydur ass'ment */
				res.corr = 0;
# elif BYTE_ORDER == LITTLE_ENDIAN

# else
#  warning unknown byte order
# endif	 /* BYTE_ORDER */
			}
		}
#endif	/* WITH_LEAP_SECONDS */
	}
	return res;
}


DEFUN int
dt_dtcmp(struct dt_dt_s d1, struct dt_dt_s d2)
{
/* for the moment D1 and D2 have to be of the same type. */
	if (UNLIKELY(d1.typ != d2.typ)) {
		/* always equal */
		return -2;
	}
	/* go through it hierarchically and without upmotes */
	switch (d1.d.typ) {
		int res;

	case DT_DUNK:
	default:
		goto try_time;
	case DT_YMD:
	case DT_DAISY:
	case DT_BIZDA:
	case DT_YWD:
	case DT_YD:
		/* use arithmetic comparison */
		if (d1.d.u < d2.d.u) {
			return -1;
		} else if (d1.d.u > d2.d.u) {
			return 1;
		} else {
			/* means they're equal, so try the time part */
			goto try_time;
		}
	case DT_YMCW:
		/* use designated thing since ymcw dates aren't
		 * increasing */
		if (!(res = __ymcw_cmp(d1.d.ymcw, d2.d.ymcw))) {
			goto try_time;
		}
		return res;
	}
try_time:
	if (d1.t.hms.u < d2.t.hms.u) {
		return -1;
	} else if (d1.t.hms.u > d2.t.hms.u) {
		return 1;
	}
	return 0;
}

DEFUN int
dt_dt_in_range_p(struct dt_dt_s d, struct dt_dt_s d1, struct dt_dt_s d2)
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
	const unsigned int i = (dt_dtcmp(d, d1) + 2) & 0b11U;
	const unsigned int j = (dt_dtcmp(d, d2) + 2) & 0b11U;
	return 2 - ((m >> (i * 8U + j * 2U)) & 0b11U);
}

#if defined __INTEL_COMPILER
# pragma warning (default:2203)
#elif defined __GNUC__
# pragma GCC diagnostic warning "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

static struct dt_dt_s base;

DEFUN void
dt_set_base(struct dt_dt_s dt)
{
	if (UNLIKELY(dt.typ != (dt_dttyp_t)DT_YMD)) {
		dt = dt_dtconv((dt_dttyp_t)DT_YMD, dt);
	}
	base = dt;
	return;
}

DEFUN struct dt_dt_s
dt_get_base(void)
{
	if (UNLIKELY(base.typ == DT_UNK)) {
		/* singleton */
		base = dt_datetime((dt_dttyp_t)DT_YMD);
	}
	return base;
}

#if defined LIBDUT
# if defined INCLUDED_date_core_h_
/* service routines */
DEFUN struct dt_d_s
dt_get_dbase(void)
{
	struct dt_dt_s b = dt_get_base();
	return b.d;
}
# endif	/* INCLUDED_date_core_h_ */

# if defined INCLUDED_time_core_h_
/* service routines */
DEFUN struct dt_t_s
dt_get_tbase(void)
{
	struct dt_dt_s b = dt_get_base();
	return b.t;
}
# endif	/* INCLUDED_time_core_h_ */
#endif	/* LIBDUT */

DEFUN __attribute__((const)) struct dt_dt_s
dt_fixup(struct dt_dt_s d)
{
	if (LIKELY(dt_sandwich_only_d_p(d) || dt_sandwich_p(d))) {
		d.d = dt_dfixup(d.d);
	}
	return d;
}

DEFUN __attribute__((const)) struct dt_dt_s
dt_milfup(struct dt_dt_s dt)
{
	if (dt_sandwich_p(dt) && UNLIKELY(dt.t.hms.h == 24)) {
		dt = dt_dtadd(dt, (struct dt_dtdur_s){DT_DURS});
	}
	return dt;
}

#endif	/* INCLUDED_date_core_c_ */
/* dt-core.c ends here */
