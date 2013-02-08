/*** dgrep.c -- grep for lines with dates
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
#include <errno.h>
#include <stdarg.h>

#include "dt-core.h"
#include "dt-io.h"
#include "tzraw.h"
#include "dexpr.h"
#include "prchunk.h"


/* error impl */
static void
__attribute__((format(printf, 2, 3)))
error(int eno, const char *fmt, ...)
{
	va_list vap;
	va_start(vap, fmt);
	fputs("dgrep: ", stderr);
	vfprintf(stderr, fmt, vap);
	va_end(vap);
	if (eno) {
		fputc(':', stderr);
		fputc(' ', stderr);
		fputs(strerror(eno), stderr);
	}
	fputc('\n', stderr);
	return;
}

/* dexpr subsystem */
#include "dexpr.c"

struct prln_ctx_s {
	struct grep_atom_soa_s *ndl;
	dexpr_t root;
	int only_matching_p;
};

static void
proc_line(struct prln_ctx_s ctx, char *line, size_t llen)
{
	struct dt_dt_s d;
	char *sp = NULL;
	char *ep = NULL;

	/* check if line matches,
	 * there's currently no way to specify NEEDLE */
	for (char *lp = line; ; lp = ep) {
		d = dt_io_find_strpdt2(lp, ctx.ndl, &sp, &ep, NULL);

		if (!dt_unk_p(d) && dexpr_matches_p(ctx.root, d)) {
			if (!ctx.only_matching_p) {
				sp = line;
				ep = line + llen;
			}
			/* make sure we finish the line */
			*ep++ = '\n';
			__io_write(sp, ep - sp, stdout);
			break;
		} else if (dt_unk_p(d)) {
			/* just plain nothing */
			break;
		}
		/* otherwise it means the line has no match (yet) */
		;
	}
	return;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wswitch-enum"
# pragma GCC diagnostic ignored "-Wunused-function"
#endif	/* __INTEL_COMPILER */
#include "dgrep-clo.h"
#include "dgrep-clo.c"
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
	ckv_fmt = fmt = argi->input_format_arg;
	ckv_nfmt = nfmt = argi->input_format_given;
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
		error(0, "Error: need an expression to grep");
		goto out;
	}
	/* fixup o, default is OP_EQ */
	if (o != OP_UNK && root->type != DEX_VAL) {
		res = 1;
		error(0, "\
long opt operators (--lt, --gt, ...) cannot be used in conjunction \n\
with complex expressions");
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
		size_t lno = 0;
		struct grep_atom_s __nstk[16], *needle = __nstk;
		size_t nneedle = countof(__nstk);
		struct grep_atom_soa_s ndlsoa;
		void *pctx;
		struct prln_ctx_s prln = {
			.ndl = &ndlsoa,
			.root = root,
			.only_matching_p = argi->only_matching_given,
		};

		/* no threads reading this stream */
		__io_setlocking_bycaller(stdout);

		/* lest we overflow the stack */
		if (nfmt >= nneedle) {
			/* round to the nearest 8-multiple */
			nneedle = (nfmt | 7) + 1;
			needle = calloc(nneedle, sizeof(*needle));
		}
		/* and now build the needle */
		ndlsoa = build_needle(needle, nneedle, fmt, nfmt);

		/* using the prchunk reader now */
		if ((pctx = init_prchunk(STDIN_FILENO)) == NULL) {
			error(errno, "Error: could not open stdin");
			goto ndl_free;
		}
		while (prchunk_fill(pctx) >= 0) {
			for (char *line; prchunk_haslinep(pctx); lno++) {
				size_t llen = prchunk_getline(pctx, &line);

				proc_line(prln, line, llen);
			}
		}
		/* get rid of resources */
		free_prchunk(pctx);
	ndl_free:
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
