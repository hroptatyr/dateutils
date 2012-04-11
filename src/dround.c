/*** dround.c -- perform simple date arithmetic, round to duration
 *
 * Copyright (C) 2012 Sebastian Freundt
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
#include "tzraw.h"
#include "prchunk.h"

#if !defined assert
# define assert(x)
#endif	/* !assert */


static bool
durs_only_d_p(struct dt_dt_s dur[], size_t ndur)
{
	for (size_t i = 0; i < ndur; i++) {
		if (dur[i].t.typ) {
			return false;
		}
	}
	return true;
}

static struct dt_t_s
tround_tdur(struct dt_t_s t, struct dt_t_s dur, bool nextp)
{
/* this will return the rounded to DUR time of T and, to encode carry
 * (which can only take values 0 or 1), we will use t's neg bit */
	signed int mdur;
	signed int diff;

	/* no dur is a no-op */
	if (UNLIKELY(!dur.sdur)) {
		return t;
	}

	/* unpack t */
	mdur = t.hms.h * SECS_PER_HOUR +
		t.hms.m * SECS_PER_MINUTE +
		t.hms.s;
	if (!(diff = mdur % dur.sdur) && !t.hms.ns && !nextp) {
		return t;
	}

	/* compute the result */
	mdur -= diff;
	if (dur.sdur > 0) {
		mdur += dur.sdur;
	}
	/* and convert back */
	t.hms.ns = 0;
	t.hms.s = mdur % SECS_PER_MINUTE;
	mdur /= SECS_PER_MINUTE;
	t.hms.m = mdur % MINS_PER_HOUR;
	mdur /= MINS_PER_HOUR;
	t.hms.h = mdur % HOURS_PER_DAY;
	/* and get the carry */
	mdur /= HOURS_PER_DAY;

	/* negative carry is not possible actually */
	assert(mdur == 0 || mdur == 1);
	t.neg = !mdur ? 0 : 1;
	return t;
}


static struct dt_dt_s
dround(struct dt_dt_s d, struct dt_dt_s dur[], size_t ndur)
{
	for (size_t i = 0; i < ndur; i++) {
		if (dt_sandwich_only_t_p(dur[i]) && d.sandwich) {
			d.t = tround_tdur(d.t, dur[i].t, false);
			/* check carry */
			if (d.t.neg) {
				/* we need to add a day */
				const struct dt_d_s one_day = {
					.typ = DT_DAISY,
					.dur = 1,
					.neg = 0,
					.daisydur = 1,
				};

				d.t.neg = 0;
				d.d = dt_dadd(d.d, one_day);
			}
		}
	}
	return d;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wswitch-enum"
# pragma GCC diagnostic ignored "-Wswitch"
#endif	/* __INTEL_COMPILER */
#include "dround-clo.h"
#include "dround-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
# pragma warning (default:181)
#elif defined __GNUC__
# pragma GCC diagnostic warning "-Wswitch-enum"
# pragma GCC diagnostic warning "-Wswitch"
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	struct dt_dt_s d;
	struct __strpdtdur_st_s st = {0};
	char *inp;
	const char *ofmt;
	char **fmt;
	size_t nfmt;
	int res = 0;
	bool dt_given_p = false;
	zif_t fromz = NULL;
	zif_t z = NULL;
	zif_t hackz = NULL;

	/* fixup negative numbers, A -1 B for dates A and B */
	fixup_argv(argc, argv, "Pp+");
	if (cmdline_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	} else if (argi->inputs_num == 0) {
		fputs("Error: DATE or DURATION must be specified\n\n", stderr);
		cmdline_parser_print_help();
		res = 1;
		goto out;
	}
	/* init and unescape sequences, maybe */
	ofmt = argi->format_arg;
	fmt = argi->input_format_arg;
	nfmt = argi->input_format_given;
	if (argi->backslash_escapes_given) {
		dt_io_unescape(argi->format_arg);
		for (size_t i = 0; i < nfmt; i++) {
			dt_io_unescape(fmt[i]);
		}
	}

	/* try and read the from and to time zones */
	if (argi->from_zone_given) {
		fromz = zif_read_inst(argi->from_zone_arg);
	}
	if (argi->zone_given) {
		z = zif_read_inst(argi->zone_arg);
	}

	/* check first arg, if it's a date the rest of the arguments are
	 * durations, if not, dates must be read from stdin */
	for (size_t i = 0; i < argi->inputs_num; i++) {
		inp = unfixup_arg(argi->inputs[i]);
		do {
			if (dt_io_strpdtdur(&st, inp) < 0) {
				if (UNLIKELY(i == 0)) {
					/* that's ok, must be a date then */
					dt_given_p = true;
				} else {
					fprintf(stderr, "Error: \
cannot parse duration string `%s'\n", st.istr);
				}
			}
		} while (__strpdtdur_more_p(&st));
	}
	/* check if there's only d durations */
	hackz = durs_only_d_p(st.durs, st.ndurs) ? NULL : fromz;

	/* sanity checks */
	if (dt_given_p) {
		/* date parsing needed postponing as we need to find out
		 * about the durations */
		inp = argi->inputs[0];
		if (dt_unk_p(d = dt_io_strpdt(inp, fmt, nfmt, hackz))) {
			fprintf(stderr, "Error: \
cannot interpret date/time string `%s'\n", argi->inputs[0]);
			res = 1;
			goto out;
		}
	} else if (st.ndurs == 0) {
		fprintf(stderr, "Error: \
no durations given\n");
		res = 1;
		goto out;
	}

	/* start the actual work */
	if (dt_given_p) {
		if (!dt_unk_p(d = dround(d, st.durs, st.ndurs))) {
			if (hackz == NULL && fromz != NULL) {
				/* fixup zone */
				d = dtz_forgetz(d, fromz);
			}
			dt_io_write(d, ofmt, z);
			res = 0;
		} else {
			res = 1;
		}
	} else {
		/* read from stdin */
		size_t lno = 0;
		struct grep_atom_s __nstk[16], *needle = __nstk;
		size_t nneedle = countof(__nstk);
		struct grep_atom_soa_s ndlsoa;
		void *pctx;

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
			perror("dtconv: could not open stdin");
			goto ndl_free;
		}
		while (prchunk_fill(pctx) >= 0) {
			for (char *line; prchunk_haslinep(pctx); lno++) {
				size_t llen;
				const char *sp = NULL;
				const char *ep = NULL;

				llen = prchunk_getline(pctx, &line);
				/* check if line matches, */
				d = dt_io_find_strpdt2(
					line, &ndlsoa,
					(char**)&sp, (char**)&ep, fromz);

				/* finish with newline again */
				line[llen] = '\n';

				if (!dt_unk_p(d)) {
					/* perform addition now */
					d = dround(d, st.durs, st.ndurs);

					if (hackz == NULL && fromz != NULL) {
						/* fixup zone */
						d = dtz_forgetz(d, fromz);
					}

					if (argi->sed_mode_given) {
						dt_io_write_sed(
							d, ofmt,
							line, llen + 1,
							sp, ep, z);
					} else {
						dt_io_write(d, ofmt, z);
					}
				} else if (argi->sed_mode_given) {
					__io_write(line, llen + 1, stdout);
				} else if (!argi->quiet_given) {
					line[llen] = '\0';
					dt_io_warn_strpdt(line);
				}
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
	/* free the strpdur status */
	__strpdtdur_free(&st);

out:
	cmdline_parser_free(argi);
	return res;
}

/* dround.c ends here */
