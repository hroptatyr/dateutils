/*** dcal.c -- convert calendrical systems
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
 **/
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include "date-core.h"

typedef enum {
	PLMODE_UNK,
	PLMODE_D2P,
	PLMODE_P2D,
} plmode_t;


static int
dcal_conv(struct dt_d_s d, const char *fmt)
{
	static char buf[64];
	int res;

	switch (d.typ) {
	case DT_YMD:
		dt_strfd(buf, sizeof(buf), fmt ?: "%Y-%m-%c-%w\n", d);
		break;
	case DT_YMCD:
		dt_strfd(buf, sizeof(buf), fmt ?: "%Y-%m-%d\n", d);
		break;
	default:
		buf[0] = '\n';
		buf[1] = '\0';
		res = 1;
	}
	fputs(buf, stdout);
	return res;
}

static struct dt_d_s
read_input(const char *input, const char *const *fmt, size_t nfmt)
{
	struct dt_d_s res;

	if (nfmt == 0) {
		res = dt_strpd(input, NULL);
	} else {
		for (size_t i = 0; i < nfmt; i++) {
			if ((res = dt_strpd(input, fmt[i])).typ > DT_UNK) {
				break;
			}
		}
	}
	return res;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
#endif	/* __INTEL_COMPILER */
#include "dcal-clo.h"
#include "dcal-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	char **fmt;
	size_t nfmt;
	int res = 0;

	if (cmdline_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	}

	fmt = argi->input_format_arg;
	nfmt = argi->input_format_given;
	if (argi->inputs_num) {
		for (size_t i = 0; i < argi->inputs_num; i++) {
			const char *inp = argi->inputs[i];
			struct dt_d_s d;

			if ((d = read_input(inp, fmt, nfmt)).typ > DT_UNK) {
				dcal_conv(d, argi->format_arg);
			}
		}
	} else {
		/* read from stdin */
		FILE *fp = stdin;
		char *line;
		size_t lno = 0;

		/* no threads reading this stream */
		__fsetlocking(fp, FSETLOCKING_BYCALLER);

		for (line = NULL; !feof_unlocked(fp); lno++) {
			ssize_t n;
			size_t len;
			struct dt_d_s d;

			n = getline(&line, &len, fp);
			if (n < 0) {
				break;
			}
			/* terminate the string accordingly */
			line[n - 1] = '\0';
			/* check if line matches */
			if ((d = read_input(line, fmt, nfmt)).typ > DT_UNK) {
				dcal_conv(d, argi->format_arg);
			}
		}
		/* get rid of resources */
		free(line);
	}

out:
	cmdline_parser_free(argi);
	return res;
}

/* dcal.c ends here */
