/*** dt-core.c -- our universe of datetimes
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
#include "nifty.h"
#include "dt-core.h"
#include "date-core.h"
#include "time-core.h"
/* parsers and formatters */
#include "date-core-strpf.h"
#include "time-core-strpf.h"
#if defined SKIP_LEAP_ARITH
# undef WITH_LEAP_SECONDS
#endif	/* SKIP_LEAP_ARITH */
#if defined WITH_LEAP_SECONDS
# include "leapseconds.h"
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
	long int i;

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
# define DAISY_UNIX_BASE	(19359)
# define DAISY_GPS_BASE		(23016)
#elif DT_DAISY_BASE_YEAR == 1753
# define DAISY_UNIX_BASE	(79258)
# define DAISY_GPS_BASE		(82915)
#elif DT_DAISY_BASE_YEAR == 1601
# define DAISY_UNIX_BASE	(134775)
# define DAISY_GPS_BASE		(138432)
#else
# error unknown daisy base year
#endif	/* DT_DAISY_BASE_YEAR */
#if DAISY_GPS_BASE - DAISY_UNIX_BASE != 3657
# error daisy unix and gps bases diverge
#endif	/* static assert */

static inline dt_ssexy_t
__to_unix_epoch(struct dt_dt_s dt)
{
/* daisy is competing with the prevalent unix epoch, this is the offset */
	if (dt.typ == DT_SEXY) {
		/* no way to find out, is there */
		return dt.sexy;
	} else if (dt_sandwich_p(dt) || dt_sandwich_only_d_p(dt)) {
		dt_daisy_t d = dt_conv_to_daisy(dt.d);
		dt_ssexy_t res = (d - DAISY_UNIX_BASE) * SECS_PER_DAY;
		if (dt_sandwich_p(dt)) {
			res += (dt.t.hms.h * 60 + dt.t.hms.m) * 60 + dt.t.hms.s;
		}
		return res;
	}
	return 0;
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
		dt = dt_dt_initialiser();
	}
	/* make sure we hand out sexies */
	dt.typ = DT_SEXY;
	return dt;
}

static inline dt_ymdhms_t
__epoch_to_ymdhms(dt_ssexy_t sx)
{
	dt_ymdhms_t res;

	res.S = sx % SECS_PER_MIN;
	sx /= SECS_PER_MIN;
	res.M = sx % MINS_PER_HOUR;
	sx /= MINS_PER_HOUR;
	res.H = sx % HOURS_PER_DAY;
	sx /= HOURS_PER_DAY;

	{
		dt_ymd_t tmp = __daisy_to_ymd(sx + DAISY_UNIX_BASE);
		res.y = tmp.y;
		res.m = tmp.m;
		res.d = tmp.d;
	}
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
__sexy_add(dt_sexy_t sx, struct dt_dt_s dur)
{
/* sexy add
 * only works for continuous types (DAISY, etc.)
 * we need to take leap seconds into account here */
	signed int delta = 0;

	switch (dur.d.typ) {
	case DT_SEXY:
	case DT_SEXYTAI:
		delta = dur.sexydur;
		break;
	case DT_DAISY:
		delta = dur.d.daisydur * SECS_PER_DAY;
	case DT_DUNK:
		delta += dur.t.sdur;
	default:
		break;
	}
	/* just go through with it */
	return sx + delta;
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
static const char ydhms_dflt[] = "%Y-%d";
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

DEFUN void
__trans_dtfmt(const char **fmt)
{
	if (UNLIKELY(*fmt == NULL)) {
		/* um, great */
		;
	} else if (LIKELY(**fmt == '%')) {
		/* don't worry about it */
		;
	} else {
		dt_dtyp_t tmp = __trans_dfmt_special(*fmt);

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
	}
	return;
}

DEFUN void
__trans_dtdurfmt(const char **fmt)
{
	if (UNLIKELY(*fmt == NULL)) {
		/* um, great */
		;
	} else if (LIKELY(**fmt == '%')) {
		/* don't worry about it */
		;
	} else {
		dt_dtyp_t tmp = __trans_dfmt_special(*fmt);

		/* thanks gcc for making me cast this :( */
		switch ((unsigned int)tmp) {
		default:
			break;
		case DT_YMD:
			*fmt = ymdhmsdur_dflt;
			break;
		case DT_YMCW:
			*fmt = ymcwhmsdur_dflt;
			break;
		case DT_BIZDA:
			*fmt = bizdahmsdur_dflt;
			break;
		case DT_DAISY:
			*fmt = daisyhmsdur_dflt;
			break;
		case DT_SEXY:
			*fmt = sexydur_dflt;
			break;
		case DT_BIZSI:
			*fmt = bizsihmsdur_dflt;
			break;
		case DT_YWD:
			*fmt = ywdhmsdur_dflt;
			break;
		case DT_YD:
			*fmt = ydhmsdur_dflt;
			break;
		}
	}
	return;
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
		return (struct tm){};
#else  /* !HAVE_SLOPPY_STRUCTS_INIT */
		memset(&tm, 0, sizeof(tm));
#endif	/* HAVE_SLOPPY_STRUCTS_INIT */
	} else {
		ffff_gmtime(&tm, tv.tv_sec);
	}
	return tm;
}

static struct tm
dflt_tm(const struct dt_dt_s *set)
{
/* getter/setter and singleton for SET == NULL */
	static struct tm tm;

	if (LIKELY(set == NULL && tm.tm_year != 0U)) {
		/* show what we've got */
		;
	} else if (set == NULL) {
		/* take over the value of now */
		tm = now_tm();
	} else {
		switch (set->typ) {
		case DT_YMD:
			tm.tm_year = set->d.ymd.y;
			tm.tm_mon = set->d.ymd.m;
			tm.tm_mday = set->d.ymd.d;
			tm.tm_hour = set->t.hms.h;
			tm.tm_min = set->t.hms.m;
			tm.tm_sec = set->t.hms.s;
			break;
		default:
			/* good question */
#if defined HAVE_SLOPPY_STRUCTS_INIT
			return (struct tm){};
#else  /* !HAVE_SLOPPY_STRUCTS_INIT */
			memset(&tm, 0, sizeof(tm));
			break;
#endif	/* HAVE_SLOPPY_STRUCTS_INIT */
		}
	}
	return tm;
}

static struct strpdt_s
massage_strpdt(struct strpdt_s d)
{
/* the reason we do this separately is that we don't want to bother
 * the pieces of code that use the guesser for different reasons */
	if (UNLIKELY(d.sd.y == 0U)) {
#if defined HAVE_SLOPPY_STRUCTS_INIT
		static const struct strpd_s d0 = {};
#else  /* !HAVE_SLOPPY_STRUCTS_INIT */
		static const struct strpd_s d0;
#endif	/* HAVE_SLOPPY_STRUCTS_INIT */
		struct tm now = dflt_tm(NULL);

		if (UNLIKELY(memcmp(&d.sd, &d0, sizeof(d0)) == 0U)) {
			goto msgg_time;
		}

		d.sd.y = now.tm_year;
		if (LIKELY(d.sd.m)) {
			goto out;
		}
		d.sd.m = now.tm_mon;
		if (LIKELY(d.sd.d)) {
			goto out;
		}
		d.sd.d = now.tm_mday;

	msgg_time:
		/* same for time values, but obtain those through now_tv() */
		if (UNLIKELY(!d.st.flags.h_set)) {
			d.st.h = now.tm_hour;
			if (LIKELY(d.st.flags.m_set)) {
				goto out;
			}
			d.st.m = now.tm_min;
			if (LIKELY(d.st.flags.s_set)) {
				goto out;
			}
			d.st.s = now.tm_sec;
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


/* parser implementations */
DEFUN struct dt_dt_s
dt_strpdt(const char *str, const char *fmt, char **ep)
{
	struct dt_dt_s res = dt_dt_initialiser();
	struct strpdt_s d;
	const char *sp = str;
	const char *fp = fmt;

	if (LIKELY(fmt == NULL)) {
		return __strpdt_std(str, ep);
	}
	/* translate high-level format names, for sandwiches */
	__trans_dtfmt(&fmt);

	d = strpdt_initialiser();
	while (*fp && *sp) {
		const char *fp_sav = fp;
		struct dt_spec_s spec = __tok_spec(fp_sav, &fp);

		if (spec.spfl == DT_SPFL_UNK) {
			/* must be literal */
			if (*fp_sav != *sp++) {
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

	/* set the end pointer */
	if (ep != NULL) {
		*ep = (char*)sp;
	}
	return res;
fucked:
	if (ep != NULL) {
		*ep = (char*)str;
	}
	return dt_dt_initialiser();
}

DEFUN size_t
dt_strfdt(char *restrict buf, size_t bsz, const char *fmt, struct dt_dt_s that)
{
	struct strpdt_s d = strpdt_initialiser();
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
		case DT_DAISY:
			/* subject to change */
			fmt = ymdhms_dflt;
			break;
		case DT_JDN:
		case DT_LDN:
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
		case DT_DAISY:
			/* subject to change */
			fmt = ymd_dflt;
			break;
		case DT_BIZDA:
			fmt = bizda_dflt;
			break;
		case DT_JDN:
		case DT_LDN:
			/* short cut, print the guy in here */
			bp = buf + __strfdt_xdn(buf, bsz, that);
			goto out;
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

	switch (that.typ) {
	case DT_YMD:
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
	case DT_DAISY:
		__prep_strfd_daisy(&d.sd, that.d.daisy);
		break;

	case DT_BIZDA:
		__prep_strfd_bizda(
			&d.sd, that.d.bizda, __get_bizda_param(that.d));
		break;

	case DT_SEXY: {
		dt_ymdhms_t tmp = __epoch_to_ymdhms(that.sxepoch);
		d.st.h = tmp.H;
		d.st.m = tmp.M;
		d.st.s = tmp.S;
		d.sd.y = tmp.y;
		d.sd.m = tmp.m;
		d.sd.d = tmp.d;
		break;
	}
	case DT_YMDHMS:
		break;
	default:
	case DT_DUNK:
	case DT_LDN:
	case DT_JDN:
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
		d.zdiff = zdiff_sec(that);
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

DEFUN struct dt_dt_s
dt_strpdtdur(const char *str, char **ep)
{
/* at the moment we allow only one format */
	struct dt_dt_s res = dt_dt_initialiser();
	const char *sp;
	long int tmp;
	struct strpdt_s d;

	if (str == NULL) {
		goto out;
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

	d = strpdt_initialiser();
sp:
	switch (*sp++) {
	case '\0':
		/* must have been day then */
		d.sd.d = tmp;
		sp--;
		break;
	case 'd':
	case 'D':
		d.sd.d = tmp;
		break;
	case 'y':
	case 'Y':
		d.sd.y = tmp;
		break;
	case 'm':
	case 'M':
		d.sd.m = tmp;
		if (*sp == 'o') {
			/* that makes it unique */
			sp++;
			break;
		}
	case '\'':
		/* could stand for minute, so just to be sure ... */
		d.st.m = tmp;
		break;
	case 'w':
	case 'W':
		d.sd.w = tmp;
		break;
	case 'b':
	case 'B':
		d.sd.b = tmp;
		break;
	case 'q':
	case 'Q':
		d.sd.q = tmp;
		break;

	case 'h':
	case 'H':
		d.st.h = tmp;
		break;
	case 's':
	case 'S':
	case '"':
		/* could also stand for second, so update this as well */
		d.st.s = tmp;
		break;

	case 'r':
		/* real seconds */
		res.tai = 1;
		goto sp;
	default:
		sp = str;
		goto out;
	}
	/* assess */
	if (d.sd.b && (d.sd.m || d.sd.y)) {
		res.d.typ = DT_BIZDA;
		res.d.bizda.y = d.sd.y;
		res.d.bizda.m = d.sd.q * 3 + d.sd.m;
		res.d.bizda.bd = d.sd.b + d.sd.w * 5;
	} else if (d.sd.y || (d.sd.m && !d.st.m)) {
	dflt:
		res.d.typ = DT_YMD;
		res.d.ymd.y = d.sd.y;
		res.d.ymd.m = d.sd.q * 3 + d.sd.m;
		res.d.ymd.d = d.sd.d + d.sd.w * 7;
	} else if (d.sd.b) {
		res.d.typ = DT_BIZSI;
		res.d.bizsi = d.sd.w * 5 + d.sd.b;
	} else if (d.sd.d || d.sd.w) {
		res.d.typ = DT_DAISY;
		res.d.daisy = d.sd.w * 7 + d.sd.d;

/* time specs here */
	} else if (d.st.h || d.st.m || d.st.s) {
		/* treat as m for minute */
		dt_make_t_only(&res, DT_HMS);
		res.t.dur = 1;
		res.t.sdur = d.st.h * SECS_PER_HOUR +
			d.st.m * SECS_PER_MIN +
			d.st.s;
		/* but also put the a note in the md slot */
		if (!d.st.h && !d.st.s && d.sd.m) {
			res.d.md.m = d.sd.m;
		}

	} else {
		/* we leave out YMCW diffs simply because YMD diffs
		 * cover them better */
		goto dflt;
	}

out:
	if (ep != NULL) {
		*ep = (char*)sp;
	}
	res.dur = 1;
	return res;
}

DEFUN size_t
dt_strfdtdur(
	char *restrict buf, size_t bsz, const char *fmt, struct dt_dt_s that)
{
	struct strpdt_s d;
	const char *fp;
	char *bp;

	if (UNLIKELY(buf == NULL || bsz == 0)) {
		bp = buf;
		goto out;
	}

	d = strpdt_initialiser();
	switch (that.d.typ) {
	case DT_YMD:
		d.sd.y = that.d.ymd.y;
		d.sd.m = that.d.ymd.m;
		d.sd.d = that.d.ymd.d;
		if (fmt == NULL && dt_sandwich_p(that)) {
			fmt = ymdhmsdur_dflt;
		} else if (fmt == NULL && dt_sandwich_only_d_p(that)) {
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
		if (fmt == NULL && dt_sandwich_p(that)) {
			fmt = ymcwhmsdur_dflt;
		} else if (fmt == NULL && dt_sandwich_only_d_p(that)) {
			fmt = ymcwdur_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;
	case DT_YWD:
		d.sd.y = that.d.ywd.y;
		d.sd.c = that.d.ywd.c;
		d.sd.d = that.d.ywd.w;
		if (fmt == NULL && dt_sandwich_p(that)) {
			fmt = ywdhmsdur_dflt;
		} else if (fmt == NULL && dt_sandwich_only_d_p(that)) {
			fmt = ywddur_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;
	case DT_YD:
		d.sd.y = that.d.yd.y;
		d.sd.d = that.d.yd.d;
		if (fmt == NULL && dt_sandwich_p(that)) {
			fmt = ydhmsdur_dflt;
		} else if (fmt == NULL && dt_sandwich_only_d_p(that)) {
			fmt = yddur_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;
	case DT_DAISY:
		d.sd.d = that.d.daisy;
		if (fmt == NULL && dt_sandwich_p(that)) {
			fmt = daisydur_dflt;
		} else if (fmt == NULL && dt_sandwich_only_d_p(that)) {
			fmt = daisydur_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;
	case DT_BIZSI:
		d.sd.d = that.d.bizsi;
		if (fmt == NULL && dt_sandwich_p(that)) {
			/* subject to change */
			fmt = bizsidur_dflt;
		} else if (fmt == NULL && dt_sandwich_only_d_p(that)) {
			fmt = bizsidur_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;
	case DT_BIZDA: {
		dt_bizda_param_t bparam = __get_bizda_param(that.d);
		d.sd.y = that.d.bizda.y;
		d.sd.m = that.d.bizda.m;
		d.sd.b = that.d.bizda.bd;
		if (LIKELY(bparam.ab == BIZDA_AFTER)) {
			d.sd.flags.ab = BIZDA_AFTER;
		} else {
			d.sd.flags.ab = BIZDA_BEFORE;
		}
		d.sd.flags.bizda = 1;
		if (fmt == NULL && dt_sandwich_p(that)) {
			fmt = bizdahmsdur_dflt;
		} else if (fmt == NULL && dt_sandwich_only_d_p(that)) {
			fmt = bizdadur_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;
	}
	default:
	case DT_DUNK:
		break;
	}
	/* translate high-level format names */
	if (dt_sandwich_p(that)) {
		__trans_dtdurfmt(&fmt);
	} else if (dt_sandwich_only_d_p(that)) {
		__trans_ddurfmt(&fmt);
	} else if (dt_sandwich_only_t_p(that)) {
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

DEFUN struct dt_dt_s
dt_neg_dtdur(struct dt_dt_s dur)
{
	dur.neg = (uint16_t)(~dur.neg & 0x01);
	dur.t.neg = (uint16_t)(~dur.t.neg & 0x01);

	/* treat daisy and bizsi durs specially */
	switch (dur.d.typ) {
	case DT_DAISY:
		dur.d.daisydur = -dur.d.daisydur;
		break;
	case DT_BIZSI:
		dur.d.bizsidur = -dur.d.bizsidur;
		break;
	default:
		break;
	}

	/* there's just DT_SEXY as time duration type atm, negate it */
	dur.t.sdur = -dur.t.sdur;
	return dur;
}

DEFUN int
dt_dtdur_neg_p(struct dt_dt_s dur)
{
	/* daisy durs and bizsi durs are special */
	switch (dur.d.typ) {
	case DT_DAISY:
		return dur.d.daisydur < 0;
	case DT_BIZSI:
		return dur.d.bizsidur < 0;
	default:
		return dur.neg;
	}
}


/* date getters, platform dependent */
DEFUN struct dt_dt_s
dt_datetime(dt_dttyp_t outtyp)
{
	struct dt_dt_s res = dt_dt_initialiser();
	const dt_dtyp_t outdtyp = (dt_dtyp_t)outtyp;

	switch (outdtyp) {
	case DT_YMD:
	case DT_YMCW: {
		struct tm tm = now_tm();
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
		default:
			/* grrrr */
			;
		}
		break;
	}
	case DT_DAISY:
		/* time_t's base is 1970-01-01, which is daisy 19359 */
		with (struct timeval tv = now_tv()) {
			res.d.daisy = tv.tv_sec / 86400U + DAISY_UNIX_BASE;
		}
		break;

	case DT_MD:
		/* this one doesn't make sense at all */

	case DT_BIZDA:
	case DT_BIZSI:
		/* could be an idea to have those, innit? */

	default:
	case DT_DUNK:
		break;
	}

	/* time assignment */
	with (struct timeval tv = now_tv()) {
		unsigned int tonly = tv.tv_sec % 86400U;
		res.t.hms.h = tonly / SECS_PER_HOUR;
		tonly %= SECS_PER_HOUR;
		res.t.hms.m = tonly / SECS_PER_MIN;
		tonly %= SECS_PER_MIN;
		res.t.hms.s = tonly;
		res.t.hms.ns = tv.tv_usec * 1000;
	}
	dt_make_sandwich(&res, (dt_dtyp_t)outtyp, DT_HMS);
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
		case DT_MD:
		case DT_YWD:
		case DT_YD:
		case DT_JDN:
		case DT_LDN:
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
				int32_t sx;
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
			return dt_dt_initialiser();
		}
	} else if (dt_sandwich_only_t_p(d)) {
		/* ah, how good is that? */
		;
	} else {
		/* great, what now? */
		;
	}
	return d;
}

DEFUN struct dt_dt_s
dt_dtadd(struct dt_dt_s d, struct dt_dt_s dur)
{
/* we decompose the problem like so:
 * carry <- dpart(dur);
 * tpart(res), carry <- tadd(d, tpart(dur), corr);
 * res <- dadd(dpart(res), carry); */
	signed int carry = 0;
	dt_dttyp_t typ = d.typ;
#if defined WITH_LEAP_SECONDS
	struct dt_dt_s orig;

	if (UNLIKELY(dur.tai)) {
		/* make a copy */
		orig = d;
	}
#endif	/* WITH_LEAP_SECONDS */

	if (UNLIKELY(dur.t.dur && dt_sandwich_only_d_p(d))) {
		/* probably +/-[n]m where `m' was meant to be `mo' */
		dur.d.typ = DT_MD;
		goto dadd;
	} else if (dur.t.dur && d.sandwich) {
		/* make sure we don't blow the carry slot */
		carry = dur.t.sdur / (signed int)SECS_PER_DAY;
		dur.t.sdur = dur.t.sdur % (signed int)SECS_PER_DAY;
		/* accept both t-onlies and sandwiches */
		d.t = dt_tadd(d.t, dur.t, 0);
		carry += d.t.carry;
	} else if (d.typ == DT_SEXY) {
		d.sexy = __sexy_add(d.sexy, dur);
		goto pre_corr;
	}

	/* store the carry somehow */
	if (carry) {
		switch (dur.d.typ) {
		case DT_DAISY:
			/* just add the carry, daisydur is signed enough */
			dur.d.daisydur += carry;
			break;
		case DT_DUNK:
			/* fiddle with DUR, so we can use date-core's adder */
			dur.d.typ = DT_DAISY;
			/* add the carry */
			dur.d.daisydur = carry;
			break;
		default:
			/* we're fucked */
			;
		}
	}

	/* demote D's and DUR's type temporarily */
	if (d.typ != DT_SANDWICH_UNK && dur.d.typ != DT_DUNK) {
	dadd:
		/* let date-core do the addition */
		d.d = dt_dadd(d.d, dur.d);
	} else if (dur.d.typ != DT_DUNK) {
		/* put the carry back into d's daisydur slot */
		d.d.daisydur += dur.d.daisydur;
	}
	/* and promote the whole shebang again */
	d.typ = typ;

pre_corr:
#if defined WITH_LEAP_SECONDS
	if (UNLIKELY(dur.tai) && d.typ != DT_SEXY) {
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
			/* reuse dur for the correction */
			if ((dur.t.sdur = nltr) &&
			    /* get our special tadd with carry */
			    (d.t = dt_tadd(d.t, dur.t, 0), d.t.carry)) {
				/* great, we need to sub/add again
				 * as there's been a wrap-around at
				 * midnight, spooky */
				dur.d.typ = DT_DAISY;
				dur.d.daisydur = d.t.carry;
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

DEFUN struct dt_dt_s
dt_dtdiff(dt_dttyp_t tgttyp, struct dt_dt_s d1, struct dt_dt_s d2)
{
	struct dt_dt_s res = dt_dt_initialiser();

	if (!dt_sandwich_only_d_p(d1) && !dt_sandwich_only_d_p(d2)) {
		/* do the time portion difference right away */
		res.t = dt_tdiff(d1.t, d2.t);
	}
	/* now assess what else is to be done */
	if (dt_sandwich_only_t_p(d1) && dt_sandwich_only_t_p(d2)) {
		dt_make_t_only(&res, (dt_ttyp_t)DT_SEXY);
	} else if (tgttyp > (dt_dttyp_t)DT_UNK &&
		   tgttyp < (dt_dttyp_t)DT_NDTYP) {
		/* check for negative carry */
		if (UNLIKELY(res.t.sdur < 0)) {
			d2.d = dt_dadd(d2.d, dt_make_daisydur(-1));
			res.t.sdur += SECS_PER_DAY;
		}
		res.d = dt_ddiff((dt_dtyp_t)tgttyp, d1.d, d2.d);
		if (dt_sandwich_only_d_p(d1) || dt_sandwich_only_d_p(d2)) {
			dt_make_d_only(&res, (dt_dtyp_t)tgttyp);
		} else {
			dt_make_sandwich(&res, (dt_dtyp_t)tgttyp, DT_HMS);
		}
	} else if (tgttyp == DT_SEXY || tgttyp == DT_SEXYTAI) {
		int64_t sxdur;

		/* go for tdiff and ddiff independently */
		res.d = dt_ddiff(DT_DAISY, d1.d, d2.d);
		/* since target type is SEXY do the conversion here */
		sxdur = (int64_t)res.t.sdur +
			(int64_t)res.d.daisydur * SECS_PER_DAY;

		/* set up the output here */
		res.typ = tgttyp;
		res.dur = 0;
		res.neg = 0;
		res.tai = (uint16_t)(tgttyp == DT_SEXYTAI);
		res.sexydur = sxdur;

#if defined WITH_LEAP_SECONDS
		if (tgttyp == DT_SEXYTAI) {
			/* check for transitions */
			zidx_t i_d1 = leaps_before(d1);
			zidx_t i_d2 = leaps_before(d2);

# if defined WORDS_BIGENDIAN
			/* not needed on little-endians
			 * the little means just that */
			res.soft = sxdur;
# endif	/* WORDS_BIGENDIAN */

			if (UNLIKELY(i_d1 != i_d2)) {
				int nltr = leaps_corr[i_d2] - leaps_corr[i_d1];

				res.corr = nltr;
# if defined WORDS_BIGENDIAN
			} else {
				/* always repack res.corr to remove clutter
				 * from the earlier res.sexydur ass'ment */
				res.corr = 0;
# endif	 /* WORDS_BIGENDIAN */
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
	case DT_DUNK:
	default:
		goto try_time;
	case DT_YMD:
	case DT_DAISY:
	case DT_BIZDA:
		/* use arithmetic comparison */
		if (d1.d.u < d2.d.u) {
			return -1;
		} else if (d1.d.u > d2.d.u) {
			return 1;
		} else {
			/* means they're equal, so try the time part */
			goto try_time;
		}
	case DT_YMCW: {
		/* use designated thing since ymcw dates aren't
		 * increasing */
		int res = __ymcw_cmp(d1.d.ymcw, d2.d.ymcw);
		if (res == 0) {
			goto try_time;
		}
		return res;
	}
	}
try_time:
#if 0
/* constant select is evil */
	switch (DT_SANDWICH_T(d1.typ)) {
	case DT_HMS:
		if (d1.t.hms.u < d2.t.hms.u) {
			return -1;
		} else if (d1.t.hms.u > d2.t.hms.u) {
			return 1;
		}
	case DT_TUNK:
	default:
		return 0;
	}
#else
	if (d1.t.hms.u < d2.t.hms.u) {
		return -1;
	} else if (d1.t.hms.u > d2.t.hms.u) {
		return 1;
	}
	return 0;
#endif

}

DEFUN int
dt_dt_in_range_p(struct dt_dt_s d, struct dt_dt_s d1, struct dt_dt_s d2)
{
	return dt_dtcmp(d, d1) >= 0 && dt_dtcmp(d, d2) <= 0;
}

#if defined __INTEL_COMPILER
# pragma warning (default:2203)
#elif defined __GNUC__
# pragma GCC diagnostic warning "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

DEFUN void
dt_set_default(struct dt_dt_s dt)
{
	(void)dflt_tm(&dt);
	return;
}

#endif	/* INCLUDED_date_core_c_ */
/* dt-core.c ends here */
