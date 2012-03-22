/*** tdiff.c -- perform simple time arithmetic, time minus time
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
#include "time-core.h"
#include "time-io.h"

#if !defined UNUSED
# define UNUSED(_x)	__attribute__((unused)) _x
#endif	/* !UNUSED */


static int
tdiff_prnt(struct dt_t_s dur, const char *UNUSED(fmt))
{
	char buf[256];

	snprintf(buf, sizeof(buf), "%ds\n", dur.sdur);
	fputs(buf, stdout);
	return 0;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wswitch-enum"
#endif	/* __INTEL_COMPILER */
#include "tdiff-clo.h"
#include "tdiff-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
# pragma warning (default:181)
#elif defined __GNUC__
# pragma GCC diagnostic warning "-Wswitch-enum"
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	struct dt_t_s t;
	const char *ofmt;
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

	ofmt = argi->format_arg;
	fmt = argi->input_format_arg;
	nfmt = argi->input_format_given;

	if (argi->inputs_num == 0 ||
	    (t = dt_io_strpt(argi->inputs[0], fmt, nfmt)).s < 0) {
		fputs("Error: reference TIME must be specified\n\n", stderr);
		cmdline_parser_print_help();
		res = 1;
		goto out;
	}

	if (argi->inputs_num > 1) {
		for (size_t i = 1; i < argi->inputs_num; i++) {
			struct dt_t_s t2;
			const char *inp = argi->inputs[i];

			if ((t2 = dt_io_strpt(inp, fmt, nfmt)).s >= 0) {
				struct dt_t_s dur = dt_tdiff(t, t2);
				tdiff_prnt(dur, ofmt);
			} else if (!argi->quiet_given) {
				dt_io_warn_strpt(inp);
			}
		}
	} else {
		/* read from stdin */
		FILE *fp = stdin;
		char *line;
		size_t lno = 0;

		/* no threads reading this stream */
		__io_setlocking_bycaller(fp);
		__io_setlocking_bycaller(stdout);

		for (line = NULL; !__io_eof_p(fp); lno++) {
			ssize_t n;
			size_t len;
			struct dt_t_s t2;

			n = getline(&line, &len, fp);
			if (n < 0) {
				break;
			}
			/* terminate the string accordingly */
			line[n - 1] = '\0';
			/* perform addition now */
			if ((t2 = dt_io_strpt(line, fmt, nfmt)).s >= 0) {
				struct dt_t_s dur = dt_tdiff(t, t2);
				tdiff_prnt(dur, ofmt);
			} else if (!argi->quiet_given) {
				dt_io_warn_strpt(line);
			}
		}
		/* get rid of resources */
		free(line);
		goto out;
	}

out:
	cmdline_parser_free(argi);
	return res;
}

/* tdiff.c ends here */
