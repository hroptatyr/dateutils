/*** date-core-strpf.c -- parser and formatter funs for date-core
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
#if !defined INCLUDED_date_core_strpf_c_
#define INCLUDED_date_core_strpf_c_

#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include "strops.h"
#include "token.h"
#include "date-core.h"
#include "date-core-strpf.h"

#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

static const char *__long_wday[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
	"Miracleday",
};
DEFVAR const char **dut_long_wday = __long_wday;
DEFVAR const ssize_t dut_nlong_wday = countof(__long_wday);

static const char *__abbr_wday[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Mir",
};
DEFVAR const char **dut_abbr_wday = __abbr_wday;
DEFVAR const ssize_t dut_nabbr_wday = countof(__abbr_wday);

static const char __abab_wday[] = "SMTWRFAX";
DEFVAR const char *dut_abab_wday = __abab_wday;
DEFVAR const ssize_t dut_nabab_wday = countof(__abab_wday);

static const char *__long_mon[] = {
	"Miraculary",
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December",
};
DEFVAR const char **dut_long_mon = __long_mon;
DEFVAR const ssize_t dut_nlong_mon = countof(__long_mon);

static const char *__abbr_mon[] = {
	"Mir",
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
};
DEFVAR const char **dut_abbr_mon = __abbr_mon;
DEFVAR const ssize_t dut_nabbr_mon = countof(__abbr_mon);

/* futures expiry codes, how convenient */
static const char __abab_mon[] = "_FGHJKMNQUVXZ";
DEFVAR const char *dut_abab_mon = __abab_mon;
DEFVAR const ssize_t dut_nabab_mon = countof(__abab_mon);


DEFUN inline void
__fill_strpdi(struct strpdi_s *tgt, struct dt_d_s dur)
{
	switch (dur.typ) {
	case DT_YMD:
		tgt->y = dur.ymd.y;
		tgt->m = dur.ymd.m;
		tgt->d = dur.ymd.d;
		break;
	case DT_DAISY:
		tgt->d = dur.daisydur;
		/* we don't need the negation, so return here */
		return;
	case DT_BIZSI:
		tgt->b = dur.bizsidur;
		/* we don't need the negation, so return here */
		return;
	case DT_BIZDA:
		tgt->y = dur.bizda.y;
		tgt->m = dur.bizda.m;
		tgt->b = dur.bizda.bd;
		break;
	case DT_YMCW:
		tgt->y = dur.ymcw.y;
		tgt->m = dur.ymcw.m;
		tgt->w = dur.ymcw.c;
		tgt->d = dur.ymcw.w;
		break;
	case DT_MD:
		tgt->m = dur.md.m;
		tgt->d = dur.md.d;
		break;
	default:
		break;
	}

	if (UNLIKELY(dur.neg)) {
		tgt->y = -tgt->y;
		tgt->m = -tgt->m;
		tgt->d = -tgt->d;
		tgt->w = -tgt->w;
		tgt->b = -tgt->b;
	}
	return;
}

DEFUN struct dt_d_s
__strpd_std(const char *str, char **ep)
{
/* code dupe, see __strpdt_std() */
	struct dt_d_s res;
	struct strpd_s d;
	const char *sp;

	if ((sp = str) == NULL) {
		goto out;
	}

	d = strpd_initialiser();
	/* read the year */
	if ((d.y = strtoi_lim(sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR)) < 0 ||
	    *sp++ != '-') {
		goto fucked;
	}
	/* check for ywd dates */
	if (UNLIKELY(*sp == 'W')) {
		/* brilliant */
		if ((sp++, d.c = strtoi_lim(sp, &sp, 0, 53)) < 0 ||
		    *sp++ != '-') {
			goto fucked;
		}
		d.flags.c_wcnt_p = 1;
		d.flags.wk_cnt = YWD_ISOWK_CNT;
		goto dow;
	}
	/* read the month */
	if ((d.m = strtoi_lim(sp, &sp, 0, GREG_MONTHS_P_YEAR)) < 0 ||
	    *sp++ != '-') {
		goto fucked;
	}
	/* read the day or the count */
	if ((d.d = strtoi_lim(sp, &sp, 0, 31)) < 0) {
		/* didn't work, fuck off */
		goto fucked;
	}
	/* check the date type */
	switch (*sp) {
	case '-':
		/* it is a YMCW date */
		if ((d.c = d.d) > 5) {
			/* nope, it was bollocks */
			break;
		}
		d.d = 0;
		sp++;
	dow:
		if ((d.w = strtoi_lim(sp, &sp, 0, GREG_DAYS_P_WEEK)) < 0) {
			/* didn't work, fuck off */
			goto fucked;
		}
		break;
	case 'B':
		/* it's a bizda/YMDU before ultimo date */
		d.flags.ab = BIZDA_BEFORE;
	case 'b':
		/* it's a bizda/YMDU after ultimo date */
		d.flags.bizda = 1;
		d.b = d.d;
		d.d = 0;
		sp++;
		break;
	default:
		/* we don't care */
		break;
	}
	/* guess what we're doing */
	res = __guess_dtyp(d);
out:
	if (ep != NULL) {
		*ep = (char*)sp;
	}
	return res;
fucked:
	if (ep != NULL) {
		*ep = (char*)str;
	}
	return dt_d_initialiser();
}

DEFUN int
__strpd_card(struct strpd_s *d, const char *sp, struct dt_spec_s s, char **ep)
{
	/* we're really pessimistic, aren't we? */
	int res = -1;

	switch (s.spfl) {
	default:
	case DT_SPFL_UNK:
		break;
	case DT_SPFL_N_DSTD:
		d->y = strtoi_lim(sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR);
		sp++;
		d->m = strtoi_lim(sp, &sp, 0, GREG_MONTHS_P_YEAR);
		sp++;
		d->d = strtoi_lim(sp, &sp, 0, 31);
		res = 0 - (d->y < 0 || d->m < 0 || d->d < 0);
		break;
	case DT_SPFL_N_YEAR:
		if (s.abbr == DT_SPMOD_NORM) {
			d->y = strtoi_lim(sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR);
		} else if (s.abbr == DT_SPMOD_ABBR) {
			d->y = strtoi_lim(sp, &sp, 0, 99);
			if (UNLIKELY(d->y < 0)) {
				;
			} else if ((d->y += 2000) > 2068) {
				d->y -= 100;
			}
		}
		res = 0 - (d->y < 0);
		break;
	case DT_SPFL_N_MON:
		d->m = strtoi_lim(sp, &sp, 0, GREG_MONTHS_P_YEAR);
		res = 0 - (d->m < 0);
		break;
	case DT_SPFL_N_DCNT_MON:
		/* ymd mode? */
		if (LIKELY(!s.bizda)) {
			d->d = strtoi_lim(sp, &sp, 0, 31);
			res = 0 - (d->d < 0);
		} else {
			d->b = strtoi_lim(sp, &sp, 0, 23);
			res = 0 - (d->b < 0);
		}
		break;
	case DT_SPFL_N_DCNT_WEEK:
		/* ymcw mode? */
		d->w = strtoi_lim(sp, &sp, 0, GREG_DAYS_P_WEEK);
		res = 0 - (d->w < 0);
		break;
	case DT_SPFL_N_WCNT_MON:
		/* ymcw mode? */
		d->c = strtoi_lim(sp, &sp, 0, 5);
		res = 0 - (d->c < 0);
		break;
	case DT_SPFL_S_WDAY:
		/* ymcw mode? */
		switch (s.abbr) {
		case DT_SPMOD_NORM:
			d->w = strtoarri(
				sp, &sp,
				dut_abbr_wday, dut_nabbr_wday);
			break;
		case DT_SPMOD_LONG:
			d->w = strtoarri(
				sp, &sp,
				dut_long_wday, dut_nlong_wday);
			break;
		case DT_SPMOD_ABBR: {
			const char *pos;
			if ((pos = strchr(dut_abab_wday, *sp++)) != NULL) {
				d->w = pos - dut_abab_wday;
			} else {
				d->w = -1;
			}
			break;
		}
		case DT_SPMOD_ILL:
		default:
			break;
		}
		res = 0 - (d->w < 0);
		break;
	case DT_SPFL_S_MON:
		switch (s.abbr) {
		case DT_SPMOD_NORM:
			d->m = strtoarri(
				sp, &sp,
				dut_abbr_mon, dut_nabbr_mon);
			break;
		case DT_SPMOD_LONG:
			d->m = strtoarri(
				sp, &sp,
				dut_long_mon, dut_nlong_mon);
			break;
		case DT_SPMOD_ABBR: {
			const char *pos;
			if ((pos = strchr(dut_abab_mon, *sp++)) != NULL) {
				d->m = pos - dut_abab_mon;
			} else {
				d->m = -1;
			}
			break;
		}
		case DT_SPMOD_ILL:
		default:
			break;
		}
		res = 0 - (d->m < 0);
		break;
	case DT_SPFL_S_QTR:
		if (*sp++ != 'Q') {
			break;
		}
	case DT_SPFL_N_QTR:
		if (d->m == 0) {
			int q;
			if ((q = strtoi_lim(sp, &sp, 1, 4)) >= 0) {
				d->m = q * 3 - 2;
				res = 0;
			}
		}
		break;

	case DT_SPFL_LIT_PERCENT:
		if (*sp++ == '%') {
			res = 0;
		}
		break;
	case DT_SPFL_LIT_TAB:
		if (*sp++ == '\t') {
			res = 0;
		}
		break;
	case DT_SPFL_LIT_NL:
		if (*sp++ == '\n') {
			res = 0;
		}
		break;
	case DT_SPFL_N_DCNT_YEAR:
		/* was %D and %j, cannot be used at the moment */
		if ((d->d = strtoi_lim(sp, &sp, 1, 366)) >= 0) {
			res = 0;
			d->flags.d_dcnt_p = 1;
		}
		break;
	case DT_SPFL_N_WCNT_YEAR:
		/* was %C, cannot be used at the moment */
		d->c = strtoi_lim(sp, &sp, 0, 53);
		d->flags.wk_cnt = s.wk_cnt;
		/* let everyone know d->c has a week-count in there */
		d->flags.c_wcnt_p = 1;
		res = 0;
		break;
	}
	/* assign end pointer */
	if (ep != NULL) {
		*ep = (char*)sp;
	}
	return res;
}

DEFUN int
__strpd_rom(struct strpd_s *d, const char *sp, struct dt_spec_s s, char **ep)
{
	int res = -1;

	switch (s.spfl) {
	default:
	case DT_SPFL_UNK:
		break;

	case DT_SPFL_N_YEAR:
		if (s.abbr == DT_SPMOD_NORM) {
			d->y = romstrtoi_lim(
				sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR);
		} else if (s.abbr == DT_SPMOD_ABBR) {
			d->y = romstrtoi_lim(sp, &sp, 0, 99);
			if (UNLIKELY(d->y < 0)) {
				;
			} else if ((d->y += 2000) > 2068) {
				d->y -= 100;
			}
		}
		res = 0 - (d->y < 0);
		break;
	case DT_SPFL_N_MON:
		d->m = romstrtoi_lim(sp, &sp, 0, GREG_MONTHS_P_YEAR);
		res = 0 - (d->m < 0);
		break;
	case DT_SPFL_N_DCNT_MON:
		d->d = romstrtoi_lim(sp, &sp, 0, 31);
		res = 0 - (d->d < 0);
		break;
	case DT_SPFL_N_WCNT_MON:
		d->c = romstrtoi_lim(sp, &sp, 0, 5);
		res = 0 - (d->c < 0);
		break;
	}
	if (ep != NULL) {
		*ep = (char*)sp;
	}
	return res;
}


#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (default:2203)
#elif defined __GNUC__
# pragma GCC diagnostic warning "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

static void
__strfd_get_md(struct strpd_s *d, struct dt_d_s this)
{
	struct __md_s both = dt_get_md(this);
	d->m = both.m;
	d->d = both.d;
	return;
}

static void
__strfd_get_m(struct strpd_s *d, struct dt_d_s this)
{
	d->m = dt_get_mon(this);
	return;
}

static void
__strfd_get_d(struct strpd_s *d, struct dt_d_s this)
{
	d->d = dt_get_mday(this);
	return;
}

DEFUN size_t
__strfd_card(
	char *buf, size_t bsz, struct dt_spec_s s,
	struct strpd_s *d, struct dt_d_s that)
{
	size_t res = 0;

	switch (s.spfl) {
	default:
	case DT_SPFL_UNK:
		break;
	case DT_SPFL_N_DSTD:
		if (UNLIKELY(!d->m && !d->d)) {
			__strfd_get_md(d, that);
		} else if (UNLIKELY(!d->d)) {
			__strfd_get_d(d, that);
		}
		if (LIKELY(bsz >= 10)) {
			ui32tostr(buf + 0, bsz, d->y, 4);
			buf[4] = '-';
			res = ui32tostr(buf + 5, bsz, d->m, 2);
			buf[7] = '-';
			ui32tostr(buf + 8, bsz, d->d, 2);
			res = 10;
		}
		break;
	case DT_SPFL_N_YEAR: {
		unsigned int y = d->y;
		int prec = 4;

		if (UNLIKELY(s.tai && d->flags.real_y_in_q)) {
			y = d->q;
		}
		if (UNLIKELY(s.abbr == DT_SPMOD_ABBR)) {
			prec = 2;
		}
		res = ui32tostr(buf, bsz, y, prec);
		break;
	}
	case DT_SPFL_N_MON:
		if (UNLIKELY(!d->m && !d->d)) {
			__strfd_get_md(d, that);
		} else if (UNLIKELY(!d->m)) {
			__strfd_get_m(d, that);
		}
		res = ui32tostr(buf, bsz, d->m, 2);
		break;
	case DT_SPFL_N_DCNT_MON: {
		/* ymd mode check? */
		unsigned int pd;

		if (LIKELY(!s.bizda)) {
			if (UNLIKELY(!d->m && !d->d)) {
				__strfd_get_md(d, that);
			} else if (UNLIKELY(!d->d)) {
				__strfd_get_d(d, that);
				pd = d->d;
			}
			pd = d->d;
		} else {
			/* must be bizda now */
			pd = dt_get_bday_q(
				that, __make_bizda_param(s.ab, BIZDA_ULTIMO));
		}
		res = ui32tostr(buf, bsz, pd, 2);
		break;
	}
	case DT_SPFL_N_DCNT_WEEK:
		/* ymcw mode check */
		if (!d->w) {
			d->w = dt_get_wday(that);
		}
		if (UNLIKELY(d->w == 0)) {
			if (s.wk_cnt == YWD_MONWK_CNT) {
				/* turn Sun 00 to Sun 07 */
				d->w = 7;
			}
		}
		res = ui32tostr(buf, bsz, d->w, 2);
		break;
	case DT_SPFL_N_WCNT_MON: {
		unsigned int c = d->c;

		/* ymcw mode check? */
		if (!c || that.typ == DT_YWD) {
			/* don't store it */
			c = (unsigned int)dt_get_wcnt_mon(that);
		}
		res = ui32tostr(buf, bsz, c, 2);
		break;
	}
	case DT_SPFL_S_WDAY:
		/* get the weekday in ymd mode!! */
		d->w = d->w ? d->w : dt_get_wday(that);
		switch (s.abbr) {
		case DT_SPMOD_NORM:
			res = arritostr(
				buf, bsz, d->w,
				dut_abbr_wday, dut_nabbr_wday);
			break;
		case DT_SPMOD_LONG:
			res = arritostr(
				buf, bsz, d->w,
				dut_long_wday, dut_nlong_wday);
			break;
		case DT_SPMOD_ABBR:
			/* super abbrev'd wday */
			if (d->w < dut_nabab_wday) {
				buf[res++] = dut_abab_wday[d->w];
			}
			break;
		case DT_SPMOD_ILL:
		default:
			break;
		}
		break;
	case DT_SPFL_S_MON:
		switch (s.abbr) {
		case DT_SPMOD_NORM:
			res = arritostr(
				buf, bsz, d->m,
				dut_abbr_mon, dut_nabbr_mon);
			break;
		case DT_SPMOD_LONG:
			res = arritostr(
				buf, bsz, d->m,
				dut_long_mon, dut_nlong_mon);
			break;
		case DT_SPMOD_ABBR:
			/* super abbrev'd month */
			if (d->m < dut_nabab_mon) {
				buf[res++] = dut_abab_mon[d->m];
			}
			break;
		case DT_SPMOD_ILL:
		default:
			break;
		}
		break;
	case DT_SPFL_S_QTR:
		buf[res++] = 'Q';
		buf[res++] = (char)(dt_get_quarter(that) + '0');
		break;
	case DT_SPFL_N_QTR:
		buf[res++] = '0';
		buf[res++] = (char)(dt_get_quarter(that) + '0');
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

	case DT_SPFL_N_DCNT_YEAR:
		if (that.typ == DT_YMD || that.typ == DT_BIZDA) {
			/* %j */
			int yd;
			if (LIKELY(!s.bizda)) {
				yd = __ymd_get_yday(that.ymd);
			} else {
				yd = __bizda_get_yday(
					that.bizda, __get_bizda_param(that));
			}
			if (yd >= 0) {
				res = ui32tostr(buf, bsz, yd, 3);
			} else {
				buf[res++] = '0';
				buf[res++] = '0';
				buf[res++] = '0';
			}
		}
		break;
	case DT_SPFL_N_WCNT_YEAR: {
		int yw = dt_get_wcnt_year(that, s.wk_cnt);
		res = ui32tostr(buf, bsz, yw, 2);
		break;
	}
	}
	return res;
}

DEFUN size_t
__strfd_rom(
	char *buf, size_t bsz, struct dt_spec_s s,
	struct strpd_s *d, struct dt_d_s that)
{
	size_t res = 0;

	if (that.typ != DT_YMD) {
		/* not supported for non-ymds */
		return res;
	}

	switch (s.spfl) {
	default:
	case DT_SPFL_UNK:
		break;
	case DT_SPFL_N_YEAR:
		if (s.abbr == DT_SPMOD_NORM) {
			res = ui32tostrrom(buf, bsz, d->y);
			break;
		} else if (s.abbr == DT_SPMOD_ABBR) {
			res = ui32tostrrom(buf, bsz, d->y % 100);
			break;
		}
		break;
	case DT_SPFL_N_MON:
		res = ui32tostrrom(buf, bsz, d->m);
		break;
	case DT_SPFL_N_DCNT_MON:
		res = ui32tostrrom(buf, bsz, d->d);
		break;
	case DT_SPFL_N_WCNT_MON: {
		unsigned int c = d->c;

		if (!c) {
			/* don't store the result */
			c = (unsigned int)dt_get_wcnt_mon(that);
		}
		res = ui32tostrrom(buf, bsz, c);
		break;
	}
	}
	return res;
}

DEFUN size_t
__strfd_dur(
	char *buf, size_t bsz, struct dt_spec_s s,
	struct strpd_s *d, struct dt_d_s UNUSED(that))
{
	size_t res = 0;

	switch (s.spfl) {
	default:
	case DT_SPFL_UNK:
		break;
	case DT_SPFL_N_DSTD:
	case DT_SPFL_N_DCNT_MON:
		res = snprintf(buf, bsz, "%d", d->sd);
		break;
	case DT_SPFL_N_YEAR:
		if (!d->y) {
			/* fill in for a mo, hack hack hack
			 * we'll think about the consequences later */
			d->y = __uidiv(d->m, GREG_MONTHS_P_YEAR);
			d->m = __uimod(d->m, GREG_MONTHS_P_YEAR);
		}
		res = snprintf(buf, bsz, "%d", d->y);
		break;
	case DT_SPFL_N_MON:
		res = snprintf(buf, bsz, "%d", d->m);
		break;
	case DT_SPFL_N_DCNT_WEEK:
		if (!d->w) {
			/* hack hack hack
			 * we'll think about the consequences later */
			d->w = __uidiv(d->d, GREG_DAYS_P_WEEK);
			d->d = __uimod(d->d, GREG_DAYS_P_WEEK);
		}
		res = snprintf(buf, bsz, "%d", d->w);
		break;
	case DT_SPFL_N_WCNT_MON:
		res = snprintf(buf, bsz, "%d", d->c);
		break;
	case DT_SPFL_S_WDAY:
	case DT_SPFL_S_MON:
	case DT_SPFL_S_QTR:
	case DT_SPFL_N_QTR:
	case DT_SPFL_N_DCNT_YEAR:
	case DT_SPFL_N_WCNT_YEAR:
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

#endif	/* INCLUDED_date_core_strpf_c_ */
