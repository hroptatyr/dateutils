/*** dtest.c -- like test(1) but for dates
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
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>

#include "dt-core.h"
#include "dt-io.h"


/* error() impl */
static void
__attribute__((format(printf, 2, 3)))
error(int eno, const char *fmt, ...)
{
	va_list vap;
	va_start(vap, fmt);
	fputs("dtest: ", stderr);
	vfprintf(stderr, fmt, vap);
	va_end(vap);
	if (eno || errno) {
		fputc(':', stderr);
		fputc(' ', stderr);
		fputs(strerror(eno ? eno : errno), stderr);
	}
	fputc('\n', stderr);
	return;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wswitch-enum"
# pragma GCC diagnostic ignored "-Wunused-function"
#endif	/* __INTEL_COMPILER */
#include "dtest-clo.h"
#include "dtest-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
# pragma warning (default:181)
#elif defined __GNUC__
# pragma GCC diagnostic warning "-Wswitch-enum"
# pragma GCC diagnostic warning "-Wunused-function"
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	char **ifmt;
	size_t nifmt;
	struct dt_dt_s d1, d2;
	int res = 0;

	if (cmdline_parser(argc, argv, argi)) {
		res = 2;
		goto out;
	}

	if (argi->inputs_num != 2) {
		cmdline_parser_print_help();
		res = 2;
		goto out;
	}

	ifmt = argi->input_format_arg;
	nifmt = argi->input_format_given;

	if (dt_unk_p(d1 = dt_io_strpdt(argi->inputs[0], ifmt, nifmt, NULL))) {
		if (!argi->quiet_given) {
			dt_io_warn_strpdt(argi->inputs[0]);
		}
		res = 2;
		goto out;
	}
	if (dt_unk_p(d2 = dt_io_strpdt(argi->inputs[1], ifmt, nifmt, NULL))) {
		if (!argi->quiet_given) {
			dt_io_warn_strpdt(argi->inputs[1]);
		}
		res = 2;
		goto out;
	}

	/* just do the comparison */
	if ((res = dt_dtcmp(d1, d2)) == -2) {
		/* uncomparable */
		res = 3;
	} else if (argi->cmp_given) {
		switch (res) {
		case 0:
			res = 0;
			break;
		case -1:
			res = 2;
			break;
		case 1:
			res = 1;
			break;
		case -2:
		default:
			res = 3;
			break;
		}
	} else if (argi->eq_given) {
		res = res == 0 ? 0 : 1;
	} else if (argi->ne_given) {
		res = res != 0 ? 0 : 1;
	} else if (argi->lt_given || argi->ot_given) {
		res = res == -1 ? 0 : 1;
	} else if (argi->le_given) {
		res = res == -1 || res == 0 ? 0 : 1;
	} else if (argi->gt_given || argi->nt_given) {
		res = res == 1 ? 0 : 1;
	} else if (argi->ge_given) {
		res = res == 1 || res == 0 ? 0 : 1;
	}
out:
	cmdline_parser_free(argi);
	return res;
}

/* dtest.c ends here */
