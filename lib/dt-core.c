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

#include "strops.c"

/* daisy is competing with the prevalent unix epoch, this is the offset */
#define DAISY_UNIX_BASE		(19359)


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
	struct dt_dt_s res = dt_dt_initialiser();
	struct strpdt_s d = {{0}, {0}};
	const char *sp;

	if ((sp = str) == NULL) {
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
	switch (*sp++) {
	case 'T':
	case ' ':
	case '\t':
		break;
	default:
		/* that's no good */
		goto try_date;
	}
try_time:
	/* and now parse the time */
	if ((d.st.h = strtoui_lim(sp, &sp, 0, 23)) == -1U) {
		sp = str;
		goto out;
	} else if (*sp++ != ':') {
		sp--;
		goto eval_time;
	} else if ((d.st.m = strtoui_lim(sp, &sp, 0, 59)) == -1U) {
		d.st.m = 0;
		goto eval_time;
	} else if (*sp++ != ':') {
		sp--;
		goto eval_time;
	} else if ((d.st.s = strtoui_lim(sp, &sp, 0, 60)) == -1U) {
		d.st.s = 0;
	} else if (*sp++ != '.') {
		sp--;
		goto eval_time;
	} else if ((d.st.ns = strtoui_lim(sp, &sp, 0, 999999999)) == -1U) {
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
	goto out;
try_date:
	/* should be a no-op */
	dt_make_d_only(&res, res.d.typ);
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
	case DT_SPFL_N_MDAY:
	case DT_SPFL_N_CNT_WEEK:
	case DT_SPFL_N_CNT_MON:
	case DT_SPFL_S_WDAY:
	case DT_SPFL_S_MON:
	case DT_SPFL_S_QTR:
	case DT_SPFL_N_QTR:
	case DT_SPFL_N_CNT_YEAR:
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

/* just like time-core's tadd() but with carry */
static struct dt_t_s
__tadd(struct dt_t_s t, struct dt_t_s dur, signed int *carry)
{
/* return the number of days carried over in CARRY */
	signed int sec;
	signed int tmp;

	sec = dur.sdur;
	sec += t.hms.s;
	if ((tmp = sec % (signed int)SECS_PER_MIN) >= 0) {
		t.hms.s = tmp;
	} else {
		t.hms.s = tmp + SECS_PER_MIN;
		sec -= SECS_PER_MIN;
	}

	sec /= (signed int)SECS_PER_MIN;
	sec += t.hms.m;
	if ((tmp = sec % (signed int)MINS_PER_HOUR) >= 0) {
		t.hms.m = tmp;
	} else {
		t.hms.m = tmp + MINS_PER_HOUR;
		sec -= MINS_PER_HOUR;
	}

	sec /= (signed int)MINS_PER_HOUR;
	sec += t.hms.h;
	if ((tmp = sec % (signed int)HOURS_PER_DAY) >= 0) {
		t.hms.h = tmp;
	} else {
		t.hms.h = tmp + HOURS_PER_DAY;
		sec -= HOURS_PER_DAY;
	}
	if (carry) {
		*carry = sec / (signed int)HOURS_PER_DAY;
	}
	return t;
}


/* parser implementations */
DEFUN struct dt_dt_s
dt_strpdt(const char *str, const char *fmt, char **ep)
{
	struct dt_dt_s res = dt_dt_initialiser();
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
	/* assign d and t types using date core and time core routines */
	res.d = __guess_dtyp(d.sd);
	res.t = __guess_ttyp(d.st);

	if (res.d.typ > DT_DUNK && res.t.typ > DT_TUNK) {
		dt_make_sandwich(&res, res.d.typ, res.t.typ);
	} else if (res.d.typ > DT_DUNK) {
		dt_make_d_only(&res, res.d.typ);
	} else if (res.t.typ > DT_TUNK) {
		dt_make_t_only(&res, res.t.typ);
	}
out:
	/* set the end pointer */
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
	} else {
		bp = buf;
		goto out;
	}

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
	struct dt_dt_s res = dt_dt_initialiser();
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
	case DT_DUNK:
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

	if (gettimeofday(&tv, NULL) < 0) {
		return res;
	}

	switch (outtyp) {
	case DT_YMD:
	case DT_YMCW: {
		struct tm tm;
		ffff_gmtime(&tm, tv.tv_sec);
		switch (outtyp) {
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
	default:
	case DT_MD:
		/* this one doesn't make sense at all */
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
	dt_make_sandwich(&res, (dt_dtyp_t)outtyp, DT_HMS);
	return res;
}

DEFUN struct dt_dt_s
dt_dtconv(dt_dtyp_t tgttyp, struct dt_dt_s d)
{
	struct dt_dt_s res = dt_dt_initialiser();

	switch (tgttyp) {
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
	case DT_DUNK:
	default:
		break;
	}

	if (dt_sandwich_p(d)) {
		dt_make_sandwich(&res, tgttyp, DT_HMS);
	} else if (dt_sandwich_only_d_p(d)) {
		dt_make_d_only(&res, tgttyp);
	} else if (dt_sandwich_only_t_p(d)) {
		dt_make_t_only(&res, DT_HMS);
	} else {
		res.typ = DT_SANDWICH_UNK;
	}
	return res;
}

DEFUN struct dt_dt_s
dt_dtadd(struct dt_dt_s d, struct dt_dt_s dur)
{
	signed int carry = 0;
	dt_dttyp_t typ = d.typ;

	if (UNLIKELY(dur.t.dur && dt_sandwich_only_d_p(d))) {
		/* probably +/-[n]m where `m' was meant to be `mo' */
		dur.d.typ = DT_MD;
		goto dadd;
	} else if (dur.t.dur) {
		d.t = __tadd(d.t, dur.t, &carry);
	}

	/* store the carry somehow */
	if (carry && dur.d.typ == DT_DAISY) {
		/* just add the carry, daisydur is signed enough */
		dur.d.daisydur += carry;
	} else if (carry && dur.d.typ == DT_DUNK) {
		/* fiddle with the dur, so we can use date-core's adder */
		dur.d.typ = DT_DAISY;
		/* add the carry */
		dur.d.daisydur = carry;
	} else if (carry) {
		/* we're fucked */
		;
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
	return d;
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
