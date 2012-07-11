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
#include "dt-core.h"

#if !defined LIKELY
# define LIKELY(_x)	__builtin_expect(!!(_x), 1)
#endif
#if !defined UNLIKELY
# define UNLIKELY(_x)	__builtin_expect(!!(_x), 0)
#endif
#if !defined countof
# define countof(x)	(sizeof(x) / sizeof(*(x)))
#endif	/* !countof */
#if !defined UNUSED
# define UNUSED(_x)	__attribute__((unused)) _x
#endif	/* !UNUSED */

#if !defined INCLUDED_date_core_c_
# include "date-core.c"
#endif	/* !INCLUDED_date_core_c_ */

#if !defined INCLUDED_time_core_c_
# include "time-core.c"
#endif	/* INCLUDED_time_core_c_ */

#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

#if defined SKIP_LEAP_ARITH
# undef WITH_LEAP_SECONDS
#endif	/* SKIP_LEAP_ARITH */

struct strpdt_s {
	struct strpd_s sd;
	struct strpt_s st;
	long int i;
};

/* used for arithmetic */
struct strpdti_s {
	signed int m;
	signed int d;
	signed int w;
	signed int b;
	signed int S;
};

#include "strops.c"
#if defined WITH_LEAP_SECONDS
# include "leapseconds.def"
#endif	/* WITH_LEAP_SECONDS */


/* converters and stuff */
static inline struct strpdt_s
__attribute__((pure, const))
strpdt_initialiser(void)
{
	struct strpdt_s res;
	res.sd = strpd_initialiser();
	res.st = strpt_initialiser();
	res.i = 0;
	return res;
}

static inline dt_ssexy_t
__to_unix_epoch(struct dt_dt_s dt)
{
/* daisy is competing with the prevalent unix epoch, this is the offset */
#define DAISY_UNIX_BASE		(19359)
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

static inline dt_ssexy_t
__attribute__((unused))
__to_gps_epoch(struct dt_dt_s dt)
{
#define DAISY_GPS_BASE		(23016)
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

static inline struct dt_dt_s
__attribute__((unused))
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


/* guessing parsers */
#include "token.c"

static const char ymdhms_dflt[] = "%FT%T";
static const char ymcwhms_dflt[] = "%Y-%m-%c-%wT%T";
static const char daisyhms_dflt[] = "%dT%T";
static const char sexy_dflt[] = "%s";
static const char bizsihms_dflt[] = "%dbT%T";
static const char bizdahms_dflt[] = "%Y-%m-%dbT%T";

DEFUN void
__trans_dtfmt(const char **fmt)
{
	if (UNLIKELY(*fmt == NULL)) {
		/* um, great */
		;
	} else if (LIKELY(**fmt == '%')) {
		/* don't worry about it */
		;
	} else if (strcasecmp(*fmt, "ymd") == 0) {
		*fmt = ymdhms_dflt;
	} else if (strcasecmp(*fmt, "ymcw") == 0) {
		*fmt = ymcwhms_dflt;
	} else if (strcasecmp(*fmt, "bizda") == 0) {
		*fmt = bizdahms_dflt;
	} else if (strcasecmp(*fmt, "daisy") == 0) {
		*fmt = daisyhms_dflt;
	} else if (strcasecmp(*fmt, "sexy") == 0) {
		*fmt = sexy_dflt;
	} else if (strcasecmp(*fmt, "bizsi") == 0) {
		*fmt = bizsihms_dflt;
	}
	return;
}

static struct dt_dt_s
__strpdt_std(const char *str, char **ep)
{
	struct dt_dt_s res = dt_dt_initialiser();
	struct strpdt_s d = {{0}, {0}};
	const char *sp;

	if ((sp = str) == NULL) {
		goto out;
	}
	/* check for epoch notation */
	if (*sp == '@') {
		/* yay, epoch */
		const char *tmp;
		d.i = strtoi(++sp, &tmp);
		if (UNLIKELY(d.i == -1 && sp == tmp)) {
			sp--;
		} else {
			/* let's make a DT_SEXY */
			res.typ = DT_SEXY;
			res.sxepoch = d.i;
		}
		goto out;
	}
	/* read the year */
	if ((d.sd.y = strtoui_lim(sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR)) == -1U ||
	    *sp++ != '-') {
		sp = str;
		goto try_time;
	}
	/* read the month */
	if ((d.sd.m = strtoui_lim(sp, &sp, 0, 12)) == -1U ||
	    *sp++ != '-') {
		sp = str;
		goto out;
	}
	/* read the day or the count */
	if ((d.sd.d = strtoui_lim(sp, &sp, 0, 31)) == -1U) {
		/* didn't work, fuck off */
		sp = str;
		goto out;
	}
	/* check the date type */
	switch (*sp) {
	case '-':
		/* it is a YMCW date */
		if ((d.sd.c = d.sd.d) > 5) {
			/* nope, it was bollocks */
			break;
		}
		d.sd.d = 0;
		if ((d.sd.w = strtoui_lim(++sp, &sp, 0, 7)) == -1U) {
			/* didn't work, fuck off */
			sp = str;
			goto out;
		}
		break;
	case 'B':
		/* it's a bizda/YMDU before ultimo date */
		d.sd.flags.ab = BIZDA_BEFORE;
	case 'b':
		/* it's a bizda/YMDU after ultimo date */
		d.sd.flags.bizda = 1;
		d.sd.b = d.sd.d;
		d.sd.d = 0;
		sp++;
		break;
	default:
		/* we don't care */
		break;
	}
	/* guess what we're doing */
	if ((res.d = __guess_dtyp(d.sd)).typ == DT_DUNK) {
		/* not much use parsing on */
		goto out;
	}
	/* check for the d/t separator */
	switch (*sp) {
	case 'T':
	case ' ':
	case '\t':
		/* could be a time, could be something, else
		 * make sure we leave a mark */
		str = sp++;
		break;
	default:
		/* should be a no-op */
		dt_make_d_only(&res, res.d.typ);
		goto out;
	}
try_time:
	/* and now parse the time */
	if ((d.st.h = strtoui_lim(sp, &sp, 0, 23)) == -1U) {
		sp = str;
		goto out;
	} else if (*sp != ':') {
		goto eval_time;
	} else if ((d.st.m = strtoui_lim(++sp, &sp, 0, 59)) == -1U) {
		d.st.m = 0;
		goto eval_time;
	} else if (*sp != ':') {
		goto eval_time;
	} else if ((d.st.s = strtoui_lim(++sp, &sp, 0, 60)) == -1U) {
		d.st.s = 0;
	} else if (*sp != '.') {
		goto eval_time;
	} else if ((d.st.ns = strtoui_lim(++sp, &sp, 0, 999999999)) == -1U) {
		d.st.ns = 0;
		goto eval_time;
	}
eval_time:
	res.t.hms.h = d.st.h;
	res.t.hms.m = d.st.m;
	res.t.hms.s = d.st.s;
	if (res.d.typ > DT_DUNK) {
		dt_make_sandwich(&res, res.d.typ, DT_HMS);
	} else {
		dt_make_t_only(&res, DT_HMS);
	}
out:
	/* res.typ coincides with DT_SANDWICH_D_ONLY() if we jumped here */
	if (ep) {
		*ep = (char*)sp;
	}
	return res;
}

static int
__strpdt_card(struct strpdt_s *d, const char *sp, struct dt_spec_s s, char **ep)
{
	int res = 0;

	switch (s.spfl) {
	default:
	case DT_SPFL_UNK:
		res = -1;
		break;
	case DT_SPFL_N_DSTD:
	case DT_SPFL_N_YEAR:
	case DT_SPFL_N_MON:
	case DT_SPFL_N_DCNT_MON:
	case DT_SPFL_N_DCNT_WEEK:
	case DT_SPFL_N_DCNT_YEAR:
	case DT_SPFL_N_WCNT_MON:
	case DT_SPFL_N_WCNT_YEAR:
	case DT_SPFL_S_WDAY:
	case DT_SPFL_S_MON:
	case DT_SPFL_S_QTR:
	case DT_SPFL_N_QTR:
		res = __strpd_card(&d->sd, sp, s, ep);
		goto out_direct;

	case DT_SPFL_N_TSTD:
	case DT_SPFL_N_HOUR:
	case DT_SPFL_N_MIN:
	case DT_SPFL_N_SEC:
	case DT_SPFL_N_NANO:
	case DT_SPFL_S_AMPM:
		res = __strpt_card(&d->st, sp, s, ep);
		goto out_direct;

	case DT_SPFL_N_EPOCH:
		/* read over @ */
		if (UNLIKELY(*sp == '@')) {
			sp++;
		}
		d->i = strtoi(sp, &sp);
		break;

	case DT_SPFL_LIT_PERCENT:
		if (*sp++ != '%') {
			res = -1;
		}
		break;
	case DT_SPFL_LIT_TAB:
		if (*sp++ != '\t') {
			res = -1;
		}
		break;
	case DT_SPFL_LIT_NL:
		if (*sp++ != '\n') {
			res = -1;
		}
		break;
	}
	/* assign end pointer */
	if (ep) {
		*ep = (char*)sp;
	}
out_direct:
	return res;
}

static size_t
__strfdt_card(
	char *buf, size_t bsz, struct dt_spec_s s,
	struct strpdt_s *d, struct dt_dt_s that)
{
	size_t res = 0;

	switch (s.spfl) {
	default:
	case DT_SPFL_UNK:
		break;
	case DT_SPFL_N_DSTD:
	case DT_SPFL_N_YEAR:
	case DT_SPFL_N_MON:
	case DT_SPFL_N_DCNT_WEEK:
	case DT_SPFL_N_DCNT_MON:
	case DT_SPFL_N_DCNT_YEAR:
	case DT_SPFL_N_WCNT_MON:
	case DT_SPFL_N_WCNT_YEAR:
	case DT_SPFL_S_WDAY:
	case DT_SPFL_S_MON:
	case DT_SPFL_S_QTR:
	case DT_SPFL_N_QTR:
		res = __strfd_card(buf, bsz, s, &d->sd, that.d);
		break;

	case DT_SPFL_N_TSTD:
	case DT_SPFL_N_HOUR:
	case DT_SPFL_N_MIN:
	case DT_SPFL_N_SEC:
	case DT_SPFL_S_AMPM:
	case DT_SPFL_N_NANO:
		res = __strft_card(buf, bsz, s, &d->st, that.t);
		break;

	case DT_SPFL_N_EPOCH: {
		/* convert to sexy */
		int64_t sexy = dt_conv_to_sexy(that).sexy;
		res = snprintf(buf, bsz, "%" PRIi64, sexy);
		break;
	}

	case DT_SPFL_LIT_PERCENT:
		/* literal % */
		buf[res++] = '%';
		break;
	case DT_SPFL_LIT_TAB:
		/* literal tab */
		buf[res++] = '\t';
		break;
	case DT_SPFL_LIT_NL:
		/* literal \n */
		buf[res++] = '\n';
		break;
	}
	return res;
}

static size_t
__strfdt_dur(
	char *buf, size_t bsz, struct dt_spec_s s,
	struct strpdt_s *d, struct dt_dt_s that)
{
	switch (s.spfl) {
	default:
	case DT_SPFL_UNK:
		return 0;

	case DT_SPFL_N_DSTD:
	case DT_SPFL_N_YEAR:
	case DT_SPFL_N_MON:
	case DT_SPFL_N_DCNT_WEEK:
	case DT_SPFL_N_DCNT_MON:
	case DT_SPFL_N_DCNT_YEAR:
	case DT_SPFL_N_WCNT_MON:
	case DT_SPFL_N_WCNT_YEAR:
	case DT_SPFL_S_WDAY:
	case DT_SPFL_S_MON:
	case DT_SPFL_S_QTR:
	case DT_SPFL_N_QTR:
		return __strfd_dur(buf, bsz, s, &d->sd, that.d);

		/* noone's ever bothered doing the same thing for times */
	case DT_SPFL_N_TSTD:
	case DT_SPFL_N_SEC:
		if (that.typ == DT_SEXY) {
			/* use the sexy slot */
			int64_t dur = that.sexydur;
			return (size_t)snprintf(buf, bsz, "%" PRIi64 "s", dur);
		} else {
			/* replace me!!! */
			int32_t dur = that.t.sdur;
			return (size_t)snprintf(buf, bsz, "%" PRIi32 "s", dur);
		}

	case DT_SPFL_LIT_PERCENT:
		/* literal % */
		*buf = '%';
		break;
	case DT_SPFL_LIT_TAB:
		/* literal tab */
		*buf = '\t';
		break;
	case DT_SPFL_LIT_NL:
		/* literal \n */
		*buf = '\n';
		break;
	}
	return 1;
}

#if defined WITH_LEAP_SECONDS
static zidx_t
leaps_before(struct dt_dt_s d)
{
	zidx_t res;
	bool on;

	switch (d.typ) {
	case DT_YMD:
		res = leaps_before_ui32(leaps_ymd, nleaps_ymd, d.d.ymd.u);
		on = res + 1 < nleaps_ymd && leaps_ymd[res + 1] == d.d.ymd.u;
		break;
	case DT_YMCW:
		res = leaps_before_ui32(leaps_ymcw, nleaps_ymcw, d.d.ymcw.u);
		on = res + 1 < nleaps_ymcw && leaps_ymcw[res + 1] == d.d.ymcw.u;
		break;
	case DT_DAISY:
		res = leaps_before_ui32(leaps_d, nleaps_d, d.d.daisy);
		on = res + 1 < nleaps_d && leaps_d[res + 1] == d.d.daisy;
		break;
	case DT_SEXY:
	case DT_SEXYTAI:
		res = leaps_before_si32(leaps_s, nleaps_s, (int32_t)d.sexy);
		on = res + 1 < nleaps_s && leaps_s[res + 1] == d.sexy;
		break;
	default:
		res = 0;
		on = false;
		break;
	}

	if (dt_sandwich_p(d) && on) {
		/* check the time part too */
		if (d.t.hms.u24 > leaps_hms[res + 1]) {
			res++;
		}
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
		struct dt_spec_s spec = __tok_spec(fp_sav, (char**)&fp);

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
	/* check if it's a sexy type */
	if (d.i) {
		res.typ = DT_SEXY;
		res.sexy = d.i;
	} else {
		/* assign d and t types using date and time core routines */
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

	/* set the end pointer */
	if (ep) {
		*ep = (char*)sp;
	}
	return res;
fucked:
	if (ep) {
		*ep = (char*)str;
	}
	return dt_dt_initialiser();
}

DEFUN size_t
dt_strfdt(char *restrict buf, size_t bsz, const char *fmt, struct dt_dt_s that)
{
	struct strpdt_s d;
	const char *fp;
	char *bp;

	if (UNLIKELY(buf == NULL || bsz == 0)) {
		bp = buf;
		goto out;
	}

	d = strpdt_initialiser();
	switch (that.typ) {
	case DT_YMD:
		d.sd.y = that.d.ymd.y;
		d.sd.m = that.d.ymd.m;
		d.sd.d = that.d.ymd.d;
		if (fmt == NULL && dt_sandwich_p(that)) {
			fmt = ymdhms_dflt;
		} else if (fmt == NULL && dt_sandwich_only_d_p(that)) {
			fmt = ymd_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;
	case DT_YMCW:
		d.sd.y = that.d.ymcw.y;
		d.sd.m = that.d.ymcw.m;
		d.sd.c = that.d.ymcw.c;
		d.sd.w = that.d.ymcw.w;
		if (fmt == NULL && dt_sandwich_p(that)) {
			fmt = ymcwhms_dflt;
		} else if (fmt == NULL && dt_sandwich_only_d_p(that)) {
			fmt = ymcw_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;
	case DT_DAISY: {
		dt_ymd_t tmp = __daisy_to_ymd(that.d.daisy);
		d.sd.y = tmp.y;
		d.sd.m = tmp.m;
		d.sd.d = tmp.d;
		if (fmt == NULL && dt_sandwich_p(that)) {
			/* subject to change */
			fmt = ymdhms_dflt;
		} else if (fmt == NULL && dt_sandwich_only_d_p(that)) {
			fmt = ymd_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;
	}
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
			fmt = bizdahms_dflt;
		} else if (fmt == NULL && dt_sandwich_only_d_p(that)) {
			fmt = bizda_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;
	}
	case DT_SEXY: {
		dt_ymdhms_t tmp = __epoch_to_ymdhms(that.sxepoch);
		d.st.h = tmp.H;
		d.st.m = tmp.M;
		d.st.s = tmp.S;
		d.sd.y = tmp.y;
		d.sd.m = tmp.m;
		d.sd.d = tmp.d;
	}
	case DT_YMDHMS:
		if (fmt == NULL) {
			fmt = ymdhms_dflt;
		}
		break;
	default:
	case DT_DUNK:
		break;
	}
	/* translate high-level format names */
	if (dt_sandwich_p(that)) {
		__trans_dtfmt(&fmt);
	} else if (dt_sandwich_only_d_p(that)) {
		__trans_dfmt(&fmt);
	} else if (dt_sandwich_only_t_p(that)) {
	try_time:
		__trans_tfmt(&fmt);
	} else if (that.typ >= DT_PACK && that.typ < DT_NDTTYP) {
		/* nothing to check in this case */
		;
	} else {
		bp = buf;
		goto out;
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
		struct dt_spec_s spec = __tok_spec(fp_sav, (char**)&fp);

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
	if (ep) {
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
			fmt = ymdhms_dflt;
		} else if (fmt == NULL && dt_sandwich_only_d_p(that)) {
			fmt = ymd_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;
	case DT_YMCW:
		d.sd.y = that.d.ymcw.y;
		d.sd.m = that.d.ymcw.m;
		d.sd.c = that.d.ymcw.c;
		d.sd.w = that.d.ymcw.w;
		if (fmt == NULL && dt_sandwich_p(that)) {
			fmt = ymcwhms_dflt;
		} else if (fmt == NULL && dt_sandwich_only_d_p(that)) {
			fmt = ymcw_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;
	case DT_DAISY:
		d.sd.d = that.d.daisy;
		if (fmt == NULL && dt_sandwich_p(that)) {
			fmt = daisy_dflt;
		} else if (fmt == NULL && dt_sandwich_only_d_p(that)) {
			fmt = daisy_dflt;
		} else if (fmt == NULL) {
			goto try_time;
		}
		break;
	case DT_BIZSI:
		d.sd.d = that.d.bizsi;
		if (fmt == NULL && dt_sandwich_p(that)) {
			/* subject to change */
			fmt = bizsi_dflt;
		} else if (fmt == NULL && dt_sandwich_only_d_p(that)) {
			fmt = bizsi_dflt;
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
			fmt = bizdahms_dflt;
		} else if (fmt == NULL && dt_sandwich_only_d_p(that)) {
			fmt = bizda_dflt;
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
		__trans_dtfmt(&fmt);
	} else if (dt_sandwich_only_d_p(that)) {
		__trans_dfmt(&fmt);
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
		struct dt_spec_s spec = __tok_spec(fp_sav, (char**)&fp);

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
	struct timeval tv;
	const dt_dtyp_t outdtyp = (dt_dtyp_t)outtyp;

	if (gettimeofday(&tv, NULL) < 0) {
		return res;
	}

	switch (outdtyp) {
	case DT_YMD:
	case DT_YMCW: {
		struct tm tm;
		ffff_gmtime(&tm, tv.tv_sec);
		switch (outdtyp) {
		case DT_YMD:
			res.d.ymd.y = tm.tm_year;
			res.d.ymd.m = tm.tm_mon;
			res.d.ymd.d = tm.tm_mday;
			break;
		case DT_YMCW: {
#if defined __C1X
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
		}
		break;
	}
	case DT_DAISY:
		/* time_t's base is 1970-01-01, which is daisy 19359 */
		res.d.daisy = tv.tv_sec / 86400U + DAISY_UNIX_BASE;
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
	{
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
		switch (tgttyp) {
		case DT_YMD:
			d.d.ymd = dt_conv_to_ymd(d.d);
			break;
		case DT_YMCW:
			d.d.ymcw = dt_conv_to_ymcw(d.d);
			break;
		case DT_DAISY:
			d.d.daisy = dt_conv_to_daisy(d.d);
			break;
		case DT_BIZDA:
			/* actually this is a parametrised date */
			d.d.bizda = dt_conv_to_bizda(d.d);
			break;
		case DT_SEXY:
		case DT_SEXYTAI: {
			dt_daisy_t dd = dt_conv_to_daisy(d.d);

			d.sandwich = 0;
			d.sexy = (dd - DAISY_UNIX_BASE) * SECS_PER_DAY +
				(d.t.hms.h * MINS_PER_HOUR + d.t.hms.m) *
				SECS_PER_MIN + d.t.hms.s;
#if defined WITH_LEAP_SECONDS
			if (tgttyp == DT_SEXYTAI) {
				zidx_t zi = leaps_before_si32(
					leaps_s, nleaps_s, (int32_t)d.sexy);
				d.sexy += leaps_corr[zi];
			}
#endif	/* WITH_LEAP_SECONDS */
			break;
		}
		case DT_YMDHMS:
			/* no support for this guy yet */

		case DT_DUNK:
		default:
			return dt_dt_initialiser();
		}
		d.typ = tgttyp;
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
			dt_make_t_only(&dur, DT_HMS);
			dur.tai = 0;
			dur.t.sdur = nltr;
			d = dt_dtadd(orig, dur);

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

	if (dt_sandwich_only_t_p(d1) && dt_sandwich_only_t_p(d2)) {
		res.t = dt_tdiff(d1.t, d2.t);
		dt_make_t_only(&res, (dt_ttyp_t)DT_SEXY);
	} else if (tgttyp > DT_UNK && tgttyp < DT_NDTYP) {
		res.d = dt_ddiff((dt_dtyp_t)tgttyp, d1.d, d2.d);
		dt_make_d_only(&res, res.d.typ);
	} else if (tgttyp == DT_SEXY || tgttyp == DT_SEXYTAI) {
		int64_t sxdur;

		/* go for tdiff and ddiff independently */
		res.t = dt_tdiff(d1.t, d2.t);
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

			if (UNLIKELY(i_d1 != i_d2)) {
				int nltr = leaps_corr[i_d2] - leaps_corr[i_d1];

				res.corr = nltr;
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

#endif	/* INCLUDED_date_core_c_ */
/* dt-core.c ends here */
