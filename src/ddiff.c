/*** ddiff.c -- perform simple date arithmetic, date minus date
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
#include "prchunk.h"

#if !defined UNUSED
# define UNUSED(_x)	__attribute__((unused)) _x
#endif	/* !UNUSED */

typedef union {
	unsigned int flags;
	struct {
		unsigned int has_year:1;
		unsigned int has_mon:1;
		unsigned int has_week:1;
		unsigned int has_day:1;
		unsigned int has_biz:1;

		unsigned int has_hour:1;
		unsigned int has_min:1;
		unsigned int has_sec:1;
		unsigned int has_nano:1;
	};
} durfmt_t;


static durfmt_t
determine_durfmt(const char *fmt)
{
	durfmt_t res = {0};

	if (fmt == NULL) {
		res.has_day = 1;
	} else if (strcasecmp(fmt, "ymd") == 0) {
		res.has_year = 1;
		res.has_mon = 1;
		res.has_day = 1;
	} else if (strcasecmp(fmt, "ymcw") == 0) {
		res.has_year = 1;
		res.has_mon = 1;
		res.has_week = 1;
		res.has_day = 1;
	} else if (strcasecmp(fmt, "daisy") == 0) {
		res.has_day = 1;
	} else if (strcasecmp(fmt, "bizda") == 0) {
		res.has_year = 1;
		res.has_mon = 1;
		res.has_day = 1;
		res.has_biz = 1;
	} else if (strcasecmp(fmt, "bizsi") == 0) {
		res.has_day = 1;
		res.has_biz = 1;
	} else {
		/* go through the fmt specs */
		for (const char *fp = fmt; *fp;) {
			const char *fp_sav = fp;
			struct dt_spec_s spec = __tok_spec(fp_sav, (char**)&fp);

			switch (spec.spfl) {
			case DT_SPFL_UNK:
			default:
				/* nothing changes */
				break;
			case DT_SPFL_N_YEAR:
				res.has_year = 1;
				break;
			case DT_SPFL_N_MON:
			case DT_SPFL_S_MON:
				res.has_mon = 1;
				break;
			case DT_SPFL_N_MDAY:
				if (spec.bizda) {
					res.has_biz = 1;
				}
			case DT_SPFL_N_DSTD:
			case DT_SPFL_N_CNT_YEAR:
				res.has_day = 1;
				break;
			case DT_SPFL_N_CNT_MON:
			case DT_SPFL_N_CNT_WEEK:
			case DT_SPFL_S_WDAY:
				res.has_week = 1;
				break;

			case DT_SPFL_N_TSTD:
			case DT_SPFL_N_SEC:
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

static dt_dttyp_t
determine_durtype(durfmt_t f)
{
	if (!f.has_hour && !f.has_min && !f.has_sec && !f.has_nano) {
		if (f.has_week && (f.has_mon || f.has_year)) {
			return (dt_dttyp_t)DT_YMCW;
		} else if (f.has_mon || f.has_year) {
			return (dt_dttyp_t)DT_YMD;
		} else if (f.has_day && f.has_biz) {
			return (dt_dttyp_t)DT_BIZSI;
		} else if (f.has_day || f.has_week) {
			return (dt_dttyp_t)DT_DAISY;
		} else {
			return (dt_dttyp_t)DT_MD;
		}
	}
	return (dt_dttyp_t)DT_SEXY;
}

static int
ddiff_prnt(struct dt_dt_s dur, const char *fmt, durfmt_t f)
{
	char buf[256];
	size_t res = dt_strfdtdur(buf, sizeof(buf), fmt, dur);

	if (res > 0 && buf[res - 1] != '\n') {
		/* auto-newline */
		buf[res++] = '\n';
	}
	if (res > 0) {
		__io_write(buf, res, stdout);
	}
	return (res > 0) - 1;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wswitch-enum"
#endif	/* __INTEL_COMPILER */
#include "ddiff-clo.h"
#include "ddiff-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
# pragma warning (default:181)
#elif defined __GNUC__
# pragma GCC diagnostic warning "-Wswitch-enum"
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	struct dt_dt_s d;
	const char *ofmt;
	char **fmt;
	size_t nfmt;
	int res = 0;
	durfmt_t dfmt;
	dt_dttyp_t dtyp;

	if (cmdline_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	}
	/* unescape sequences, maybe */
	if (argi->backslash_escapes_given) {
		dt_io_unescape(argi->format_arg);
	}

	ofmt = argi->format_arg;
	fmt = argi->input_format_arg;
	nfmt = argi->input_format_given;

	if (argi->inputs_num == 0 ||
	    dt_unk_p(d = dt_io_strpdt(argi->inputs[0], fmt, nfmt, NULL))) {
		fputs("Error: reference DATE must be specified\n\n", stderr);
		cmdline_parser_print_help();
		res = 1;
		goto out;
	}

	/* try and guess the diff tgttype most suitable for user's FMT */
	dfmt = determine_durfmt(ofmt);
	dtyp = determine_durtype(dfmt);

	if (argi->inputs_num > 1) {
		for (size_t i = 1; i < argi->inputs_num; i++) {
			struct dt_dt_s d2;
			struct dt_dt_s dur;
			const char *inp = argi->inputs[i];

			d2 = dt_io_strpdt(inp, fmt, nfmt, NULL);
			if (dt_unk_p(d2)) {
				if (!argi->quiet_given) {
					dt_io_warn_strpdt(inp);
				}
				continue;
			}
			/* subtraction and print */
			dur = dt_dtdiff(dtyp, d, d2);
			ddiff_prnt(dur, ofmt, dfmt);
		}
	} else {
		/* read from stdin */
		size_t lno = 0;
		void *pctx;

		/* no threads reading this stream */
		__io_setlocking_bycaller(stdout);

		/* using the prchunk reader now */
		if ((pctx = init_prchunk(STDIN_FILENO)) == NULL) {
			perror("dtconv: could not open stdin");
			goto out;
		}
		while (prchunk_fill(pctx) >= 0) {
			for (char *line; prchunk_haslinep(pctx); lno++) {
				size_t UNUSED(llen);
				struct dt_dt_s d2;
				struct dt_dt_s dur;

				llen = prchunk_getline(pctx, &line);
				d2 = dt_io_strpdt(line, fmt, nfmt, NULL);

				if (dt_unk_p(d2)) {
					if (!argi->quiet_given) {
						dt_io_warn_strpdt(line);
					}
					continue;
				}
				/* perform subtraction now */
				dur = dt_dtdiff(dtyp, d, d2);
				ddiff_prnt(dur, ofmt, dfmt);
			}
		}
		/* get rid of resources */
		free_prchunk(pctx);
	}

out:
	cmdline_parser_free(argi);
	return res;
}

/* ddiff.c ends here */
