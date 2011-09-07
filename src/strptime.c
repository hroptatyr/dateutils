/*** strptime.c -- a shell interface to strptime(3)
 *
 * Copyright (C) 2011 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of datetools.
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
#include <stdio_ext.h>
#include <sys/time.h>
#include <time.h>

static void
prline(const char *line, const char *const *fmt, size_t nfmt, const char *ofmt)
{
	struct tm tm[1] = {{0}};
	size_t i = 0;

	for (i = 0; i < nfmt && !strptime(line, fmt[i], tm); i++);

	if (i < nfmt) {
		char res[256];
		strftime(res, sizeof(res), ofmt, tm);
		fputs(res, stdout);
	}
	return;
}

static void
prlines(const char *const *fmt, size_t nfmt, const char *ofmt)
{
	FILE *fp = stdin;
	char *line;
	size_t lno = 0;

	/* no threads reading this stream */
	__fsetlocking(fp, FSETLOCKING_BYCALLER);

	for (line = NULL; !feof_unlocked(fp); lno++) {
		ssize_t n;
		size_t len;

		n = getline(&line, &len, fp);
		if (n < 0) {
			break;
		}
		/* terminate the string accordingly */
		line[n - 1] = '\0';
		/* check if line matches */
		prline(line, fmt, nfmt, ofmt);
	}
	/* get rid of resources */
	free(line);
	return;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
#endif	/* __INTEL_COMPILER */
#include "strptime-clo.h"
#include "strptime-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	char outfmt[] = "%Y-%m-%d\n\0H:%M:%S %Z\n";
	int res = 0;

	if (cmdline_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	}

	if (argi->time_given) {
		outfmt[8] = ' ';
		outfmt[9] = '%';
	}

	/* get lines one by one, apply format string and print date/time */
	prlines(argi->inputs, argi->inputs_num, outfmt);

out:
	cmdline_parser_free(argi);
	return res;
}

/* strptime.c ends here */
