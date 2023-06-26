/*** dround.c -- perform simple date arithmetic, round to duration
 *
 * Copyright (C) 2012-2022 Sebastian Freundt
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
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#include "dt-core.h"
#include "dt-io.h"
#include "dt-core-tz-glue.h"
#include "dt-locale.h"
#include "prchunk.h"
/* parsers and formatters */
#include "date-core-strpf.h"
#include "date-core-private.h"

#if !defined assert
# define assert(x)
#endif	/* !assert */

const char *prog = "dround";


static struct dt_t_s
tround_tdur_cocl(struct dt_t_s t, struct dt_dtdur_s dur, bool nextp)
{
/* this will return the rounded to DUR time of T with carry */
	signed int tunp;
	signed int sdur;
	bool downp = false;

	t.carry = 0;

	/* get directions, no dur is a no-op */
	if (UNLIKELY(!(sdur = dur.dv))) {
		/* IPO/LTO hack */
		goto out;
	} else if (sdur < 0) {
		downp = true;
		sdur = -sdur;
	} else if (dur.neg) {
		downp = true;
	}

	switch (dur.durtyp) {
	case DT_DURH:
		sdur *= MINS_PER_HOUR;
		/*@fallthrough@*/
	case DT_DURM:
		sdur *= SECS_PER_MIN;
		/*@fallthrough@*/
	case DT_DURS:
		/* only accept values whose remainder is 0 */
		if (LIKELY(!(SECS_PER_DAY % (unsigned int)sdur))) {
			break;
		}
		/*@fallthrough@*/
	default:
		/* IPO/LTO hack */
		goto out;
	}
	/* unpack t */
	tunp = (t.hms.h * MINS_PER_HOUR + t.hms.m) * SECS_PER_MIN + t.hms.s;
	with (unsigned int diff = tunp % (unsigned int)sdur) {
		if (!diff && !nextp) {
			/* do nothing, i.e. really nothing,
			 * in particular, don't set the slots again in the
			 * assign section
			 * this is not some obscure optimisation but to
			 * support special notations like, military midnight
			 * or leap seconds */
			goto out;
		} else if (!downp) {
			tunp += sdur - diff;
		} else if (!diff/* && downp && nextp*/) {
			tunp -= sdur;
		} else {
			tunp -= diff;
		}
		if (tunp < 0) {
			tunp += SECS_PER_DAY;
			t.carry = -1;
		}
	}

	/* assign */
	t.hms.ns = 0;
	t.hms.s = tunp % SECS_PER_MIN;
	tunp /= SECS_PER_MIN;
	t.hms.m = tunp % MINS_PER_HOUR;
	tunp /= MINS_PER_HOUR;
	t.hms.h = tunp % HOURS_PER_DAY;
	tunp /= HOURS_PER_DAY;
	t.carry += tunp;
out:
	return t;
}

static struct dt_t_s
tround_tdur(struct dt_t_s t, struct dt_dtdur_s dur, bool nextp)
{
/* this will return the rounded to DUR time of T and, to encode carry
 * (which can only take values 0 or 1), we will use t's neg bit */
	bool downp = false;
	signed int dv;

	/* initialise carry */
	t.carry = 0;
	/* get directions */
	if ((dv = dur.dv) < 0) {
		downp = true;
		dv = -dv;
	} else if (dur.neg) {
		downp = true;
	}

	switch (dur.durtyp) {
	case DT_DURH:
		if ((!downp && t.hms.h < dv) ||
		    (downp && t.hms.h > dv)) {
			/* no carry adjustments */
			t.hms.h = dv;
		} else if (t.hms.h == dv && !nextp) {
			/* we're on the hour in question */
			t.hms.h = dv;
		} else if (!downp) {
			t.hms.h = dv;
		hour_oflo:
			t.carry = 1;
		} else {
			t.hms.h = dv;
		hour_uflo:
			t.carry = -1;
		}
		break;
	case DT_DURM:
		if ((!downp && t.hms.m < dv) ||
		    (downp && t.hms.m > dv)) {
			/* no carry adjustments */
			t.hms.m = dv;
		} else if (t.hms.m == dv && !nextp) {
			/* we're on the hour in question */
			t.hms.m = dv;
		} else if (!downp) {
			t.hms.m = dv;
		min_oflo:
			if (UNLIKELY(++t.hms.h >= HOURS_PER_DAY)) {
				t.hms.h = 0;
				goto hour_oflo;
			}
		} else {
			t.hms.m = dv;
		min_uflo:
			if (UNLIKELY(!t.hms.h--)) {
				t.hms.h = HOURS_PER_DAY - 1;
				goto hour_uflo;
			}
		}
		break;
	case DT_DURS:
		if ((!downp && t.hms.s < dv) ||
		    (downp && t.hms.s > dv)) {
			/* no carry adjustments */
			t.hms.s = dv;
		} else if (t.hms.s == dv && !nextp) {
			/* we're on the hour in question */
			t.hms.s = dv;
		} else if (!downp) {
			t.hms.s = dv;
			if (UNLIKELY(++t.hms.m >= MINS_PER_HOUR)) {
				t.hms.m = 0;
				goto min_oflo;
			}
		} else {
			t.hms.s = dv;
			if (UNLIKELY(!t.hms.m--)) {
				t.hms.m = MINS_PER_HOUR - 1;
				goto min_uflo;
			}
		}
		break;
	default:
		break;
	}
	return t;
}

static struct dt_d_s
dround_ddur_cocl(struct dt_d_s d, struct dt_ddur_s dur, bool UNUSED(nextp))
{
/* we won't be using next here because next/prev adjustments should have
 * been made in dround already */
	signed int sdur = dur.dv;

	switch (dur.durtyp) {
	case DT_DURD:
		break;
	case DT_DURBD: {
		struct dt_d_s tmp;
		unsigned int wday;
		signed int diff = 0;

		tmp = dt_dconv(DT_DAISY, d);
		wday = dt_get_wday(tmp);

		if (wday >= DT_SATURDAY) {
			if (sdur < 0 || dur.neg) {
				/* set to previous friday */
				diff = -(signed int)(wday - DT_FRIDAY);
			} else {
				/* set to next monday */
				diff = GREG_DAYS_P_WEEK + DT_MONDAY - wday;
			}
		}

		/* final assignment */
		tmp.daisy += diff;
		d = dt_dconv(d.typ, tmp);
		break;
	}

	case DT_DURYR:
		sdur *= 4;
	case DT_DURQU:
		sdur *= 3;
	case DT_DURMO: {
		int ym, of, on;

		/* we need the concept of months and years
		 * and we use the fact that ymd's and ymcw's
		 * y and m slots coincide*/
		ym = d.ymcw.y * 12 + d.ymcw.m - 1;

		switch (d.typ) {
		case DT_YMD:
			on = d.ymd.d == 1;
			break;
		case DT_YMCW:
			on = d.ymcw.c == 1 && d.ymcw.w == DT_MONDAY;
			break;
		default:
			/* warning? */
			on = 1;
			break;
		}
		of = ym % sdur;
		ym -= of;
		if (sdur > 0 && !dur.neg && (of || !on)) {
			ym += sdur;
		}
		/* reassemble */
		d.ymd.y = ym / 12;
		d.ymd.m = (ym % 12) + 1;
		switch (d.typ) {
		case DT_YMD:
			d.ymd.d = 1;
			break;
		case DT_YMCW:
			d.ymcw.c = 1;
			break;
		}
		break;
	}
	default:
		/* warning? */
		break;
	}
	return d;
}

static struct dt_d_s
dround_ddur(struct dt_d_s d, struct dt_ddur_s dur, bool nextp)
{
	switch (dur.durtyp) {
		unsigned int tgt;
		bool forw;
	case DT_DURD:
		if (dur.dv > 0) {
			tgt = dur.dv;
			forw = true;
		} else if (dur.dv < 0) {
			tgt = -dur.dv;
			forw = false;
		} else {
			/* user is an idiot */
			break;
		}

		switch (d.typ) {
			unsigned int mdays;
		case DT_YMD:
			if ((forw && d.ymd.d < tgt) ||
			    (!forw && d.ymd.d > tgt)) {
				/* no month or year adjustment */
				;
			} else if (d.ymd.d == tgt && !nextp) {
				/* we're ON the date already and no
				 * next/prev date is requested */
				;
			} else if (forw) {
				if (LIKELY(d.ymd.m < GREG_MONTHS_P_YEAR)) {
					d.ymd.m++;
				} else {
					d.ymd.m = 1;
					d.ymd.y++;
				}
			} else {
				if (UNLIKELY(--d.ymd.m < 1)) {
					d.ymd.m = GREG_MONTHS_P_YEAR;
					d.ymd.y--;
				}
			}
			/* get ultimo */
			mdays = __get_mdays(d.ymd.y, d.ymd.m);
			if (UNLIKELY(tgt > mdays)) {
				tgt = mdays;
			}
			/* final assignment */
			d.ymd.d = tgt;
			break;
		default:
			break;
		}
		break;

	case DT_DURBD:
		/* bizsis only work on bizsidurs atm */
		if (dur.dv > 0) {
			tgt = dur.dv;
			forw = true;
		} else if (dur.dv < 0) {
			tgt = -dur.dv;
			forw = false;
		} else {
			/* user is an idiot */
			break;
		}

		switch (d.typ) {
			unsigned int bdays;
		case DT_BIZDA:
			if ((forw && d.bizda.bd < tgt) ||
			    (!forw && d.bizda.bd > tgt)) {
				/* no month or year adjustment */
				;
			} else if (d.bizda.bd == tgt && !nextp) {
				/* we're ON the date already and no
				 * next/prev date is requested */
				;
			} else if (forw) {
				if (LIKELY(d.bizda.m < GREG_MONTHS_P_YEAR)) {
					d.bizda.m++;
				} else {
					d.bizda.m = 1;
					d.bizda.y++;
				}
			} else {
				if (UNLIKELY(--d.bizda.m < 1)) {
					d.bizda.m = GREG_MONTHS_P_YEAR;
					d.bizda.y--;
				}
			}
			/* get ultimo */
			bdays = __get_bdays(d.bizda.y, d.bizda.m);
			if (UNLIKELY(tgt > bdays)) {
				tgt = bdays;
			}
			/* final assignment */
			d.bizda.bd = tgt;
			break;
		default:
			serror("\
Warning: rounding to n-th business day not supported for input value");
			break;
		}
		break;

	case DT_DURQU:
		dur.dv *= 3;
		dur.dv -= (dur.dv > 0) * 2;
		dur.dv += (dur.dv < 0) * 2;
	case DT_DURMO:
	case DT_DURYMD:
		switch (d.typ) {
			unsigned int mdays;
		case DT_YMD:
			tgt = dur.durtyp == DT_DURYMD ? dur.ymd.m : dur.dv;
			forw = !dt_dur_neg_p(dur);

			if ((forw && d.ymd.m < tgt) ||
			    (!forw && d.ymd.m > tgt)) {
				/* no year adjustment */
				;
			} else if (d.ymd.m == tgt && !nextp) {
				/* we're IN the month already and no
				 * next/prev date is requested */
				;
			} else if (forw) {
				/* years don't wrap around */
				d.ymd.y++;
			} else {
				/* years don't wrap around */
				d.ymd.y--;
			}
			/* final assignment */
			d.ymd.m = tgt;
			/* fixup ultimo mismatches */
			mdays = __get_mdays(d.ymd.y, d.ymd.m);
			if (UNLIKELY(d.ymd.d > mdays)) {
				d.ymd.d = mdays;
			}
			break;
		default:
			break;
		}
		break;

	case DT_DURYMCW: {
		struct dt_d_s tmp;
		unsigned int wday;
		signed int diff;

		forw = !dt_dur_neg_p(dur);
		tgt = dur.ymcw.w;

		tmp = dt_dconv(DT_DAISY, d);
		wday = dt_get_wday(tmp);
		diff = (signed)tgt - (signed)wday;


		if ((forw && wday < tgt) ||
		    (!forw && wday > tgt)) {
			/* nothing to do */
			;
		} else if (wday == tgt && !nextp) {
			/* we're on WDAY already, do fuckall */
			;
		} else if (forw) {
			/* week wrap */
			diff += 7;
		} else {
			/* week wrap */
			diff -= 7;
		}

		/* final assignment */
		tmp.daisy += diff;
		d = dt_dconv(d.typ, tmp);
		break;
	}

	case DT_DURWK:
		if (dur.dv > 0) {
			tgt = dur.dv;
			forw = true;
		} else if (dur.dv < 0) {
			tgt = -dur.dv;
			forw = false;
		} else {
			/* user is an idiot */
			break;
		}

		switch (d.typ) {
			unsigned int nw;
		case DT_YWD:
			if ((forw && d.ywd.c < tgt) ||
			    (!forw && d.ywd.c > tgt)) {
				/* no year adjustment */
				;
			} else if (d.ywd.c == tgt && !nextp) {
				/* we're IN the week already and no
				 * next/prev date is requested */
				;
			} else if (forw) {
				/* years don't wrap around */
				d.ywd.y++;
			} else {
				/* years don't wrap around */
				d.ywd.y--;
			}
			/* final assignment */
			d.ywd.c = tgt;
			/* fixup ultimo mismatches */
			nw = __get_isowk(d.ywd.y);
			if (UNLIKELY(d.ywd.c > nw)) {
				d.ywd.c = nw;
			}
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
	return d;
}

static dt_sexy_t
sxround_dur_cocl(dt_sexy_t t, struct dt_dtdur_s dur, bool nextp)
{
/* this will return the rounded to DUR time of T and, to encode carry
 * (which can only take values 0 or 1), we will use t's neg bit */
	dt_ssexy_t sdur;
	bool downp = false;

	/* get directions, no dur is a no-op */
	if (UNLIKELY(!(sdur = dur.dv))) {
		return t;
	} else if (sdur < 0) {
		downp = true;
		sdur = -sdur;
	} else if (dur.neg) {
		downp = true;
	}

	switch (dur.durtyp) {
	case DT_DURH:
		sdur *= MINS_PER_HOUR;
		/*@fallthrough@*/
	case DT_DURM:
		sdur *= SECS_PER_MIN;
		/*@fallthrough@*/
	case DT_DURS:
		/* only accept values whose remainder is 0 */
		if (LIKELY(!(SECS_PER_DAY % (unsigned int)sdur))) {
			break;
		}
		/*@fallthrough@*/
	default:
		return t;
	}
	/* unpack t */
	with (unsigned int diff = t % (dt_sexy_t)sdur) {
		if (!diff && !nextp) {
			/* do nothing, i.e. really nothing,
			 * in particular, don't set the slots again in the
			 * assign section
			 * this is not some obscure optimisation but to
			 * support special notations like, military midnight
			 * or leap seconds */
			return t;
		} else if (!downp) {
			t += sdur - diff;
		} else if (!diff/* && downp && nextp*/) {
			t -= sdur;
		} else {
			t -= diff;
		}
	}
	return t;
}


static struct dt_dt_s
dt_round(struct dt_dt_s d, struct dt_dtdur_s dur, bool nextp)
{
	switch (d.typ) {
	default:
		switch (dur.durtyp) {
		default:
			/* all the other date durs */
			break;

		case DT_DURH:
		case DT_DURM:
		case DT_DURS:
		case DT_DURNANO:
			if (UNLIKELY(dt_sandwich_only_d_p(d))) {
				/* this doesn't make sense */
				break;
			}
			if (!dur.cocl) {
				d.t = tround_tdur(d.t, dur, nextp);
			} else {
				d.t = tround_tdur_cocl(d.t, dur, nextp);
			}
			break;

		case DT_DURD:
		case DT_DURBD:
		case DT_DURMO:
		case DT_DURQU:
		case DT_DURYR:
			/* special case for cocl days/bizdays */
			if (dur.cocl) {
#define midnightp(x)	(!(x).hms.h && !(x).hms.m && !(x).hms.s)
				d.t.carry =
					(dur.d.dv > 0 &&
					 (nextp || !midnightp(d.t))) |
					/* or if midnight and nextp */
					-(dur.d.dv < 0 &&
					  (nextp && midnightp(d.t)));
				/* set to midnight */
				d.t.hms = (dt_hms_t){0};
			}
		}
		/* check carry */
		if (UNLIKELY(d.t.carry)) {
			/* we need to add a day */
			struct dt_ddur_s one_day =
				dt_make_ddur(DT_DURD, d.t.carry);
			d.t.carry = 0;
			d.d = dt_dadd(d.d, one_day);
		}
		with (unsigned int sw = d.sandwich) {
			if (!dur.cocl) {
				d.d = dround_ddur(d.d, dur.d, nextp);
			} else {
				d.d = dround_ddur_cocl(d.d, dur.d, nextp);;
			}
			d.sandwich = (uint8_t)sw;
		}
		break;
	case DT_SEXY:
	case DT_SEXYTAI:
		if (UNLIKELY(!dur.cocl)) {
			error("Error: \
Epoch date/times have no divisions to round to.");
			break;
		}
		/* just keep it sexy */
		d.sexy = sxround_dur_cocl(d.sexy, dur, nextp);
		break;
	}
	return d;
}


static struct dt_dt_s
dround(struct dt_dt_s d, struct dt_dtdur_s dur[], size_t ndur, bool nextp)
{
	for (size_t i = 0; i < ndur; i++) {
		d = dt_round(d, dur[i], nextp);
	}
	return d;
}

/* extended duration reader */
static int
dt_io_strpdtrnd(struct __strpdtdur_st_s *st, const char *str)
{
	char *sp = NULL;
	struct strpd_s d = {0};
	struct dt_spec_s s = {0};
	struct dt_dtdur_s payload = {(dt_dtdurtyp_t)DT_DURUNK};
	int negp = 0;
	int coclp = 0;

	if (dt_io_strpdtdur(st, str) >= 0) {
		return 0;
	}

	/* check for co-classes */
	coclp = (*str == '/');
	str += coclp;
	/* check if there's a sign + or - */
	negp = (*str == '-');
	str += negp || *str == '+';

	/* try weekdays, set up s */
	s.spfl = DT_SPFL_S_WDAY;
	s.abbr = DT_SPMOD_NORM;
	if (__strpd_card(&d, str, s, &sp) >= 0) {
#if defined HAVE_ANON_STRUCTS_INIT
		payload.d = (struct dt_ddur_s){
			DT_DURYMCW,
			.neg = negp,
			.cocl = coclp,
			.ymcw.w = d.w,
		};
#else
		payload.d.durtyp = DT_DURYMCW;
		payload.d.neg = negp;
		payload.d.cocl = coclp;
		payload.d.ymcw.w = d.w;
#endif
		goto out;
	}

	/* try months, set up s */
	s.spfl = DT_SPFL_S_MON;
	s.abbr = DT_SPMOD_NORM;
	if (__strpd_card(&d, str, s, &sp) >= 0) {
#if defined HAVE_ANON_STRUCTS_INIT
		payload.d = (struct dt_ddur_s){
			DT_DURYMD,
			.neg = negp,
			.cocl = coclp,
			.ymd.m = d.m,
		};
#else
		payload.d.durtyp = DT_DURYMD;
		payload.d.neg = negp;
		payload.d.cocl = coclp;
		payload.d.ymd.m = d.m;
#endif
		goto out;
	}

	/* bugger */
	st->istr = str;
	return -1;
out:
	st->sign = 0;
	st->cont = NULL;
	return __add_dur(st, payload);
}

struct prln_ctx_s {
	struct grep_atom_soa_s *ndl;
	const char *ofmt;
	zif_t fromz;
	zif_t outz;
	int sed_mode_p;
	int empty_mode_p;
	int quietp;

	const struct __strpdtdur_st_s *st;
	bool nextp;
};

static int
proc_line(struct prln_ctx_s ctx, char *line, size_t llen)
{
	struct dt_dt_s d;
	char *sp = NULL;
	char *ep = NULL;
	size_t nmatch = 0U;
	int rc = 0;

	do {
		/* check if line matches, */
		d = dt_io_find_strpdt2(
			line, llen, ctx.ndl, &sp, &ep, ctx.fromz);

		if (!dt_unk_p(d)) {
			if (UNLIKELY(d.fix) && !ctx.quietp) {
				rc = 2;
			}
			/* perform addition now */
			d = dround(d, ctx.st->durs, ctx.st->ndurs, ctx.nextp);

			if (ctx.fromz != NULL) {
				/* fixup zone */
				d = dtz_forgetz(d, ctx.fromz);
			}

			if (ctx.sed_mode_p) {
				__io_write(line, sp - line, stdout);
				dt_io_write(d, ctx.ofmt, ctx.outz, '\0');
				llen -= (ep - line);
				line = ep;
				nmatch++;
			} else {
				dt_io_write(d, ctx.ofmt, ctx.outz, '\n');
				break;
			}
		} else if (ctx.sed_mode_p) {
			llen = !(ctx.empty_mode_p && !nmatch) ? llen : 0U;
			line[llen] = '\n';
			__io_write(line, llen + 1, stdout);
			break;
		} else if (ctx.empty_mode_p) {
			__io_write("\n", 1U, stdout);
			break;
		} else {
			/* obviously unmatched, warn about it in non -q mode */
			if (!ctx.quietp) {
				dt_io_warn_strpdt(line);
				rc = 2;
			}
			break;
		}
	} while (1);
	return rc;
}


#include "dround.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	struct dt_dt_s d;
	struct __strpdtdur_st_s st = {0};
	char *inp;
	const char *ofmt;
	char **fmt;
	size_t nfmt;
	int rc = 0;
	bool dt_given_p = false;
	bool nextp = false;
	zif_t fromz = NULL;
	zif_t z = NULL;

	if (yuck_parse(argi, argc, argv)) {
		rc = 1;
		goto out;
	} else if (argi->nargs == 0U) {
		error("Error: DATE or DURATION must be specified\n");
		yuck_auto_help(argi);
		rc = 1;
		goto out;
	}
	/* init and unescape sequences, maybe */
	ofmt = argi->format_arg;
	fmt = argi->input_format_args;
	nfmt = argi->input_format_nargs;
	if (argi->backslash_escapes_flag) {
		dt_io_unescape(argi->format_arg);
		for (size_t i = 0; i < nfmt; i++) {
			dt_io_unescape(fmt[i]);
		}
	}

	if (argi->from_locale_arg) {
		setilocale(argi->from_locale_arg);
	}
	if (argi->locale_arg) {
		setflocale(argi->locale_arg);
	}

	/* try and read the from and to time zones */
	if (argi->from_zone_arg &&
	    (fromz = dt_io_zone(argi->from_zone_arg)) == NULL) {
		error("\
Error: cannot find zone specified in --from-zone: `%s'", argi->from_zone_arg);
		rc = 1;
		goto clear;
	}
	if (argi->zone_arg &&
	    (z = dt_io_zone(argi->zone_arg)) == NULL) {
		error("\
Error: cannot find zone specified in --zone: `%s'", argi->zone_arg);
		rc = 1;
		goto clear;
	}
	if (argi->next_flag) {
		nextp = true;
	}
	if (argi->base_arg) {
		struct dt_dt_s base = dt_strpdt(argi->base_arg, NULL, NULL);
		dt_set_base(base);
	}

	/* check first arg, if it's a date the rest of the arguments are
	 * durations, if not, dates must be read from stdin */
	dt_given_p = !dt_unk_p(d = dt_io_strpdt(*argi->args, fmt, nfmt, NULL));
	for (size_t i = dt_given_p; i < argi->nargs; i++) {
		inp = argi->args[i];
		do {
#define LAST_DUR	(st.durs[st.ndurs - 1])
			if (dt_io_strpdtrnd(&st, inp) < 0) {
				if (UNLIKELY(i == 0)) {
					/* that's ok, must be a date then */
					dt_given_p = true;
				} else {
					serror("Error: \
cannot parse duration/rounding string `%s'", st.istr);\
					rc = 1;
					goto out;
				}
			} else if (LAST_DUR.cocl) {
				switch (LAST_DUR.durtyp) {
				case DT_DURH:
					if (!LAST_DUR.dv ||
					    HOURS_PER_DAY % LAST_DUR.dv) {
						goto nococl;
					}
					break;
				case DT_DURM:
					if (!LAST_DUR.dv ||
					    MINS_PER_HOUR % LAST_DUR.dv) {
						goto nococl;
					}
					break;
				case DT_DURS:
					if (!LAST_DUR.dv ||
					    SECS_PER_MIN % LAST_DUR.dv) {
						goto nococl;
					}
					break;

				case DT_DURD:
				case DT_DURBD:
					if (LAST_DUR.d.dv != 1 &&
					    LAST_DUR.d.dv != -1) {
						goto nococl;
					}
					break;
				case DT_DURMO:
					/* make a millennium the next milestone */
					if (!LAST_DUR.d.dv ||
					    12000 % LAST_DUR.d.dv) {
						goto nococl;
					}
					break;
				case DT_DURQU:
					/* make a millennium the next milestone */
					if (!LAST_DUR.d.dv ||
					    4000 % LAST_DUR.d.dv) {
						goto nococl;
					}
					break;
				case DT_DURYR:
					/* make a millennium the next milestone */
					if (!LAST_DUR.d.dv ||
					    1000 % LAST_DUR.d.dv) {
						goto nococl;
					}
					break;

				nococl:
					error("\
Error: subdivisions must add up to whole divisions");
					rc = 1;
					goto out;
				}
			} else {
				switch (LAST_DUR.durtyp) {
				case DT_DURH:
					if (LAST_DUR.dv >= 24 ||
					    LAST_DUR.dv <= -24) {
						goto range;
					}
					break;
				case DT_DURM:
					if (LAST_DUR.dv >= 60 ||
					    LAST_DUR.dv <= -60) {
						goto range;
					}
					break;
				case DT_DURS:
					if (LAST_DUR.dv >= 60 ||
					    LAST_DUR.dv <= -60) {
						goto range;
					}
					break;
				case DT_DURMO:
					if (!LAST_DUR.d.dv ||
					    LAST_DUR.d.dv > 12 ||
					    LAST_DUR.d.dv < -12) {
						goto range;
					}
					break;
				case DT_DURQU:
					if (!LAST_DUR.d.dv ||
					    LAST_DUR.d.dv > 4 ||
					    LAST_DUR.d.dv < -4) {
						goto range;
					}
					break;
				case DT_DURYR:
					serror("\
Error: Gregorian years are non-recurrent.\n\
Did you mean year class rounding?  Try `/%s'", inp);
					rc = 1;
					goto out;
				default:
					break;

				range:
					serror("\
Error: rounding parameter out of range `%s'", inp);
					rc = 1;
					goto out;
				}
			}
#undef LAST_DUR
		} while (__strpdtdur_more_p(&st));
	}

	/* sanity checks */
	if (dt_given_p) {
		/* date parsing needed postponing as we need to find out
		 * about the durations
		 * Also from-zone conversion needs postponing as we define
		 * the conversion to take place on the rounded date */
		inp = argi->args[0U];
		if (dt_unk_p(d = dt_io_strpdt(inp, fmt, nfmt, NULL))) {
			error("Error: \
cannot interpret date/time string `%s'", argi->args[0U]);
			rc = 1;
			goto out;
		}
	} else if (st.ndurs == 0) {
		error("Error: \
no durations given");
		rc = 1;
		goto out;
	}

	/* start the actual work */
	if (dt_given_p) {
		if (UNLIKELY(d.fix) && !argi->quiet_flag) {
			rc = 2;
		}
		if (!dt_unk_p(d = dround(d, st.durs, st.ndurs, nextp))) {
			if (fromz != NULL) {
				/* fixup zone */
				d = dtz_forgetz(d, fromz);
			}
			dt_io_write(d, ofmt, z, '\n');
		} else {
			rc = 1;
		}
	} else if (!argi->sed_mode_flag && argi->empty_mode_flag) {
		/* read from stdin in exact/empty mode */
		size_t lno = 0;
		void *pctx;

		/* no threads reading this stream */
		__io_setlocking_bycaller(stdout);

		/* using the prchunk reader now */
		if ((pctx = init_prchunk(STDIN_FILENO)) == NULL) {
			serror("could not open stdin");
			goto clear;
		}

		while (prchunk_fill(pctx) >= 0) {
			for (char *line; prchunk_haslinep(pctx); lno++) {
				size_t llen = prchunk_getline(pctx, &line);
				char *ep = NULL;


				if (UNLIKELY(!llen)) {
					goto empty;
				}
				/* try and parse the line */
				d = dt_io_strpdt_ep(line, fmt, nfmt, &ep, fromz);
				if (UNLIKELY(dt_unk_p(d))) {
					goto empty;
				} else if (ep && (unsigned)*ep >= ' ') {
					goto empty;
				}
				/* do the rounding */
				d = dround(d, st.durs, st.ndurs, nextp);
				if (UNLIKELY(dt_unk_p(d))) {
					goto empty;
				}
				if (fromz != NULL) {
					/* fixup zone */
					d = dtz_forgetz(d, fromz);
				}
				dt_io_write(d, ofmt, z, '\n');
				continue;
			empty:
				__io_write("\n", 1U, stdout);
			}
		}
	} else {
		/* read from stdin */
		size_t lno = 0;
		struct grep_atom_s __nstk[16], *needle = __nstk;
		size_t nneedle = countof(__nstk);
		struct grep_atom_soa_s ndlsoa;
		void *pctx;
		struct prln_ctx_s prln = {
			.ndl = &ndlsoa,
			.ofmt = ofmt,
			.fromz = fromz,
			.outz = z,
			.sed_mode_p = argi->sed_mode_flag,
			.empty_mode_p = argi->empty_mode_flag,
			.quietp = argi->quiet_flag,
			.st = &st,
			.nextp = nextp,
		};

		/* no threads reading this stream */
		__io_setlocking_bycaller(stdout);

		/* lest we overflow the stack */
		if (nfmt >= nneedle) {
			/* round to the nearest 8-multiple */
			nneedle = (nfmt | 7) + 1;
			needle = calloc(nneedle, sizeof(*needle));
		}
		/* and now build the needle */
		ndlsoa = build_needle(needle, nneedle, fmt, nfmt);


		/* using the prchunk reader now */
		if ((pctx = init_prchunk(STDIN_FILENO)) == NULL) {
			serror("Error: could not open stdin");
			goto ndl_free;
		}
		while (prchunk_fill(pctx) >= 0) {
			for (char *line; prchunk_haslinep(pctx); lno++) {
				size_t llen = prchunk_getline(pctx, &line);

				rc |= proc_line(prln, line, llen);
			}
		}
		/* get rid of resources */
		free_prchunk(pctx);
	ndl_free:
		if (needle != __nstk) {
			free(needle);
		}
		goto out;
	}
clear:
	/* free the strpdur status */
	__strpdtdur_free(&st);

	dt_io_clear_zones();
	if (argi->from_locale_arg) {
		setilocale(NULL);
	}
	if (argi->locale_arg) {
		setflocale(NULL);
	}

out:
	yuck_free(argi);
	return rc;
}

/* dround.c ends here */
