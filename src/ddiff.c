/*** ddiff.c -- perform simple date arithmetic, date minus date
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
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#include "date-core-private.h"
#include "dt-core-private.h"
#include "dt-io.h"
#include "dt-locale.h"
#include "prchunk.h"

#if !defined UNUSED
# define UNUSED(_x)	__attribute__((unused)) _x
#endif	/* !UNUSED */

typedef union {
	unsigned int flags;
	struct {
		unsigned int has_year:1;
		unsigned int has_qtr:1;
		unsigned int has_mon:1;
		unsigned int has_week:1;
		unsigned int has_day:1;
		unsigned int has_biz:1;

		unsigned int has_hour:1;
		unsigned int has_min:1;
		unsigned int has_sec:1;
		unsigned int has_nano:1;

		unsigned int has_tai:1;
	};
} durfmt_t;

const char *prog = "ddiff";


static durfmt_t
determine_durfmt(const char *fmt)
{
	durfmt_t res = {0};
	dt_dtyp_t special;

	if (fmt == NULL) {
		/* decide later on */
		;
	} else if (UNLIKELY((special = __trans_dfmt_special(fmt)) != DT_DUNK)) {
		switch (special) {
		default:
		case DT_DUNK:
			break;
		case DT_YMD:
			res.has_year = 1;
			res.has_mon = 1;
			res.has_day = 1;
			break;
		case DT_YMCW:
			res.has_year = 1;
			res.has_mon = 1;
			res.has_week = 1;
			res.has_day = 1;
			break;
		case DT_BIZDA:
			res.has_year = 1;
			res.has_mon = 1;
			res.has_day = 1;
			res.has_biz = 1;
			break;
		case DT_DAISY:
			res.has_day = 1;
			break;
		case DT_BIZSI:
			res.has_day = 1;
			res.has_biz = 1;
			break;
		case DT_YWD:
			res.has_year = 1;
			res.has_week = 1;
			res.has_day = 1;
			break;
		case DT_YD:
			res.has_year = 1;
			res.has_day = 1;
			break;
		}

		/* all special types have %0H:%0M:%0S */
		res.has_hour = 1;
		res.has_min = 1;
		res.has_sec = 1;
	} else {
		/* go through the fmt specs */
		for (const char *fp = fmt; *fp;) {
			const char *fp_sav = fp;
			struct dt_spec_s spec = __tok_spec(fp_sav, &fp);

			switch (spec.spfl) {
			case DT_SPFL_UNK:
			default:
				/* nothing changes */
				break;
			case DT_SPFL_N_YEAR:
				res.has_year = 1;
				break;
			case DT_SPFL_N_QTR:
				res.has_qtr = 1;
				break;
			case DT_SPFL_N_MON:
			case DT_SPFL_S_MON:
				res.has_mon = 1;
				break;
			case DT_SPFL_N_DCNT_MON:
				if (spec.bizda) {
					res.has_biz = 1;
				}
			case DT_SPFL_N_DSTD:
			case DT_SPFL_N_DCNT_YEAR:
				res.has_day = 1;
				break;
			case DT_SPFL_N_WCNT_MON:
			case DT_SPFL_N_WCNT_YEAR:
			case DT_SPFL_N_DCNT_WEEK:
			case DT_SPFL_S_WDAY:
				res.has_week = 1;
				break;

			case DT_SPFL_N_TSTD:
			case DT_SPFL_N_SEC:
				if (spec.tai) {
					res.has_tai = 1;
				}
				res.has_sec = 1;
				break;
			case DT_SPFL_N_HOUR:
				res.has_hour = 1;
				break;
			case DT_SPFL_N_MIN:
				res.has_min = 1;
				break;
			case DT_SPFL_N_NANO:
				res.has_nano = 1;
				break;
			}
		}
	}
	return res;
}

static dt_dtdurtyp_t
determine_durtype(struct dt_dt_s d1, struct dt_dt_s d2, durfmt_t f)
{
	/* the type-multiplication table looks like:
	 *
	 * -   D   T   DT
	 * D   d   x   d
	 * T   x   t   x
	 * DT  d   x   s
	 *
	 * where d means a ddur type, t a tdur type and s is DT_SEXY */

	if (UNLIKELY(dt_sandwich_only_t_p(d1) && dt_sandwich_only_t_p(d2))) {
		/* time only duration */
		;
	} else if (dt_sandwich_only_t_p(d1) || dt_sandwich_only_t_p(d2)) {
		/* isn't defined */
		return (dt_dtdurtyp_t)DT_DURUNK;
	} else if (f.has_week && (f.has_mon || f.has_qtr)) {
		return (dt_dtdurtyp_t)DT_DURYMD;
	} else if (f.has_week && f.has_year) {
		return (dt_dtdurtyp_t)DT_DURYWD;
	} else if (f.has_mon || f.has_qtr) {
		return (dt_dtdurtyp_t)DT_DURYMD;
	} else if (f.has_year && f.has_day) {
		return (dt_dtdurtyp_t)DT_DURYD;
	} else if (f.has_day && f.has_biz) {
		return (dt_dtdurtyp_t)DT_DURBD;
	} else if (f.has_year) {
		return (dt_dtdurtyp_t)DT_DURYMD;
	} else if (dt_sandwich_only_d_p(d1) || dt_sandwich_only_d_p(d2)) {
		/* default date-only type */
		return (dt_dtdurtyp_t)DT_DURD;
	} else if (UNLIKELY(f.has_tai)) {
		/* we has tais */
		return (dt_dtdurtyp_t)0xffU;
	}
	/* otherwise */
	return (dt_dtdurtyp_t)(DT_DURS + (f.has_nano));
}


static size_t
ltostr(char *restrict buf, size_t bsz, long int v,
       int range, unsigned int pad)
{
#define C(x)	(char)((x) + '0')
	char *restrict bp = buf;
	const char *const ep = buf + bsz;
	bool negp;

	if (UNLIKELY((negp = v < 0))) {
		v = -v;
	} else if (!v) {
		*bp++ = C(0U);
		range--;
	}
	/* write the mantissa right to left */
	for (; v && bp < ep; range--) {
		register unsigned int x = v % 10U;

		v /= 10U;
		*bp++ = C(x);
	}
	/* fill up with padding */
	if (UNLIKELY(pad)) {
		static const char pads[] = " 0";
		const char p = pads[2U - pad];

		while (range-- > 0) {
			*bp++ = p;
		}
	}
	/* write the sign */
	if (UNLIKELY(negp)) {
		*bp++ = '-';
	}

	/* reverse the string */
	for (char *ip = buf, *jp = bp - 1; ip < jp; ip++, jp--) {
		register char tmp = *ip;
		*ip = *jp;
		*jp = tmp;
	}
#undef C
	return bp - buf;
}

static inline void
dt_io_warn_dur(const char *d1, const char *d2)
{
	error("\
duration between `%s' and `%s' is not defined", d1, d2);
	return;
}

static __attribute__((pure)) long int
__strf_tot_secs(struct dt_dtdur_s dur)
{
/* return time portion of duration in UTC seconds */
	int64_t s = dur.dv;

	if (UNLIKELY(dur.tai) && dur.durtyp == DT_DURS) {
		return dur.soft - dur.corr;
	}

	switch (dur.durtyp) {
	default:
		/* all the date types */
		return dur.t.sdur;
	case DT_DURH:
		s *= MINS_PER_HOUR;
		/*@fallthrough@*/
	case DT_DURM:
		s *= SECS_PER_MIN;
		/*@fallthrough@*/
	case DT_DURS:
		break;
	case DT_DURNANO:
		s /= NANOS_PER_SEC;
		break;
	}
	return s;
}

static __attribute__((pure)) long int
__strf_tot_corr(struct dt_dtdur_s dur)
{
	if (dur.durtyp == DT_DURS && dur.tai) {
		return dur.corr;
	}
	/* otherwise no corrections */
	return 0;
}

static __attribute__((pure)) int
__strf_tot_days(struct dt_dtdur_s dur)
{
/* return date portion of DURation in days */
	int d;

	switch (dur.d.durtyp) {
	case DT_DURD:
	case DT_DURBD:
		d = dur.d.dv;
		break;
	case DT_DURBIZDA:
		d = dur.d.bizda.bd;
		break;
	case DT_DURYMD:
		d = dur.d.ymd.d;
		break;
	case DT_DURYD:
		d = dur.d.yd.d;
		break;
	case DT_DURYMCW:
		d = dur.d.ymcw.w + dur.d.ymcw.c * (int)GREG_DAYS_P_WEEK;
		break;
	case DT_DURYWD:
		d = dur.d.ywd.w + dur.d.ywd.c * (int)GREG_DAYS_P_WEEK;
		break;
	default:
		d = 0;
		break;
	}
	return d;
}

static __attribute__((pure)) int
__strf_tot_mon(struct dt_dtdur_s dur)
{
/* DUR expressed as month and days */
	int m;

	switch (dur.d.durtyp) {
	case DT_DURBIZDA:
		m = dur.d.bizda.m + dur.d.bizda.y * (int)GREG_MONTHS_P_YEAR;
		break;
	case DT_DURYMD:
		m = dur.d.ymd.m + dur.d.ymd.y * (int)GREG_MONTHS_P_YEAR;
		break;
	case DT_DURYMCW:
		m = dur.d.ymcw.m + dur.d.ymcw.y * (int)GREG_MONTHS_P_YEAR;
		break;
	case DT_DURYD:
		m = dur.d.yd.y * (int)GREG_MONTHS_P_YEAR;
		break;
	case DT_DURYWD:
		m = dur.d.ywd.y * (int)GREG_MONTHS_P_YEAR;
		break;
	default:
		m = 0;
		break;
	}
	return m;
}

static __attribute__((pure)) int
__strf_ym_mon(struct dt_dtdur_s dur)
{
	return __strf_tot_mon(dur) % (int)GREG_MONTHS_P_YEAR;
}

static __attribute__((pure)) int
__strf_tot_years(struct dt_dtdur_s dur)
{
	return __strf_tot_mon(dur) / (int)GREG_MONTHS_P_YEAR;
}

static struct precalc_s {
	int neg;
	int Y;
	int q;
	int m;
	int w;
	int d;
	int db;

	long int H;
	long int M;
	long int S;
	long int N;

	long int rS;
} precalc(durfmt_t f, struct dt_dtdur_s dur)
{
#define MINS_PER_DAY	(MINS_PER_HOUR * HOURS_PER_DAY)
#define SECS_PER_WEEK	(SECS_PER_DAY * GREG_DAYS_P_WEEK)
#define MINS_PER_WEEK	(MINS_PER_DAY * GREG_DAYS_P_WEEK)
#define HOURS_PER_WEEK	(HOURS_PER_DAY * GREG_DAYS_P_WEEK)
	struct precalc_s res = {0};
	long long int us;

	/* date specs */
	if (f.has_year) {
		/* just years */
		res.Y = __strf_tot_years(dur);
	}
	if (f.has_year && (f.has_mon || f.has_qtr)) {
		/* years and months */
		res.m = __strf_ym_mon(dur);
	} else if (f.has_mon || f.has_qtr) {
		/* just months */
		res.m = __strf_tot_mon(dur);
	}

	if (f.has_qtr) {
		/* split m slot */
		res.q = res.m / 3;
		res.m = res.m % 3;
	}

	/* the other units are easily converted as their factors are fixed.
	 * we operate on clean seconds and attribute leap seconds only
	 * to the S slot, so 59 seconds plus a leap second != 1 minute */
	with (int64_t S = __strf_tot_secs(dur), d = __strf_tot_days(dur)) {
		us = d * (int)SECS_PER_DAY + S;
		res.neg = dur.neg || us < 0;
		us = us >= 0 ? us : -us;
	}

	if (f.has_week) {
		/* week shadows days in the hierarchy */
		res.w = us / SECS_PER_WEEK;
		us %= SECS_PER_WEEK;
	}
	if (f.has_day) {
		res.d += us / SECS_PER_DAY;
		us %= SECS_PER_DAY;
	}
	if (f.has_hour) {
		res.H = us / SECS_PER_HOUR;
		us %= SECS_PER_HOUR;
	}
	if (f.has_min) {
		/* minutes and seconds */
		res.M = us / SECS_PER_MIN;
		us %= SECS_PER_MIN;
	}
	if (f.has_sec) {
		res.S = us + __strf_tot_corr(dur);
	}
	if (f.has_nano) {
		if (dur.durtyp == DT_DURNANO) {
			res.N = dur.dv % NANOS_PER_SEC;
		}
	}
	return res;
}

static size_t
__strfdtdur(
	char *restrict buf, size_t bsz, const char *fmt,
	struct dt_dtdur_s dur, durfmt_t f, bool only_d_p)
{
/* like strfdtdur() but do some calculations based on F on the way there */
	static const char sexy_dflt_dur[] = "%0T";
	static const char ddur_dflt_dur[] = "%d";
	struct precalc_s pre;
	const char *fp;
	char *bp;

	if (UNLIKELY(buf == NULL || bsz == 0)) {
		bp = buf;
		goto out;
	}

	/* translate high-level format names */
	if (fmt == NULL && dur.durtyp >= (dt_dtdurtyp_t)DT_NDURTYP) {
		fmt = sexy_dflt_dur;
		f.has_sec = 1U;
	} else if (fmt == NULL) {
		fmt = ddur_dflt_dur;
		f.has_day = 1U;
	} else if (only_d_p) {
		__trans_ddurfmt(&fmt);
	} else {
		__trans_dtdurfmt(&fmt);
	}

	/* precompute */
	pre = precalc(f, dur);

	/* assign and go */
	bp = buf;
	fp = fmt;
	if (pre.neg) {
		*bp++ = '-';
	}
	for (char *const eo = buf + bsz; *fp && bp < eo;) {
		const char *fp_sav = fp;
		struct dt_spec_s spec = __tok_spec(fp_sav, &fp);

		if (spec.spfl == DT_SPFL_UNK) {
			/* must be literal then */
			*bp++ = *fp_sav;
		} else if (UNLIKELY(spec.rom)) {
			continue;
		}
		/* otherwise switch over spec.spfl */
		switch (spec.spfl) {
		case DT_SPFL_LIT_PERCENT:
			/* literal % */
			*bp++ = '%';
			break;
		case DT_SPFL_LIT_TAB:
			/* literal tab */
			*bp++ = '\t';
			break;
		case DT_SPFL_LIT_NL:
			/* literal \n */
			*bp++ = '\n';
			break;

		case DT_SPFL_N_DSTD:
			bp += ltostr(bp, eo - bp, pre.d, -1, DT_SPPAD_NONE);
			*bp++ = 'd';
			goto bizda_suffix;

		case DT_SPFL_N_DCNT_MON: {
			int rng = 2;

			if (!f.has_mon && !f.has_week && f.has_year) {
				rng++;
			}
			bp += ltostr(bp, eo - bp, pre.d, rng, spec.pad);
		}
		bizda_suffix:
			if (spec.bizda) {
				/* don't print the b after an ordinal */
				dt_bizda_param_t bprm;

				bprm.bs = dur.d.param;
				switch (bprm.ab) {
				case BIZDA_AFTER:
					*bp++ = 'b';
					break;
				case BIZDA_BEFORE:
					*bp++ = 'B';
					break;
				}
			}
			break;

		case DT_SPFL_N_WCNT_MON:
		case DT_SPFL_N_DCNT_WEEK:
			bp += ltostr(bp, eo - bp, pre.w, 2, spec.pad);
			break;

		case DT_SPFL_N_MON:
			bp += ltostr(bp, eo - bp, pre.m, 2, spec.pad);
			break;

		case DT_SPFL_N_QTR:
			bp += ltostr(bp, eo - bp, pre.q, 2, spec.pad);
			break;

		case DT_SPFL_N_YEAR:
			bp += ltostr(bp, eo - bp, pre.Y, -1, DT_SPPAD_NONE);
			break;

			/* time specs */
		case DT_SPFL_N_TSTD:
			if (UNLIKELY(spec.tai)) {
				pre.S += __strf_tot_corr(dur);
			}
			bp += ltostr(bp, eo - bp, pre.S, -1, DT_SPPAD_NONE);
			*bp++ = 's';
			break;

		case DT_SPFL_N_SEC:
			if (UNLIKELY(spec.tai)) {
				pre.S += __strf_tot_corr(dur);
			}

			bp += ltostr(bp, eo - bp, pre.S, 2, spec.pad);
			break;

		case DT_SPFL_N_MIN:
			bp += ltostr(bp, eo - bp, pre.M, 2, spec.pad);
			break;

		case DT_SPFL_N_HOUR:
			bp += ltostr(bp, eo - bp, pre.H, 2, spec.pad);
			break;

		case DT_SPFL_N_NANO:
			bp += ltostr(bp, eo - bp, pre.N, 9, DT_SPPAD_ZERO);
			break;

		default:
			break;
		}
	}
out:
	if (bp < buf + bsz) {
		*bp = '\0';
	}
	return bp - buf;
}

static int
ddiff_prnt(struct dt_dtdur_s dur, const char *fmt, durfmt_t f, bool only_d_p)
{
/* this is mainly a better dt_strfdtdur() */
	char buf[256];
	size_t res = __strfdtdur(buf, sizeof(buf), fmt, dur, f, only_d_p);

	if (res > 0 && buf[res - 1] != '\n') {
		/* auto-newline */
		buf[res++] = '\n';
	}
	if (res > 0) {
		__io_write(buf, res, stdout);
	}
	return (res > 0) - 1;
}


#include "ddiff.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	struct dt_dt_s d;
	const char *ofmt;
	const char *refinp;
	char **fmt;
	size_t nfmt;
	int rc = 0;
	durfmt_t dfmt;
	dt_dtdurtyp_t dtyp;
	zif_t fromz = NULL;

	if (yuck_parse(argi, argc, argv)) {
		rc = 1;
		goto out;
	}
	/* unescape sequences, maybe */
	if (argi->backslash_escapes_flag) {
		dt_io_unescape(argi->format_arg);
	}

	if (argi->from_locale_arg) {
		setilocale(argi->from_locale_arg);
	}

	/* try and read the from and to time zones */
	if (argi->from_zone_arg &&
	    (fromz = dt_io_zone(argi->from_zone_arg)) == NULL) {
		error("\
Error: cannot find zone specified in --from-zone: `%s'", argi->from_zone_arg);
		rc = 1;
		goto clear;
	}
	if (argi->base_arg) {
		struct dt_dt_s base = dt_strpdt(argi->base_arg, NULL, NULL);
		dt_set_base(base);
	}

	ofmt = argi->format_arg;
	fmt = argi->input_format_args;
	nfmt = argi->input_format_nargs;

	if (argi->nargs == 0 ||
	    (refinp = argi->args[0U],
	     dt_unk_p(d = dt_io_strpdt(refinp, fmt, nfmt, fromz)) &&
	     dt_unk_p(d = dt_io_strpdt(refinp, NULL, 0U, fromz)))) {
		error("Error: reference DATE must be specified\n");
		yuck_auto_help(argi);
		rc = 1;
		goto out;
	} else if (UNLIKELY(d.fix) && !argi->quiet_flag) {
		rc = 2;
	}

	/* try and guess the diff tgttype most suitable for user's FMT */
	dfmt = determine_durfmt(ofmt);

	if (argi->nargs > 1) {
		for (size_t i = 1; i < argi->nargs; i++) {
			struct dt_dt_s d2;
			struct dt_dtdur_s dur;
			const char *inp = argi->args[i];
			bool onlydp;

			d2 = dt_io_strpdt(inp, fmt, nfmt, fromz);
			if (dt_unk_p(d2)) {
				if (!argi->quiet_flag) {
					dt_io_warn_strpdt(inp);
					rc = 2;
				}
				continue;
			} else if (UNLIKELY(d2.fix) && !argi->quiet_flag) {
				rc = 2;
			}
			/* guess the diff type */
			onlydp = dt_sandwich_only_d_p(d) ||
				dt_sandwich_only_d_p(d2);
			if (!(dtyp = determine_durtype(d, d2, dfmt))) {
				if (!argi->quiet_flag) {
				        dt_io_warn_dur(refinp, inp);
					rc = 2;
				}
				continue;
			}
			/* subtraction and print */
			dur = dt_dtdiff(dtyp, d, d2);
			ddiff_prnt(dur, ofmt, dfmt, onlydp);
		}
	} else {
		/* read from stdin */
		size_t lno = 0;
		void *pctx;

		/* convert deprecated -S|--skip-illegal */
		argi->empty_mode_flag += argi->skip_illegal_flag;

		/* no threads reading this stream */
		__io_setlocking_bycaller(stdout);

		/* using the prchunk reader now */
		if ((pctx = init_prchunk(STDIN_FILENO)) == NULL) {
			serror("Error: could not open stdin");
			goto out;
		}
		while (prchunk_fill(pctx) >= 0) {
			for (char *line; prchunk_haslinep(pctx); lno++) {
				struct dt_dt_s d2;
				struct dt_dtdur_s dur;
				bool onlydp;

				(void)prchunk_getline(pctx, &line);
				d2 = dt_io_strpdt(line, fmt, nfmt, fromz);

				if (dt_unk_p(d2)) {
					if (!argi->quiet_flag) {
						dt_io_warn_strpdt(line);
						rc = 2;
					}
					if (argi->empty_mode_flag) {
						/* empty line */
						__io_write("\n", 1U, stdout);
					}
					continue;
				} else if (UNLIKELY(d2.fix) &&
					   !argi->quiet_flag) {
					rc = 2;
				}
				/* guess the diff type */
				onlydp = dt_sandwich_only_d_p(d) ||
					dt_sandwich_only_d_p(d2);
				if (!(dtyp = determine_durtype(d, d2, dfmt))) {
					if (!argi->quiet_flag) {
						dt_io_warn_dur(refinp, line);
						rc = 2;
					}
					continue;
				}
				/* perform subtraction now */
				dur = dt_dtdiff(dtyp, d, d2);
				ddiff_prnt(dur, ofmt, dfmt, onlydp);
			}
		}
		/* get rid of resources */
		free_prchunk(pctx);
	}

clear:
	dt_io_clear_zones();
	if (argi->from_locale_arg) {
		setilocale(NULL);
	}

out:
	yuck_free(argi);
	return rc;
}

/* ddiff.c ends here */
