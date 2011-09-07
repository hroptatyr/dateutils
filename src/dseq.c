/*** dseq.c -- like seq(1) but for dates
 *
 * Copyright (C) 2009 - 2011 Sebastian Freundt
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

static char *fmt = "%Y-%m-%d";

static void
pr_ts(time_t ts)
{
	struct tm tm;
	static char b[32];

	memset(&tm, 0, sizeof(tm));
	(void)gmtime_r(&ts, &tm);
	strftime(b, sizeof(b), fmt, &tm);
	fputs(b, stdout);
	return;
}

static time_t
sc_ts(const char *s)
{
	struct tm tm;

	/* basic sanity check */
	if (s == NULL) {
		return 0;
	}
	/* wipe tm */
	memset(&tm, 0, sizeof(tm));
	(void)strptime(s, fmt, &tm);
	return mktime(&tm);
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
#endif	/* __INTEL_COMPILER */
#include "dseq-clo.h"
#include "dseq-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	time_t fst, lst;
	long int ite = 1;
	char *sep = "\n";
	int res = 0;

	if (cmdline_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	}

	switch (argi->inputs_num) {
	default:
		cmdline_parser_print_help();
		res = 1;
		goto out;
	case 1:
		fst = sc_ts(argi->inputs[0]);
		lst = time(NULL);
		break;
	case 2:
		fst = sc_ts(argi->inputs[0]);
		lst = sc_ts(argi->inputs[1]);
		break;
	case 3:
		fst = sc_ts(argi->inputs[0]);
		ite = strtol(argi->inputs[1], NULL, 10);
		lst = sc_ts(argi->inputs[2]);
		break;
	}

	while (fst <= lst) {
		pr_ts(fst);
		fputs(sep, stdout);
		fst += ite * 86400;
	}
out:
	cmdline_parser_free(argi);
	return res;
}

/* dseq.c ends here */
