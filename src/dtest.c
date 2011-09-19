/*** dtest.c -- like test(1) but for dates
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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "date-core.h"
#include "date-io.h"


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#endif	/* __INTEL_COMPILER */
#include "dtest-clo.h"
#include "dtest-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
# pragma warning (default:181)
#endif	/* __INTEL_COMPILER */

DECLF size_t __attribute__((unused))
dt_strfd(char *restrict buf, size_t bsz, const char *fmt, struct dt_d_s this);

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	char **ifmt;
	size_t nifmt;
	struct dt_d_s d1, d2;
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

	if ((d1 = dt_io_strpd(argi->inputs[0], ifmt, nifmt)).typ == DT_UNK) {
		fputs("cannot read first date specified\n", stderr);
		res = 2;
		goto out;
	}
	if ((d2 = dt_io_strpd(argi->inputs[1], ifmt, nifmt)).typ == DT_UNK) {
		fputs("cannot read second date specified\n", stderr);
		res = 2;
		goto out;
	}

	if (argi->cmp_given) {
		if (d1.u == d2.u) {
			res = 0;
		} else if (d1.u < d2.u) {
			res = 2;
		} else /*if (d1.u > d2.u)*/ {
			res = 1;
		}
	} else if (argi->eq_given) {
		res = 1 - (d1.u == d2.u);
	} else if (argi->ne_given) {
		res = 1 - (d1.u != d2.u);
	} else if (argi->lt_given || argi->ot_given) {
		res = 1 - (d1.u < d2.u);
	} else if (argi->le_given) {
		res = 1 - (d1.u <= d2.u);
	} else if (argi->gt_given || argi->nt_given) {
		res = 1 - (d1.u > d2.u);
	} else if (argi->ge_given) {
		res = 1 - (d1.u >= d2.u);
	}
out:
	cmdline_parser_free(argi);
	return res;
}

/* dtest.c ends here */
