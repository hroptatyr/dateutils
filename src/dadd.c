/*** dadd.c -- perform simple date arithmetic, date plus duration
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

#include "dt-core.h"
#include "dt-io.h"
#include "dt-core-tz-glue.h"
#include "dt-locale.h"
#include "prchunk.h"

const char *prog = "dadd";


static bool
durs_only_d_p(struct dt_dtdur_s dur[], size_t ndur)
{
	for (size_t i = 0; i < ndur; i++) {
		if (dur[i].durtyp >= (dt_dtdurtyp_t)DT_NDURTYP) {
			return false;
		}
	}
	return true;
}

static struct dt_dt_s
dadd_add(struct dt_dt_s d, struct dt_dtdur_s dur[], size_t ndur)
{
	for (size_t i = 0; i < ndur; i++) {
		d = dt_dtadd(d, dur[i]);
	}
	return d;
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
	int empty_mode_p;
	int quietp;
};

static int
proc_line(const struct mass_add_clo_s *clo, char *line, size_t llen)
{
	struct dt_dt_s d;
	char *sp = NULL;
	char *ep = NULL;
	size_t nmatch = 0U;
	int rc = 0;

	do {
		/* check if line matches, */
		d = dt_io_find_strpdt2(
			line, llen, clo->gra, &sp, &ep, clo->fromz);

		if (!dt_unk_p(d)) {
			if (UNLIKELY(d.fix) && !clo->quietp) {
				rc = 2;
			}
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
				nmatch++;
			} else {
				dt_io_write(d, clo->ofmt, clo->z, '\n');
				break;
			}
		} else if (clo->sed_mode_p) {
			llen = !(clo->empty_mode_p && !nmatch) ? llen : 0U;
			line[llen] = '\n';
			__io_write(line, llen + 1, stdout);
			break;
		} else if (clo->empty_mode_p) {
			__io_write("\n", 1U, stdout);
			break;
		} else {
			/* obviously unmatched, warn about it in non -q mode */
			if (!clo->quietp) {
				dt_io_warn_strpdt(line);
				rc = 2;
			}
			break;
		}
	} while (1);
	return rc;
}

static int
mass_add_dur(const struct mass_add_clo_s *clo)
{
/* read lines from stdin
 * interpret as dates
 * add to reference duration
 * output */
	size_t lno = 0;
	int rc = 0;

	for (char *line; prchunk_haslinep(clo->pctx); lno++) {
		size_t llen = prchunk_getline(clo->pctx, &line);

		rc |= proc_line(clo, line, llen);
	}
	return rc;
}

static int
mass_add_d(const struct mass_add_clo_s *clo)
{
/* read lines from stdin
 * interpret as durations
 * add to reference date
 * output */
	size_t lno = 0;
	struct dt_dt_s d;
	struct __strpdtdur_st_s st = {0};
	int rc = 0;

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
			if (UNLIKELY(clo->rd.fix) && !clo->quietp) {
				rc = 2;
			}
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
			rc = 2;
		}
		/* just reset the ndurs slot */
		st.ndurs = 0;
	}
	/* free associated duration resources */
	__strpdtdur_free(&st);
	return rc;
}


#include "dadd.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	struct dt_dt_s d;
	struct __strpdtdur_st_s st = {0};
	const char *ofmt;
	char **fmt;
	size_t nfmt;
	int rc = 0;
	bool dt_given_p = false;
	zif_t fromz = NULL;
	zif_t z = NULL;
	zif_t hackz = NULL;

	if (yuck_parse(argi, argc, argv)) {
		rc = 1;
		goto out;
	} else if (argi->nargs == 0) {
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
	if (argi->base_arg) {
		struct dt_dt_s base = dt_strpdt(argi->base_arg, NULL, NULL);
		dt_set_base(base);
	}

	/* sanity checks, decide whether we're a mass date adder
	 * or a mass duration adder, or both, a date and durations are
	 * present on the command line */
	with (const char *inp = argi->args[0U]) {
		/* date parsing needed postponing as we need to find out
		 * about the durations */
		if (!dt_unk_p(dt_io_strpdt(inp, fmt, nfmt, NULL))) {
			dt_given_p = true;
		}
	}

	/* check first arg, if it's a date the rest of the arguments are
	 * durations, if not, dates must be read from stdin */
	for (size_t i = dt_given_p; i < argi->nargs; i++) {
		const char *inp = argi->args[i];
		do {
			if (dt_io_strpdtdur(&st, inp) < 0) {
				serror("Error: \
cannot parse duration string `%s'", st.istr);
				rc = 1;
				goto clear;
			}
		} while (__strpdtdur_more_p(&st));
	}
	/* check if there's only d durations */
	hackz = durs_only_d_p(st.durs, st.ndurs) ? NULL : fromz;

	/* read the first argument again in light of a completely parsed
	 * duration sequence */
	if (dt_given_p) {
		const char *inp = argi->args[0U];
		if (dt_unk_p(d = dt_io_strpdt(inp, fmt, nfmt, hackz))) {
			error("\
Error: cannot interpret date/time string `%s'", inp);
			rc = 1;
			goto clear;
		}
	}

	/* start the actual work */
	if (dt_given_p && st.ndurs) {
		if (!dt_unk_p(d = dadd_add(d, st.durs, st.ndurs))) {
			if (UNLIKELY(d.fix) && !argi->quiet_flag) {
				rc = 2;
			}
			if (hackz == NULL && fromz != NULL) {
				/* fixup zone */
				d = dtz_forgetz(d, fromz);
			}
			dt_io_write(d, ofmt, z, '\n');
		} else {
			rc = 1;
		}

	} else if (st.ndurs && !argi->sed_mode_flag && argi->empty_mode_flag) {
		size_t lno = 0U;
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
				/* do the adding */
				d = dadd_add(d, st.durs, st.ndurs);
				if (UNLIKELY(dt_unk_p(d))) {
					goto empty;
				}

				if (hackz == NULL && fromz != NULL) {
					/* fixup zone */
					d = dtz_forgetz(d, fromz);
				}
				dt_io_write(d, ofmt, z, '\n');
				continue;
			empty:
				__io_write("\n", 1U, stdout);
			}
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
			serror("could not open stdin");
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
		clo->sed_mode_p = argi->sed_mode_flag;
		clo->empty_mode_p = argi->empty_mode_flag;
		clo->quietp = argi->quiet_flag;
		while (prchunk_fill(pctx) >= 0) {
			rc |= mass_add_dur(clo);
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
			serror("could not open stdin");
			goto clear;
		}

		/* build the clo and then loop */
		clo->pctx = pctx;
		clo->rd = d;
		clo->fromz = fromz;
		clo->hackz = hackz;
		clo->z = z;
		clo->ofmt = ofmt;
		clo->sed_mode_p = argi->sed_mode_flag;
		clo->empty_mode_p = argi->empty_mode_flag;
		clo->quietp = argi->quiet_flag;
		while (prchunk_fill(pctx) >= 0) {
			rc |= mass_add_d(clo);
		}
		/* get rid of resources */
		free_prchunk(pctx);
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

/* dadd.c ends here */
