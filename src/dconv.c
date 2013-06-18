/*** dconv.c -- convert calendrical and time stamp systems
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


/* error() impl */
static void
__attribute__((format(printf, 2, 3)))
error(int eno, const char *fmt, ...)
{
	va_list vap;
	va_start(vap, fmt);
	fputs("dconv: ", stderr);
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

struct prln_ctx_s {
	struct grep_atom_soa_s *ndl;
	const char *ofmt;
	zif_t fromz;
	zif_t outz;
	int sed_mode_p;
	int quietp;
};

static void
proc_line(struct prln_ctx_s ctx, char *line, size_t llen)
{
	struct dt_dt_s d;
	char *sp = NULL;
	char *ep = NULL;

	do {
		d = dt_io_find_strpdt2(line, ctx.ndl, &sp, &ep, ctx.fromz);

		/* check if line matches */
		if (!dt_unk_p(d) && ctx.sed_mode_p) {
			__io_write(line, sp - line, stdout);
			dt_io_write(d, ctx.ofmt, ctx.outz, '\0');
			llen -= (ep - line);
			line = ep;
		} else if (!dt_unk_p(d)) {
			dt_io_write(d, ctx.ofmt, ctx.outz, '\n');
			break;
		} else if (ctx.sed_mode_p) {
			line[llen] = '\n';
			__io_write(line, llen + 1, stdout);
			break;
		} else if (!ctx.quietp) {
			dt_io_warn_strpdt(line);
			break;
		}
	} while (1);
	return;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wswitch-enum"
# pragma GCC diagnostic ignored "-Wunused-function"
#endif	/* __INTEL_COMPILER */
#include "dconv-clo.h"
#include "dconv-clo.c"
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
	const char *ofmt;
	char **fmt;
	size_t nfmt;
	int res = 0;
	zif_t fromz = NULL;
	zif_t z = NULL;

	if (cmdline_parser(argc, argv, argi)) {
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
	if (argi->default_given) {
		struct dt_dt_s dflt = dt_strpdt(argi->default_arg, NULL, NULL);
		dt_set_default(dflt);
	}

	if (argi->inputs_num) {
		for (size_t i = 0; i < argi->inputs_num; i++) {
			const char *inp = argi->inputs[i];
			struct dt_dt_s d = dt_io_strpdt(inp, fmt, nfmt, fromz);

			if (!dt_unk_p(d)) {
				dt_io_write(d, ofmt, z, '\n');
			} else if (!argi->quiet_given) {
				dt_io_warn_strpdt(inp);
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
			.sed_mode_p = argi->sed_mode_given,
			.quietp = argi->quiet_given,
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
			error(errno, "Error: could not open stdin");
			goto ndl_free;
		}
		while (prchunk_fill(pctx) >= 0) {
			for (char *line; prchunk_haslinep(pctx); lno++) {
				size_t llen = prchunk_getline(pctx, &line);

				proc_line(prln, line, llen);
			}
		}
		/* get rid of resources */
		free_prchunk(pctx);
	ndl_free:
		if (needle != __nstk) {
			free(needle);
		}
	}

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

/* dconv.c ends here */
