/*** dadd.c -- perform simple date arithmetic, date plus duration
 *
 * Copyright (C) 2011 Sebastian Freundt
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
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include "date-core.h"
#include "date-io.h"


static struct dt_d_s
dadd_add(struct dt_d_s d, struct dt_dur_s dur[], size_t ndur)
{
	for (size_t i = 0; i < ndur; i++) {
		d = dt_add(d, dur[i]);
	}
	return d;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#endif	/* __INTEL_COMPILER */
#include "dadd-clo.h"
#include "dadd-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
# pragma warning (default:181)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	struct dt_d_s d;
	struct __strpdur_st_s st = {0};
	char *inp;
	const char *ofmt;
	char **fmt;
	size_t nfmt;
	int res = 0;
	size_t beg_idx = 0;

	/* fixup negative numbers, A -1 B for dates A and B */
	fixup_argv(argc, argv, "Pp+");
	if (cmdline_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	} else if (argi->inputs_num == 0) {
		fputs("Error: DATE or DURATION must be specified\n\n", stderr);
		cmdline_parser_print_help();
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

	/* check first arg, if it's a date the rest of the arguments are
	 * durations, if not, dates must be read from stdin */
	inp = unfixup_arg(argi->inputs[0]);
	if ((d = dt_io_strpd(inp, fmt, nfmt)).typ > DT_UNK) {
		/* ah good, it's a date */
		beg_idx++;
	}

	/* check durations */
	for (size_t i = beg_idx; i < argi->inputs_num; i++) {
		inp = unfixup_arg(argi->inputs[i]);
		do {
			if (dt_io_strpdur(&st, inp) < 0) {
				fprintf(stderr, "Error: \
cannot parse duration string `%s'\n", st.istr);
			}
		} while (__strpdur_more_p(&st));
	}
	if (st.ndurs == 0) {
		fputs("Error: no duration given\n\n", stderr);
		cmdline_parser_print_help();
		res = 1;
		goto out;
	}

	/* start the actual work */
	if (beg_idx > 0) {
		if ((d = dadd_add(d, st.durs, st.ndurs)).typ > DT_UNK) {
			dt_io_write(d, ofmt);
			res = 0;
		} else {
			res = 1;
		}
	} else {
		/* read from stdin */
		FILE *fp = stdin;
		char *line;
		size_t lno = 0;
		const char *needle = "\t";
		size_t needlen = 1;

		/* no threads reading this stream */
		__fsetlocking(fp, FSETLOCKING_BYCALLER);
		/* no threads reading this stream */
		__fsetlocking(stdout, FSETLOCKING_BYCALLER);

		if (argi->sed_mode_given && argi->sed_mode_arg) {
			needle = argi->sed_mode_arg;
			needlen = strlen(needle);
		}
		for (line = NULL; !feof_unlocked(fp); lno++) {
			ssize_t n;
			size_t len;

			n = getline(&line, &len, fp);
			if (n < 0) {
				break;
			}
			/* perform addition now */
			if (argi->sed_mode_given) {
				const char *sp = NULL;
				const char *ep = NULL;

				if ((d = dt_io_find_strpd(
					     line, fmt, nfmt,
					     needle, needlen,
					     (char**)&sp, (char**)&ep)).typ) {
					d = dadd_add(d, st.durs, st.ndurs);
					dt_io_write_sed(
						d, ofmt, line, n, sp, ep);
				} else if (!argi->quiet_given) {
					goto warn;
				}
			} else if ((d = dt_io_strpd(line, fmt, nfmt)).typ) {
				d = dadd_add(d, st.durs, st.ndurs);
				dt_io_write(d, ofmt);
			} else if (!argi->quiet_given) {
			warn:
				dt_io_warn_strpd(line);
			}
		}
		/* get rid of resources */
		free(line);
		goto out;
	}
	/* free the strpdur status */
	__strpdur_free(&st);

out:
	cmdline_parser_free(argi);
	return res;
}

/* dcal.c ends here */
