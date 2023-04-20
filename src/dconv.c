/*** dconv.c -- convert calendrical and time stamp systems
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
#include "dt-locale.h"
#include "prchunk.h"


const char *prog = "dconv";

struct prln_ctx_s {
	struct grep_atom_soa_s *ndl;
	const char *ofmt;
	zif_t fromz;
	zif_t outz;
	int sed_mode_p;
	int empty_mode_p;
	int quietp;
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
		d = dt_io_find_strpdt2(
			line, llen, ctx.ndl, &sp, &ep, ctx.fromz);

		/* check if line matches */
		if (!dt_unk_p(d) && ctx.sed_mode_p) {
			__io_write(line, sp - line, stdout);
			dt_io_write(d, ctx.ofmt, ctx.outz, '\0');
			llen -= (ep - line);
			line = ep;
			nmatch++;
		} else if (!dt_unk_p(d)) {
			if (UNLIKELY(d.fix) && !ctx.quietp) {
				rc = 2;
			}
			dt_io_write(d, ctx.ofmt, ctx.outz, '\n');
			break;
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


#include "dconv.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	const char *ofmt;
	char **fmt;
	size_t nfmt;
	int rc = 0;
	zif_t fromz = NULL;
	zif_t z = NULL;

	if (yuck_parse(argi, argc, argv)) {
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

	if (argi->locale_arg) {
		setflocale(argi->locale_arg);
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

	if (argi->nargs) {
		for (size_t i = 0; i < argi->nargs; i++) {
			const char *inp = argi->args[i];
			struct dt_dt_s d = dt_io_strpdt(inp, fmt, nfmt, fromz);

			if (!dt_unk_p(d)) {
				if (UNLIKELY(d.fix) && !argi->quiet_flag) {
					rc = 2;
				}
				dt_io_write(d, ofmt, z, '\n');
			} else if (!argi->quiet_flag) {
				rc = 2;
				dt_io_warn_strpdt(inp);
			}
		}
	} else if (!argi->sed_mode_flag && argi->empty_mode_flag) {
		/* read from stdin */
		size_t lno = 0;
		void *pctx;

		/* no threads reading this stream */
		__io_setlocking_bycaller(stdout);

		/* using the prchunk reader now */
		if ((pctx = init_prchunk(STDIN_FILENO)) == NULL) {
			serror("Error: could not open stdin");
			goto clear;
		}
		while (prchunk_fill(pctx) >= 0) {
			for (char *line; prchunk_haslinep(pctx); lno++) {
				size_t llen = prchunk_getline(pctx, &line);
				struct dt_dt_s d;
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
				dt_io_write(d, ofmt, z, '\n');
				continue;
			empty:
				__io_write("\n", 1U, stdout);
			}
		}
		/* get rid of resources */
		free_prchunk(pctx);
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
		};

		/* no threads reading this stream */
		__io_setlocking_bycaller(stdout);

		/* lest we overflow the stack */
		if (nfmt >= nneedle) {
			/* round to the nearest 8-multiple */
			nneedle = (nfmt | 7) + 1;
			needle = calloc(nneedle, sizeof(*needle));
		}
		/* and now build the needles */
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
	}

clear:
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

/* dconv.c ends here */
