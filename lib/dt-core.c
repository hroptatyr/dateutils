/*** dt-core.c -- our universe of datetimes
 *
 * Copyright (C) 2011 Sebastian Freundt
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
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "date-core.h"
#include "time-core.h"
#include "dt-core.h"
#include "strops.h"

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
#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

struct strpdt_s {
	struct strpd_s sd;
	struct strpt_s st;
};

/* used for arithmetic */
struct strpdti_s {
	signed int m;
	signed int d;
	signed int w;
	signed int b;
	signed int S;
};

static struct dt_dt_s
__dt_dt_initialiser(void)
{
#if defined __C1X
	struct dt_dt_s res = {.d.typ = DT_UNK, .d.u = 0, .t.u = 0};
#else  /* !__C1X */
	struct dt_dt_s res;
#endif	/* __C1X */

#if !defined __C1X
	res.d.typ = DT_UNK;
	res.d.u = 0;
	res.t.u = 0;
#endif	/* !__C1X */
	return res;
}

#include "strops.c"


/* guessing parsers */
#include "token.c"

static const char ymdhms_dflt[] = "%FT%T";
static const char ymcwhms_dflt[] = "%Y-%m-%c-%wT%T";
static const char daisyhms_dflt[] = "%dT%T";
static const char sexy_dflt[] = "%s";
static const char bizsihms_dflt[] = "%dbT%T";
static const char bizdahms_dflt[] = "%Y-%m-%dbT%T";

static void
__trans_dtfmt(const char **fmt)
{
	if (LIKELY(**fmt == '%')) {
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
	struct dt_dt_s res = __dt_dt_initialiser();
	struct strpdt_s d = {0};
	const char *sp;

	if ((sp = str) == NULL) {
		goto out;
	}
	/* read the year */
	if ((d.sd.y = strtoui_lim(sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR)) == -1U ||
	    *sp++ != '-') {
		sp = str;
		goto out;
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
	res.d = __guess_dtyp(d.sd);
out:
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
	case DT_SPFL_N_MDAY:
	case DT_SPFL_N_CNT_WEEK:
	case DT_SPFL_N_CNT_MON:
	case DT_SPFL_S_WDAY:
	case DT_SPFL_S_MON:
	case DT_SPFL_S_QTR:
	case DT_SPFL_N_QTR:
	case DT_SPFL_N_CNT_YEAR:
		res = __strpd_card(&d->sd, sp, s, ep);
		break;

	case DT_SPFL_N_TSTD:
	case DT_SPFL_N_HOUR:
	case DT_SPFL_N_MIN:
	case DT_SPFL_N_SEC:
	case DT_SPFL_N_NANO:
	case DT_SPFL_S_AMPM:
		res = __strpt_card(&d->st, sp, s, ep);
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
	case DT_SPFL_N_MDAY:
	case DT_SPFL_N_CNT_WEEK:
	case DT_SPFL_N_CNT_MON:
	case DT_SPFL_S_WDAY:
	case DT_SPFL_S_MON:
	case DT_SPFL_S_QTR:
	case DT_SPFL_N_QTR:
	case DT_SPFL_N_CNT_YEAR:
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


/* parser implementations */
DEFUN struct dt_dt_s
dt_strpdt(const char *str, const char *fmt, char **ep)
{
	struct dt_dt_s res = __dt_dt_initialiser();
	struct strpdt_s d = {0};
	const char *sp = str;
	const char *fp = fmt;

	if (UNLIKELY(fmt == NULL)) {
		return __strpdt_std(str, ep);
	}
	/* translate high-level format names, for sandwiches */
	__trans_dtfmt(&fmt);

	while (*fp && *sp) {
		const char *fp_sav = fp;
		struct dt_spec_s spec = __tok_spec(fp_sav, (char**)&fp);

		if (spec.spfl == DT_SPFL_UNK) {
			/* must be literal */
			if (*fp_sav != *sp++) {
				sp = str;
				goto out;
			}
		} else if (LIKELY(!spec.rom)) {
			const char *sp_sav = sp;
			if (__strpdt_card(&d, sp, spec, (char**)&sp) < 0) {
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
				sp = str;
				goto out;
			}
		}
	}
	/* set the end pointer */
	res.d = __guess_dtyp(d.sd);
out:
	if (ep) {
		*ep = (char*)sp;
	}
	return res;
}

DEFUN size_t
dt_strfdt(char *restrict buf, size_t bsz, const char *fmt, struct dt_dt_s that)
{
	struct strpdt_s d = {0};
	const char *fp;
	char *bp;

	if (UNLIKELY(buf == NULL || bsz == 0)) {
		bp = buf;
		goto out;
	}

	switch (that.d.typ) {
	case DT_YMD:
		d.sd.y = that.d.ymd.y;
		d.sd.m = that.d.ymd.m;
		d.sd.d = that.d.ymd.d;
		if (fmt == NULL) {
			fmt = ymdhms_dflt;
		}
		break;
	case DT_YMCW:
		d.sd.y = that.d.ymcw.y;
		d.sd.m = that.d.ymcw.m;
		d.sd.c = that.d.ymcw.c;
		d.sd.w = that.d.ymcw.w;
		if (fmt == NULL) {
			fmt = ymcwhms_dflt;
		}
		break;
	case DT_DAISY: {
		dt_ymd_t tmp = __daisy_to_ymd(that.d.daisy);
		d.sd.y = tmp.y;
		d.sd.m = tmp.m;
		d.sd.d = tmp.d;
		if (fmt == NULL) {
			/* subject to change */
			fmt = ymdhms_dflt;
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
		if (fmt == NULL) {
			fmt = bizdahms_dflt;
		}
		break;
	}
	default:
	case DT_UNK:
		bp = buf;
		goto out;
	}
	/* translate high-level format names */
	__trans_dtfmt(&fmt);

	/* now cope with the time part */
	d.st.h = that.t.hms.h;
	d.st.m = that.t.hms.m;
	d.st.s = that.t.hms.s;
	d.st.ns = that.t.hms.ns;

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
	struct dt_dt_s res = __dt_dt_initialiser();
	const char *sp = str;
	int tmp;
	struct strpdt_s d = {0};

	if (str == NULL) {
		goto out;
	}
	/* read just one component */
	tmp = strtol(sp, (char**)&sp, 10);
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
	} else if (LIKELY((d.sd.m || d.sd.y))) {
	dflt:
		res.d.typ = DT_YMD;
		res.d.ymd.y = d.sd.y;
		res.d.ymd.m = d.sd.q * 3 + d.sd.m;
		res.d.ymd.d = d.sd.d + d.sd.w * 7;
	} else if (d.sd.d) {
		res.d.typ = DT_DAISY;
		res.d.daisy = d.sd.w * 7 + d.sd.d;
	} else if (d.sd.b) {
		res.d.typ = DT_BIZSI;
		res.d.bizsi = d.sd.w * 5 + d.sd.b;
	} else {
		/* we leave out YMCW diffs simply because YMD diffs
		 * cover them better */
		goto dflt;
	}
out:
	if (ep) {
		*ep = (char*)sp;
	}
	return res;
}

DEFUN size_t
dt_strfdtdur(
	char *restrict buf, size_t bsz, const char *fmt, struct dt_dt_s that)
{
	struct strpdt_s d = {0};
	const char *fp;
	char *bp;

	if (UNLIKELY(buf == NULL || bsz == 0 || !that.d.dur)) {
		bp = buf;
		goto out;
	}

	switch (that.d.typ) {
	case DT_YMD:
		d.sd.y = that.d.ymd.y;
		d.sd.m = that.d.ymd.m;
		d.sd.d = that.d.ymd.d;
		if (fmt == NULL) {
			fmt = ymd_dflt;
		}
		break;
	case DT_YMCW:
		d.sd.y = that.d.ymcw.y;
		d.sd.m = that.d.ymcw.m;
		d.sd.c = that.d.ymcw.c;
		d.sd.w = that.d.ymcw.w;
		if (fmt == NULL) {
			fmt = ymcw_dflt;
		}
		break;
	case DT_DAISY:
		d.sd.d = that.d.daisy;
		if (fmt == NULL) {
			/* subject to change */
			fmt = daisy_dflt;
		}
		break;
	case DT_BIZSI:
		d.sd.d = that.d.bizsi;
		if (fmt == NULL) {
			/* subject to change */
			fmt = bizsi_dflt;
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
		if (fmt == NULL) {
			fmt = bizda_dflt;
		}
		break;
	}
	default:
	case DT_UNK:
		goto out;
	}
	/* translate high-level format names */
	__trans_dtfmt(&fmt);

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
			bp += __strfd_dur(bp, eo - bp, spec, &d.sd, that.d);
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


/* date getters, platform dependent */
DEFUN struct dt_dt_s
dt_datetime(dt_dtyp_t outtyp)
{
	struct dt_dt_s res = __dt_dt_initialiser();
	struct timeval tv;

	if (gettimeofday(&tv, NULL) < 0) {
		return res;
	}

	switch ((res.d.typ = outtyp)) {
	case DT_YMD:
	case DT_YMCW: {
		struct tm tm;
		ffff_gmtime(&tm, tv.tv_sec);
		switch (res.d.typ) {
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
		res.d.daisy = tv.tv_sec / 86400U + 19359;
		break;
	default:
	case DT_UNK:
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
	return res;
}

DEFUN struct dt_dt_s
dt_dtconv(dt_dtyp_t tgttyp, struct dt_dt_s d)
{
	struct dt_dt_s res = __dt_dt_initialiser();

	switch ((res.d.typ = tgttyp)) {
	case DT_YMD:
		res.d.ymd = dt_conv_to_ymd(d.d);
		break;
	case DT_YMCW:
		res.d.ymcw = dt_conv_to_ymcw(d.d);
		break;
	case DT_DAISY:
		res.d.daisy = dt_conv_to_daisy(d.d);
		break;
	case DT_BIZDA:
		/* actually this is a parametrised date */
		res.d.bizda = dt_conv_to_bizda(d.d);
		break;
	case DT_UNK:
	default:
		break;
	}
	return res;
}

#if defined __INTEL_COMPILER
# pragma warning (default:2203)
#elif defined __GNUC__
# pragma GCC diagnostic warning "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

#endif	/* INCLUDED_date_core_c_ */
/* dt-core.c ends here */
