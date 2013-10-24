/*** dadd.c -- perform simple date arithmetic, date plus duration
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
#include <errno.h>
#include <stdarg.h>

#include "dt-core.h"
#include "dt-io.h"
#include "tzraw.h"
#include "prchunk.h"


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

static struct dt_dt_s
dadd_add(struct dt_dt_s d, struct dt_dt_s dur[], size_t ndur)
{
	for (size_t i = 0; i < ndur; i++) {
		d = dt_dtadd(d, dur[i]);
	}
	return d;
}

static void
__attribute__((format(printf, 2, 3)))
error(int eno, const char *fmt, ...)
{
	va_list vap;
	va_start(vap, fmt);
	fputs("dadd: ", stderr);
	vfprintf(stderr, fmt, vap);
	va_end(vap);
	if (eno) {
		fputc(':', stderr);
		fputc(' ', stderr);
		fputs(strerror(eno), stderr);
	}
	fputc('\n', stderr);
	return;
}


struct mass_add_clo_s {
	void *pctx;
	const struct grep_atom_soa_s *gra;
	struct __strpdtdur_st_s st;
	struct dt_dt_s rd;
	zif_t fromz;
	zif_t hackz;
	zif_t z;
	const char *ofmt;
	int sed_mode_p;
	int quietp;
};

static void
proc_line(const struct mass_add_clo_s *clo, char *line, size_t llen)
{
	struct dt_dt_s d;
	char *sp = NULL;
	char *ep = NULL;

	do {
		/* check if line matches, */
		d = dt_io_find_strpdt2(line, clo->gra, &sp, &ep, clo->fromz);

		if (!dt_unk_p(d)) {
			/* perform addition now */
			d = dadd_add(d, clo->st.durs, clo->st.ndurs);

			if (clo->hackz == NULL && clo->fromz != NULL) {
				/* fixup zone */
				d = dtz_forgetz(d, clo->fromz);
			}

			if (clo->sed_mode_p) {
				__io_write(line, sp - line, stdout);
				dt_io_write(d, clo->ofmt, clo->z, '\0');
				llen -= (ep - line);
				line = ep;
			} else {
				dt_io_write(d, clo->ofmt, clo->z, '\n');
				break;
			}
		} else if (clo->sed_mode_p) {
			line[llen] = '\n';
			__io_write(line, llen + 1, stdout);
			break;
		} else {
			/* obviously unmatched, warn about it in non -q mode */
			if (!clo->quietp) {
				dt_io_warn_strpdt(line);
			}
			break;
		}
	} while (1);
	return;
}

static void
mass_add_dur(const struct mass_add_clo_s *clo)
{
/* read lines from stdin
 * interpret as dates
 * add to reference duration
 * output */
	size_t lno = 0;

	for (char *line; prchunk_haslinep(clo->pctx); lno++) {
		size_t llen = prchunk_getline(clo->pctx, &line);

		proc_line(clo, line, llen);
	}
	return;
}

static void
mass_add_d(const struct mass_add_clo_s *clo)
{
/* read lines from stdin
 * interpret as durations
 * add to reference date
 * output */
	size_t lno = 0;
	struct dt_dt_s d;
	struct __strpdtdur_st_s st = __strpdtdur_st_initialiser();

	for (char *line; prchunk_haslinep(clo->pctx); lno++) {
		size_t llen;
		int has_dur_p  = 1;

		llen = prchunk_getline(clo->pctx, &line);

		/* check for durations on this line */
		do {
			if (dt_io_strpdtdur(&st, line) < 0) {
				has_dur_p = 0;
			}
		} while (__strpdtdur_more_p(&st));

		/* finish with newline again */
		line[llen] = '\n';

		if (has_dur_p) {
			/* perform addition now */
			d = dadd_add(clo->rd, st.durs, st.ndurs);

			if (clo->hackz == NULL && clo->fromz != NULL) {
				/* fixup zone */
				d = dtz_forgetz(d, clo->fromz);
			}

			/* no sed mode here */
			dt_io_write(d, clo->ofmt, clo->z, '\n');
		} else if (clo->sed_mode_p) {
			__io_write(line, llen + 1, stdout);
		} else if (!clo->quietp) {
			line[llen] = '\0';
			dt_io_warn_strpdt(line);
		}
		/* just reset the ndurs slot */
		st.ndurs = 0;
	}
	/* free associated duration resources */
	__strpdtdur_free(&st);
	return;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wswitch-enum"
# pragma GCC diagnostic ignored "-Wunused-function"
#endif	/* __INTEL_COMPILER */
#include "dadd.xh"
#include "dadd.x"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
# pragma warning (default:181)
#elif defined __GNUC__
# pragma GCC diagnostic warning "-Wswitch-enum"
# pragma GCC diagnostic warning "-Wunused-function"
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	struct dt_dt_s d;
	struct __strpdtdur_st_s st = __strpdtdur_st_initialiser();
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
		error(0, "Error: DATE or DURATION must be specified\n");
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
		fromz = zif_open(argi->from_zone_arg);
	}
	if (argi->zone_given) {
		z = zif_open(argi->zone_arg);
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
					error(errno, "Error: \
cannot parse duration string `%s'", st.istr);
					res = 1;
					goto dur_out;
				}
			}
		} while (__strpdtdur_more_p(&st));
	}
	/* check if there's only d durations */
	hackz = durs_only_d_p(st.durs, st.ndurs) ? NULL : fromz;

	/* sanity checks, decide whether we're a mass date adder
	 * or a mass duration adder, or both, a date and durations are
	 * present on the command line */
	if (dt_given_p) {
		/* date parsing needed postponing as we need to find out
		 * about the durations */
		inp = argi->inputs[0];
		if (dt_unk_p(d = dt_io_strpdt(inp, fmt, nfmt, hackz))) {
			error(0, "Error: \
cannot interpret date/time string `%s'", argi->inputs[0]);
			res = 1;
			goto dur_out;
		}
	}

	/* start the actual work */
	if (dt_given_p && st.ndurs) {
		if (!dt_unk_p(d = dadd_add(d, st.durs, st.ndurs))) {
			if (hackz == NULL && fromz != NULL) {
				/* fixup zone */
				d = dtz_forgetz(d, fromz);
			}
			dt_io_write(d, ofmt, z, '\n');
			res = 0;
		} else {
			res = 1;
		}

	} else if (st.ndurs) {
		/* read dates from stdin */
		struct grep_atom_s __nstk[16], *needle = __nstk;
		size_t nneedle = countof(__nstk);
		struct grep_atom_soa_s ndlsoa;
		struct mass_add_clo_s clo[1];
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
			error(errno, "could not open stdin");
			goto ndl_free;
		}

		/* build the clo and then loop */
		clo->pctx = pctx;
		clo->gra = &ndlsoa;
		clo->st = st;
		clo->fromz = fromz;
		clo->hackz = hackz;
		clo->z = z;
		clo->ofmt = ofmt;
		clo->sed_mode_p = argi->sed_mode_given;
		clo->quietp = argi->quiet_given;
		while (prchunk_fill(pctx) >= 0) {
			mass_add_dur(clo);
		}
		/* get rid of resources */
		free_prchunk(pctx);
	ndl_free:
		if (needle != __nstk) {
			free(needle);
		}

	} else {
		/* mass-adding durations to reference date */
		struct mass_add_clo_s clo[1];
		void *pctx;

		/* no threads reading this stream */
		__io_setlocking_bycaller(stdout);

		/* using the prchunk reader now */
		if ((pctx = init_prchunk(STDIN_FILENO)) == NULL) {
			error(errno, "could not open stdin");
		}

		/* build the clo and then loop */
		clo->pctx = pctx;
		clo->rd = d;
		clo->fromz = fromz;
		clo->hackz = hackz;
		clo->z = z;
		clo->ofmt = ofmt;
		clo->sed_mode_p = argi->sed_mode_given;
		clo->quietp = argi->quiet_given;
		while (prchunk_fill(pctx) >= 0) {
			mass_add_d(clo);
		}
		/* get rid of resources */
		free_prchunk(pctx);
	}
dur_out:
	/* free the strpdur status */
	__strpdtdur_free(&st);

	if (argi->from_zone_given) {
		zif_close(fromz);
	}
	if (argi->zone_given) {
		zif_close(z);
	}

out:
	cmdline_parser_free(argi);
	return res;
}

/* dadd.c ends here */
