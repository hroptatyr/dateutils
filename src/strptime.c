/*** strptime.c -- a shell interface to strptime(3)
 *
 * Copyright (C) 2011-2016 Sebastian Freundt
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
 ***/

#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <locale.h>

#include "dt-io.h"
#include "prchunk.h"

const char *prog = "strptime";


static int
pars_line(struct tm *tm, const char *const *fmt, size_t nfmt, const char *line)
{
	for (size_t i = 0; i < nfmt; i++) {
		if (strptime(line, fmt[i], tm) != NULL) {
			return 0;
		}
	}
	return -1;
}

static void
prnt_line(const char *ofmt, struct tm *tm)
{
	char res[256];
	strftime(res, sizeof(res), ofmt, tm);
	fputs(res, stdout);
	return;
}

static inline __attribute__((pure, const)) struct tm
__tm_initialiser(void)
{
#if defined HAVE_SLOPPY_STRUCTS_INIT
	static const struct tm res = {};
#else
	static const struct tm res;
#endif	/* HAVE_SLOPPY_STRUCTS_INIT */
	return res;
}

static void
proc_line(
	const char *ln, const char *const *fmt, size_t nfmt,
	const char *ofmt,
	int quietp)
{
	struct tm tm = __tm_initialiser();

	if (pars_line(&tm, fmt, nfmt, ln) < 0) {
		if (!quietp) {
			dt_io_warn_strpdt(ln);
		}
	} else {
		prnt_line(ofmt, &tm);
	}
	return;
}

static void
proc_lines(const char *const *fmt, size_t nfmt, const char *ofmt, int quietp)
{
	size_t lno = 0;
	void *pctx;

	/* using the prchunk reader now */
	if ((pctx = init_prchunk(STDIN_FILENO)) == NULL) {
		serror("Error: could not open stdin");
		return;
	}
	while (prchunk_fill(pctx) >= 0) {
		for (char *line; prchunk_haslinep(pctx); lno++) {
			(void)prchunk_getline(pctx, &line);
			/* check if line matches */
			proc_line(line, fmt, nfmt, ofmt, quietp);
		}
	}
	/* get rid of resources */
	free_prchunk(pctx);
	return;
}


#include "strptime.yucc"

int
main(int argc, char *argv[])
{
	static char dflt_fmt[] = "%Y-%m-%d\n\0H:%M:%S %Z\n";
	yuck_t argi[1U];
	char *outfmt = dflt_fmt;
	char **infmt;
	size_t ninfmt;
	char **input;
	size_t ninput;
	int quietp;
	int res = 0;

	if (yuck_parse(argi, argc, argv)) {
		res = 1;
		goto out;
	}

	if (argi->format_arg) {
		outfmt = argi->format_arg;
		/* unescape sequences, maybe */
		if (argi->backslash_escapes_flag) {
			dt_io_unescape(outfmt);
		}
	} else if (argi->time_flag) {
		outfmt[8] = ' ';
		outfmt[9] = '%';
	}

	if (!argi->input_format_nargs) {
		infmt = argi->args;
		ninfmt = argi->nargs;
		input = NULL;
		ninput = 0;
	} else {
		infmt = argi->input_format_args;
		ninfmt = argi->input_format_nargs;
		input = argi->args;
		ninput = argi->nargs;
	}
	/* get quiet predicate */
	quietp = argi->quiet_flag;

	/* set locale specific/independent behaviour */
	with (const char *loc) {
		if (!argi->locale_flag) {
			loc = "C";
			/* we need to null out TZ for UTC */
			setenv("TZ", "", 1);
		} else {
			loc = "";
		}
		/* actually set our findings in stone */
		setlocale(LC_TIME, loc);
		tzset();
	}

	/* get lines one by one, apply format string and print date/time */
	if (ninput == 0) {
		/* read from stdin */
		proc_lines((const char*const*)infmt, ninfmt, outfmt, quietp);
	} else {
		const char *const *cinfmt = (const char*const*)infmt;
		for (size_t i = 0; i < ninput; i++) {
			proc_line(input[i], cinfmt, ninfmt, outfmt, quietp);
		}
	}

out:
	yuck_free(argi);
	return res;
}

/* strptime.c ends here */
