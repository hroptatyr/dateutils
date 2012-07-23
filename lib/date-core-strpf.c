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

DEFUN inline void
__fill_strpdi(struct strpdi_s *tgt, struct dt_d_s dur)
{
	switch (dur.typ) {
	case DT_YMD:
		tgt->m = dur.ymd.y * GREG_MONTHS_P_YEAR + dur.ymd.m;
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
		tgt->m = dur.bizda.y * GREG_MONTHS_P_YEAR + dur.bizda.m;
		tgt->b = dur.bizda.bd;
		break;
	case DT_YMCW:
		tgt->m = dur.ymcw.y * GREG_MONTHS_P_YEAR + dur.ymcw.m;
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
	struct dt_d_s res;
	struct strpd_s d;
	const char *sp;

	if ((sp = str) == NULL) {
		goto out;
	}

	d = strpd_initialiser();
	/* read the year */
	if ((d.y = strtoui_lim(sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR)) == -1U ||
	    *sp++ != '-') {
		goto fucked;
	}
	/* read the month */
	if ((d.m = strtoui_lim(sp, &sp, 0, GREG_MONTHS_P_YEAR)) == -1U ||
	    *sp++ != '-') {
		goto fucked;
	}
	/* read the day or the count */
	if ((d.d = strtoui_lim(sp, &sp, 0, 31)) == -1U) {
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
		if ((d.w = strtoui_lim(sp, &sp, 0, GREG_DAYS_P_WEEK)) == -1U) {
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
		d->y = strtoui_lim(sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR);
		sp++;
		d->m = strtoui_lim(sp, &sp, 0, GREG_MONTHS_P_YEAR);
		sp++;
		d->d = strtoui_lim(sp, &sp, 0, 31);
		res = 0 - (d->y == -1U || d->m == -1U || d->d == -1U);
		break;
	case DT_SPFL_N_YEAR:
		if (s.abbr == DT_SPMOD_NORM) {
			d->y = strtoui_lim(sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR);
		} else if (s.abbr == DT_SPMOD_ABBR) {
			d->y = strtoui_lim(sp, &sp, 0, 99);
			if (UNLIKELY(d->y == -1U)) {
				;
			} else if ((d->y += 2000) > 2068) {
				d->y -= 100;
			}
		}
		res = 0 - (d->y == -1U);
		break;
	case DT_SPFL_N_MON:
		d->m = strtoui_lim(sp, &sp, 0, GREG_MONTHS_P_YEAR);
		res = 0 - (d->m == -1U);
		break;
	case DT_SPFL_N_DCNT_MON:
		/* ymd mode? */
		if (LIKELY(!s.bizda)) {
			d->d = strtoui_lim(sp, &sp, 0, 31);
			res = 0 - (d->d == -1U);
		} else {
			d->b = strtoui_lim(sp, &sp, 0, 23);
			res = 0 - (d->b == -1U);
		}
		break;
	case DT_SPFL_N_DCNT_WEEK:
		/* ymcw mode? */
		d->w = strtoui_lim(sp, &sp, 0, GREG_DAYS_P_WEEK);
		res = 0 - (d->w == -1U);
		break;
	case DT_SPFL_N_WCNT_MON:
		/* ymcw mode? */
		d->c = strtoui_lim(sp, &sp, 0, 5);
		res = 0 - (d->c == -1U);
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
		res = 0 - (d->w == -1U);
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
				d->m = -1U;
			}
			break;
		}
		case DT_SPMOD_ILL:
		default:
			break;
		}
		res = 0 - (d->m == -1U);
		break;
	case DT_SPFL_S_QTR:
		if (*sp++ != 'Q') {
			break;
		}
	case DT_SPFL_N_QTR:
		if (d->m == 0) {
			unsigned int q;
			if ((q = strtoui_lim(sp, &sp, 1, 4)) < -1U) {
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
		if ((d->d = strtoui_lim(sp, &sp, 1, 366)) < -1U) {
			res = 0;
			d->flags.d_dcnt_p = 1;
		}
		break;
	case DT_SPFL_N_WCNT_YEAR:
		/* was %C, cannot be used at the moment */
		(void)strtoui_lim(sp, &sp, 0, 53);
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
			d->y = romstrtoui_lim(
				sp, &sp, DT_MIN_YEAR, DT_MAX_YEAR);
		} else if (s.abbr == DT_SPMOD_ABBR) {
			d->y = romstrtoui_lim(sp, &sp, 0, 99);
			if (UNLIKELY(d->y == -1U)) {
				;
			} else if ((d->y += 2000) > 2068) {
				d->y -= 100;
			}
		}
		res = 0 - (d->y == -1U);
		break;
	case DT_SPFL_N_MON:
		d->m = romstrtoui_lim(sp, &sp, 0, GREG_MONTHS_P_YEAR);
		res = 0 - (d->m == -1U);
		break;
	case DT_SPFL_N_DCNT_MON:
		d->d = romstrtoui_lim(sp, &sp, 0, 31);
		res = 0 - (d->d == -1U);
		break;
	case DT_SPFL_N_WCNT_MON:
		d->c = romstrtoui_lim(sp, &sp, 0, 5);
		res = 0 - (d->c == -1U);
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
		d->d = d->d ? d->d : (unsigned int)dt_get_mday(that);
		if (LIKELY(bsz >= 10)) {
			ui32tostr(buf + 0, bsz, d->y, 4);
			buf[4] = '-';
			res = ui32tostr(buf + 5, bsz, d->m, 2);
			buf[7] = '-';
			ui32tostr(buf + 8, bsz, d->d, 2);
			res = 10;
		}
		break;
	case DT_SPFL_N_YEAR:
		if (s.abbr == DT_SPMOD_NORM) {
			res = ui32tostr(buf, bsz, d->y, 4);
		} else if (s.abbr == DT_SPMOD_ABBR) {
			res = ui32tostr(buf, bsz, d->y, 2);
		}
		break;
	case DT_SPFL_N_MON:
		res = ui32tostr(buf, bsz, d->m, 2);
		break;
	case DT_SPFL_N_DCNT_MON:
		/* ymd mode check? */
		if (LIKELY(!s.bizda)) {
			d->d = d->d ? d->d : (unsigned int)dt_get_mday(that);
			res = ui32tostr(buf, bsz, d->d, 2);
		} else {
			int bd = dt_get_bday_q(
				that, __make_bizda_param(s.ab, BIZDA_ULTIMO));
			res = ui32tostr(buf, bsz, bd, 2);
		}
		break;
	case DT_SPFL_N_DCNT_WEEK:
		/* ymcw mode check */
		d->w = d->w ? d->w : dt_get_wday(that);
		res = ui32tostr(buf, bsz, d->w, 2);
		break;
	case DT_SPFL_N_WCNT_MON:
		/* ymcw mode check? */
		d->c = d->c ? d->c : (unsigned int)dt_get_count(that);
		res = ui32tostr(buf, bsz, d->c, 2);
		break;
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
		int yw;
		/* %C/%W week count */
		switch (that.typ) {
		case DT_YMD:
			if (s.cnt_weeks_iso) {
				yw = __ymd_get_wcnt_iso(that.ymd);
			} else {
				yw = __ymd_get_wcnt(that.ymd, s.cnt_wdays_from);
			}
			break;
		case DT_YMCW:
			yw = __ymcw_get_yday(that.ymcw);
			break;
		default:
			yw = 0;
			break;
		}
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
	case DT_SPFL_N_WCNT_MON:
		d->c = d->c ? d->c : (unsigned int)dt_get_count(that);
		res = ui32tostrrom(buf, bsz, d->c);
		break;
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
		res = snprintf(buf, bsz, "%u", d->y);
		break;
	case DT_SPFL_N_MON:
		res = snprintf(buf, bsz, "%u", d->m);
		break;
	case DT_SPFL_N_DCNT_WEEK:
		if (!d->w) {
			/* hack hack hack
			 * we'll think about the consequences later */
			d->w = __uidiv(d->d, GREG_DAYS_P_WEEK);
			d->d = __uimod(d->d, GREG_DAYS_P_WEEK);
		}
		res = snprintf(buf, bsz, "%u", d->w);
		break;
	case DT_SPFL_N_WCNT_MON:
		res = snprintf(buf, bsz, "%u", d->c);
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
