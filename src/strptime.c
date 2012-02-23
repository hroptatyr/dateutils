/*** strptime.c -- a shell interface to strptime(3)
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
 ***/

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "date-io.h"

static int
pars_line(struct tm *tm, const char *const *fmt, size_t nfmt, const char *line)
{
	for (size_t i = 0; i < nfmt; i++) {
		if (strptime(line, fmt[i], tm)) {
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

static void
proc_line(
	const char *ln, const char *const *fmt, size_t nfmt,
	const char *ofmt,
	int quietp)
{
	struct tm tm = {0};

	if (pars_line(&tm, fmt, nfmt, ln) < 0) {
		if (!quietp) {
			dt_io_warn_strpd(ln);
		}
	} else {
		prnt_line(ofmt, &tm);
	}
	return;
}

static void
proc_lines(const char *const *fmt, size_t nfmt, const char *ofmt, int quietp)
{
	FILE *fp = stdin;
	char *line;
	size_t lno = 0;

	/* no threads reading this stream */
	__io_setlocking_bycaller(fp);

	for (line = NULL; !__io_eof_p(fp); lno++) {
		ssize_t n;
		size_t len;

		n = getline(&line, &len, fp);
		if (n < 0) {
			break;
		}
		/* terminate the string accordingly */
		line[n - 1] = '\0';
		/* check if line matches */
		proc_line(line, fmt, nfmt, ofmt, quietp);
	}
	/* get rid of resources */
	free(line);
	return;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wswitch-enum"
#endif	/* __INTEL_COMPILER */
#include "strptime-clo.h"
#include "strptime-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
# pragma warning (default:181)
#elif defined __GNUC__
# pragma GCC diagnostic warning "-Wswitch-enum"
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	static char dflt_fmt[] = "%Y-%m-%d\n\0H:%M:%S %Z\n";
	struct gengetopt_args_info argi[1];
	char *outfmt = dflt_fmt;
	char **infmt;
	size_t ninfmt;
	char **input;
	size_t ninput;
	int quietp;
	int res = 0;

	if (cmdline_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	}

	if (argi->format_given) {
		outfmt = argi->format_arg;
		/* unescape sequences, maybe */
		if (argi->backslash_escapes_given) {
			dt_io_unescape(outfmt);
		}
	} else if (argi->time_given) {
		outfmt[8] = ' ';
		outfmt[9] = '%';
	}

	if (!argi->input_format_given) {
		infmt = argi->inputs;
		ninfmt = argi->inputs_num;
		input = NULL;
		ninput = 0;
	} else {
		infmt = argi->input_format_arg;
		ninfmt = argi->input_format_given;
		input = argi->inputs;
		ninput = argi->inputs_num;
	}
	/* get quiet predicate */
	quietp = argi->quiet_given;

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
	cmdline_parser_free(argi);
	return res;
}

/* strptime.c ends here */
