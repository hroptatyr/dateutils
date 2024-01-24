/*** strptime.c -- a shell interface to strptime(3)
 *
 * Copyright (C) 2011-2024 Sebastian Freundt
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
pars_line(struct tm *tm, char *const *fmt, size_t nfmt, const char *line)
{
	for (size_t i = 0; i < nfmt; i++) {
		if (fmt[i] && strptime(line, fmt[i], tm) != NULL) {
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
	puts(res);
	return;
}

static int
proc_line(
	const char *ln, char *const *fmt, size_t nfmt,
	const char *ofmt,
	int quietp)
{
	struct tm tm = {0};
	int rc = 0;

	if (pars_line(&tm, fmt, nfmt, ln) < 0) {
		if (!quietp) {
			dt_io_warn_strpdt(ln);
			rc = 2;
		}
	} else {
		prnt_line(ofmt, &tm);
	}
	return rc;
}

static int
proc_lines(char *const *fmt, size_t nfmt, const char *ofmt, int quietp)
{
	size_t lno = 0;
	int rc = 0;
	void *pctx;

	/* using the prchunk reader now */
	if ((pctx = init_prchunk(STDIN_FILENO)) == NULL) {
		serror("Error: could not open stdin");
		return 1;
	}
	while (prchunk_fill(pctx) >= 0) {
		for (char *line; prchunk_haslinep(pctx); lno++) {
			(void)prchunk_getline(pctx, &line);
			/* check if line matches */
			rc |= proc_line(line, fmt, nfmt, ofmt, quietp);
		}
	}
	/* get rid of resources */
	free_prchunk(pctx);
	return rc;
}


#include "strptime.yucc"

int
main(int argc, char *argv[])
{
	static char dflt_fmt[] = "%Y-%m-%d\0%H:%M:%S %Z";
	yuck_t argi[1U];
	char *outfmt = dflt_fmt;
	int quietp;
	int rc = 0;

	if (yuck_parse(argi, argc, argv)) {
		rc = 1;
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
	if (argi->nargs == 0U) {
		/* read from stdin */
		rc |= proc_lines(argi->input_format_args, argi->input_format_nargs, outfmt, quietp);
	} else {
		for (size_t i = 0; i < argi->nargs; i++) {
			rc |= proc_line(argi->args[i], argi->input_format_args, argi->input_format_nargs, outfmt, quietp);
		}
	}

out:
	yuck_free(argi);
	return rc;
}

/* strptime.c ends here */
