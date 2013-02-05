/*** dt-core-strpf.c -- parser and formatter funs for dt-core
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
/* implementation part of date-core-strpf.h */
#if !defined INCLUDED_dt_core_strpf_c_
#define INCLUDED_dt_core_strpf_c_

#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include "strops.h"
#include "token.h"
#include "dt-core.h"
#include "dt-core-strpf.h"

#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

#if defined SKIP_LEAP_ARITH
# undef WITH_LEAP_SECONDS
#endif	/* SKIP_LEAP_ARITH */

static int32_t
try_zone(const char *str, const char **ep)
{
	int minusp = 0;
	const char *sp = str;
	int32_t tmp;
	int32_t res;

	switch (*sp) {
	case '-':
		minusp = 1;
	case '+':
		if ((tmp = strtoi_lim(++sp, &sp, 0, 14)) < 0) {
			goto nope;
		} else if ((res = 3600 * tmp, *sp != ':')) {
			goto nope;
		} else if ((tmp = strtoi_lim(++sp, &sp, 0, 59)) < 0) {
			goto nope;
		} else if ((res += 60 * tmp, *sp != ':')) {
			goto nope;
		} else if ((tmp = strtoi_lim(++sp, &sp, 0, 59)) < 0) {
			goto nope;
		}
		res += tmp;
		break;
	default:
		goto nope;
	}
nope:
	/* res.typ coincides with DT_SANDWICH_D_ONLY() if we jumped here */
	if (ep != NULL) {
		*ep = sp;
	}
	return minusp ? -res : res;
}

DEFUN struct dt_dt_s
__strpdt_std(const char *str, char **ep)
{
/* code dupe, see __strpd_std() */
	struct dt_dt_s res = dt_dt_initialiser();
	struct strpdt_s d = strpdt_initialiser();
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
	if ((d.sd.y = strtoi_lim(sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR)) < 0 ||
	    *sp++ != '-') {
		sp = str;
		goto try_time;
	}
	/* check for ywd dates */
	if (UNLIKELY(*sp == 'W')) {
		/* brilliant */
		if ((sp++, d.sd.c = strtoi_lim(sp, &sp, 0, 53)) < 0 ||
		    *sp++ != '-') {
			goto try_time;
		}
		d.sd.flags.c_wcnt_p = 1;
		d.sd.flags.wk_cnt = YWD_ISOWK_CNT;
		goto dow;
	}
	/* read the month */
	if ((d.sd.m = strtoi_lim(sp, &sp, 0, 12)) < 0 ||
	    *sp++ != '-') {
		sp = str;
		goto out;
	}
	/* read the day or the count */
	if ((d.sd.d = strtoi_lim(sp, &sp, 0, 31)) < 0) {
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
	dow:
		if ((d.sd.w = strtoi_lim(++sp, &sp, 0, 7)) < 0) {
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
	if ((d.st.h = strtoi_lim(sp, &sp, 0, 23)) < 0) {
		sp = str;
		goto out;
	} else if (*sp != ':') {
		goto eval_time;
	} else if ((d.st.m = strtoi_lim(++sp, &sp, 0, 59)) < 0) {
		d.st.m = 0;
		goto eval_time;
	} else if (*sp != ':') {
		goto eval_time;
	} else if ((d.st.s = strtoi_lim(++sp, &sp, 0, 60)) < 0) {
		d.st.s = 0;
	} else if (*sp != '.') {
		goto try_zone;
	} else if ((d.st.ns = strtoi_lim(++sp, &sp, 0, 999999999)) < 0) {
		d.st.ns = 0;
		goto eval_time;
	}
try_zone:
	d.zdiff = try_zone(sp, &sp);
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
	if (ep != NULL) {
		*ep = (char*)sp;
	}
	return res;
}

DEFUN int
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

	case DT_SPFL_N_ZDIFF:
		d->zdiff = try_zone(sp, &sp);
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
	if (ep != NULL) {
		*ep = (char*)sp;
	}
out_direct:
	return res;
}

DEFUN size_t
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

	case DT_SPFL_N_ZDIFF: {
		int32_t z = 0U;
		char sign = '+';

		if (z < 0) {
			z = -z;
			sign = '-';
		}
		res = snprintf(
			buf, bsz, "%c%02u:%02u",
			sign, (uint32_t)z / 3600U, ((uint32_t)z / 60U) % 60U);
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

DEFUN size_t
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

static size_t
__strfdt_xdn(char *buf, size_t bsz, struct dt_dt_s that)
{
	double dn;

	switch (that.d.typ) {
	case DT_JDN:
		dn = (double)that.d.jdn;
		break;
	case DT_LDN:
		dn = (double)that.d.ldn;
		break;
	default:
		return 0;
	}

	if (dt_sandwich_p(that)) {
		unsigned int ss = __secs_since_midnight(that.t);
		dn += (double)ss / (double)SECS_PER_DAY;
	}

	return snprintf(buf, bsz, "%.6f", dn);
}

#endif	/* INCLUDED_dt_core_strpf_c_ */
