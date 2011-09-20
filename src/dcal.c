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
#include "date-io.h"


static int
dcal_conv(struct dt_d_s d, const char *fmt)
{
	static char buf[64];
	size_t n;

	n = dt_strfd(buf, sizeof(buf), fmt, d);
	fwrite(buf, sizeof(*buf), n, stdout);
	return (n > 0) - 1;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#endif	/* __INTEL_COMPILER */
#include "dcal-clo.h"
#include "dcal-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
# pragma warning (default:181)
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
	/* unescape sequences, maybe */
	if (argi->backslash_escapes_given) {
		dt_io_unescape(argi->format_arg);
	}

	fmt = argi->input_format_arg;
	nfmt = argi->input_format_given;
	if (argi->inputs_num) {
		for (size_t i = 0; i < argi->inputs_num; i++) {
			const char *inp = argi->inputs[i];
			struct dt_d_s d;

			if ((d = dt_io_strpd(inp, fmt, nfmt)).typ > DT_UNK) {
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
			if ((d = dt_io_strpd(line, fmt, nfmt)).typ > DT_UNK) {
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
