/*** dgrep.c -- grep for lines with dates
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
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include "date-core.h"
#include "date-io.h"
#include "dexpr.h"


/* dexpr subsystem */
#include "dexpr.c"


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#endif	/* __INTEL_COMPILER */
#include "dgrep-clo.h"
#include "dgrep-clo.c"
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
	dexpr_t root;
	oper_t o = OP_UNK;
	int res = 0;

	if (cmdline_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	}

	/* init and unescape sequences, maybe */
	fmt = argi->input_format_arg;
	nfmt = argi->input_format_given;
	if (argi->backslash_escapes_given) {
		for (size_t i = 0; i < nfmt; i++) {
			dt_io_unescape(fmt[i]);
		}
	}

	if (argi->eq_given) {
		o = OP_EQ;
	} else if (argi->ne_given) {
		o = OP_NE;
	} else if (argi->lt_given || argi->ot_given) {
		o = OP_LT;
	} else if (argi->le_given) {
		o = OP_LE;
	} else if (argi->gt_given || argi->nt_given) {
		o = OP_GT;
	} else if (argi->ge_given) {
		o = OP_GE;
	}
	/* parse the expression */
	if (argi->inputs_num == 0 || 
	    dexpr_parse(&root, argi->inputs[0], strlen(argi->inputs[0])) < 0) {
		res = 1;
		fputs("need an expression to grep\n", stderr);
		goto out;
	}
	/* fixup o, default is OP_EQ */
	if (o != OP_UNK && root->type != DEX_VAL) {
		res = 1;
		fputs("\
long opt operators (--lt, --gt, ...) cannot be used in conjunction \n\
with complex expressions\n",
		      stderr);
		goto out;
	} else if (o != OP_UNK) {
		/* fiddle with the operator in the expression */
		root->kv->op = o;
	}

	/* otherwise bring dexpr to normal form */
	dexpr_simplify(root);
	/* beef */
	{
		/* read from stdin */
		FILE *fp = stdin;
		char *line;
		size_t lno = 0;
		struct grep_atom_s __nstk[16], *needle = __nstk;
		size_t nneedle = countof(__nstk);
		struct grep_atom_soa_s ndlsoa;

		/* no threads reading this stream */
		__io_setlocking_bycaller(fp);
		__io_setlocking_bycaller(stdout);

		/* lest we overflow the stack */
		if (nfmt >= nneedle) {
			/* round to the nearest 8-multiple */
			nneedle = (nfmt | 7) + 1;
			needle = calloc(nneedle, sizeof(*needle));
		}
		/* and now build the needle */
		ndlsoa = build_needle(needle, nneedle, fmt, nfmt);

		for (line = NULL; !__io_eof_p(fp); lno++) {
			ssize_t n;
			size_t len;
			struct dt_d_s d;
			const char *sp = NULL;
			const char *ep = NULL;

			n = getline(&line, &len, fp);
			if (n < 0) {
				break;
			}
			/* check if line matches,
			 * there is currently no way to specify NEEDLE */
			d = dt_io_find_strpd2(
				line, &ndlsoa, (char**)&sp, (char**)&ep);
			if (d.typ && dexpr_matches_p(root, d)) {
				if (!argi->only_matching_given) {
					sp = line;
					ep = line + n - 1;
				}
				fwrite(sp, sizeof(*sp), ep - sp, stdout);
				fputc('\n', stdout);
			}
		}
		/* get rid of resources */
		free(line);
		if (needle != __nstk) {
			free(needle);
		}
	}
	/* resource freeing */
	free_dexpr(root);
out:
	cmdline_parser_free(argi);
	return res;
}

/* dgrep.c ends here */
