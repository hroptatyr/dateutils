/*** dgrep.c -- grep for lines with dates
 *
 * Copyright (C) 2011-2022 Sebastian Freundt
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

#include "dt-core.h"
#include "dt-core-tz-glue.h"
#include "dt-io.h"
#include "dexpr.h"
#include "dt-locale.h"
#include "prchunk.h"

const char *prog = "dgrep";


/* dexpr subsystem */
#include "dexpr.c"

struct prln_ctx_s {
	struct grep_atom_soa_s *ndl;
	dexpr_t root;
	zif_t fromz;
	zif_t z;
	unsigned int only_matching_p:1U;
	unsigned int invert_match_p:1U;
};

static void
proc_line(struct prln_ctx_s ctx, char *line, size_t llen)
{
	char *osp = NULL;
	char *oep = NULL;

	/* check if line matches,
	 * there's currently no way to specify NEEDLE */
	for (char *lp = line, *const zp = line + llen, *sp, *ep;
	     /*no check*/; lp = ep, osp = sp, oep = ep) {
		struct dt_dt_s d =
			dt_io_find_strpdt2(
				lp, zp - lp, ctx.ndl, &sp, &ep, ctx.fromz);
		bool unkp = dt_unk_p(d);

		if (unkp) {
			/* just plain nothing */
			break;
		} else if (ctx.z != NULL) {
			/* promote to zone ctx.z */
			d = dtz_enrichz(d, ctx.z);
		}
		/* otherwise */
		if (dexpr_matches_p(ctx.root, d)) {
			if (ctx.invert_match_p) {
				/* nothing must match */
				return;
			} else if (!ctx.only_matching_p) {
				sp = line;
				ep = line + llen;
			}
			/* make sure we finish the line */
			*ep++ = '\n';
			__io_write(sp, ep - sp, stdout);
			return;
		}
	}
	if (ctx.invert_match_p) {
		/* no match but invert_match select, print line */
		if (!ctx.only_matching_p) {
			osp = line;
			oep = line + llen;
		} else if (osp == NULL || oep == NULL) {
			/* no date in line and only-matching is active
			 * bugger off */
			return;
		}
		/* finish the line and bugger off */
		*oep++ = '\n';
		__io_write(osp, oep - osp, stdout);
	}
	return;
}


#include "dgrep.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	char **fmt;
	size_t nfmt;
	dexpr_t root;
	oper_t o = OP_UNK;
	zif_t fromz = NULL;
	zif_t z = NULL;
	int rc = 0;

	if (yuck_parse(argi, argc, argv)) {
		rc = 1;
		goto out;
	}

	/* init and unescape sequences, maybe */
	ckv_fmt = fmt = argi->input_format_args;
	ckv_nfmt = nfmt = argi->input_format_nargs;
	if (argi->backslash_escapes_flag) {
		for (size_t i = 0; i < nfmt; i++) {
			dt_io_unescape(fmt[i]);
		}
	}
	if (argi->base_arg) {
		struct dt_dt_s base = dt_strpdt(argi->base_arg, NULL, NULL);
		dt_set_base(base);
	}

	if (argi->eq_flag) {
		o = OP_EQ;
	} else if (argi->ne_flag) {
		o = OP_NE;
	} else if (argi->lt_flag || argi->ot_flag) {
		o = OP_LT;
	} else if (argi->le_flag) {
		o = OP_LE;
	} else if (argi->gt_flag || argi->nt_flag) {
		o = OP_GT;
	} else if (argi->ge_flag) {
		o = OP_GE;
	}
	/* parse the expression */
	if (argi->nargs == 0U || 
	    dexpr_parse(&root, argi->args[0U], strlen(argi->args[0U])) < 0) {
		rc = 1;
		error("Error: need an expression to grep");
		goto out;
	}
	/* fixup o, default is OP_EQ */
	if (o != OP_UNK && root->type != DEX_VAL) {
		rc = 1;
		error("\
long opt operators (--lt, --gt, ...) cannot be used in conjunction \n\
with complex expressions");
		goto out;
	} else if (o != OP_UNK) {
		/* fiddle with the operator in the expression */
		root->kv->op = o;
	}

	if (argi->from_locale_arg) {
		setilocale(argi->from_locale_arg);
	}
	if (argi->from_zone_arg &&
	    (fromz = dt_io_zone(argi->from_zone_arg)) == NULL) {
		error("\
Error: cannot find zone specified in --from-zone: `%s'", argi->from_zone_arg);
		rc = 1;
		goto clear;
	}
	if (argi->zone_arg &&
	    (z = dt_io_zone(argi->zone_arg)) == NULL) {
		error("\
Error: cannot find zone specified in --zone: `%s'", argi->zone_arg);
		rc = 1;
		goto clear;
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
			.fromz = fromz,
			.z = z,
			.only_matching_p = argi->only_matching_flag,
			.invert_match_p = argi->invert_match_flag,
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
			serror("Error: could not open stdin");
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
clear:
	/* resource freeing */
	free_dexpr(root);
	dt_io_clear_zones();
	if (argi->from_locale_arg) {
		setilocale(NULL);
	}
out:
	yuck_free(argi);
	return rc;
}

/* dgrep.c ends here */
