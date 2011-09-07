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

static char **infmt = NULL;
static size_t ninfmt = 0;
static char *outfmt = "%Y-%m-%d\n";

static void
prline(const char *line)
{
	struct tm tm[1] = {{0}};
	size_t i = 0;

	for (i = 0; i < ninfmt && !strptime(line, infmt[i], tm); i++);

	if (i < ninfmt) {
		char res[256];
		strftime(res, sizeof(res), outfmt, tm);
		fputs(res, stdout);
	}
	return;
}

static void
prlines(void)
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
		prline(line);
	}
	/* get rid of resources */
	free(line);
	return;
}


static void
usage(void)
{
	fputs("\
Usage: strptime [-t] IN_FORMAT [IN_FORMAT ...]\n\
\n\
Parse input from stdin according to the specified IN_FORMAT.\n\
The format string specifiers are the same as for strptime(3).\n\
\n\
Options:\n\
-t  also display time in the output, default is to display the date\n\
\n\
strptime v0.1 is part of the rolf tools.\n", stderr);
	return;
}

static int
init(int argc, char *argv[])
{
	infmt = malloc(argc * sizeof(char*));

	/* parse cli args */
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-h") ||
		    !strcmp(argv[i], "--help")) {
			usage();
			return -1;
		} else if (!strcmp(argv[i], "-t")) {
			outfmt = "%Y-%m-%d %H:%M:%S %Z\n";
		} else {
			/* must be the format string */
			infmt[ninfmt++] = argv[i];
		}
	}
	return 0;
}

static void
deinit(void)
{
	free(infmt);
	return;
}

int
main(int argc, char *argv[])
{
	/* initialise */
	if (init(argc, argv)) {
		deinit();
		return 0;
	}

	/* last checks */
	if (ninfmt == 0 || argc <= 1) {
		usage();
		deinit();
		return 1;
	}
	/* get lines one by one, apply format string and print date/time */
	prlines();

	/* free our resources */
	deinit();
	return 0;
}

/* strptime.c ends here */
