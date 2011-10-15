/*** tgrep.c -- grep for lines with time values
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
#include "time-core.h"
#include "time-io.h"

typedef uint32_t oper_t;

enum {
	OP_UNK = 0,
	/* bit 1 set */
	OP_EQ,
	/* bit 2 set */
	OP_LT,
	OP_LE,
	/* bit 3 set */
	OP_GT,
	OP_GE,
	/* bits 2 and 3 set */
	OP_NE,
	/* bits 1, 2 and 3 set */
	OP_TRUE,
};


static oper_t
find_oper(const char *s, char **ep)
{
#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#endif	/* __INTEL_COMPILER */
	oper_t res = OP_UNK;

	switch (*s) {
	default:
		break;
	case '<':
		switch (*++s) {
		default:
			res = OP_LT;
			break;
		case '=':
			res = OP_LE;
			s++;
			break;
		case '>':
			res = OP_NE;
			s++;
			break;
		}
		break;
	case '>':
		switch (*++s) {
		default:
			res = OP_GT;
			break;
		case '=':
			res = OP_GE;
			s++;
			break;
		}
		break;
	case '=':
		switch (*++s) {
		case '=':
			s++;
		default:
			res = OP_EQ;
			break;
		case '>':
			res = OP_GE;
			s++;
			break;
		}
		break;
	case '!':
		switch (*++s) {
		case '=':
			s++;
		default:
			res = OP_NE;
			break;
		}
		break;
	}
	if (ep) {
		*ep = (char*)s;
	}
	return res;
#if defined __INTEL_COMPILER
# pragma warning (default:2203)
#endif	/* __INTEL_COMPILER */
}

static bool
matchp(struct dt_t_s linet, struct dt_t_s reft, oper_t o)
{
	int cmp = dt_tcmp(linet, reft);

	switch (o) {
	case OP_EQ:
		return cmp == 0;
	case OP_LT:
		return cmp < 0;
	case OP_LE:
		return cmp <= 0;
	case OP_GT:
		return cmp > 0;
	case OP_GE:
		return cmp >= 0;
	case OP_NE:
		return cmp != 0;
	case OP_TRUE:
		return true;
	case OP_UNK:
	default:
		return false;
	}
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#endif	/* __INTEL_COMPILER */
#include "tgrep-clo.h"
#include "tgrep-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
# pragma warning (default:181)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	struct dt_t_s reft;
	char **fmt;
	size_t nfmt;
	char *inp;
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
	if (UNLIKELY(argi->inputs_num == 0)) {
		o = OP_TRUE;
	} else if (argi->inputs_num != 1 ||
	    (o |= find_oper(argi->inputs[0], &inp)) == OP_TRUE ||
	    (reft = dt_io_strpt(inp, fmt, nfmt)).s < 0) {
		res = 1;
		fputs("need a TIME to grep\n", stderr);
		goto out;
	}

	/* fixup o, default is OP_EQ */
	o = o ?: OP_EQ;
	{
		/* read from stdin */
		FILE *fp = stdin;
		char *line;
		size_t lno = 0;
		struct tgrep_atom_s __nstk[16], *needle = __nstk;
		size_t nneedle = countof(__nstk);
		struct tgrep_atom_soa_s ndlsoa;

		/* no threads reading this stream */
		__fsetlocking(fp, FSETLOCKING_BYCALLER);
		/* no threads reading this stream */
		__fsetlocking(stdout, FSETLOCKING_BYCALLER);

		/* lest we overflow the stack */
		if (nfmt >= nneedle) {
			/* round to the nearest 8-multiple */
			nneedle = (nfmt | 7) + 1;
			needle = calloc(nneedle, sizeof(*needle));
		}
		/* and now build the needle */
		ndlsoa = build_tneedle(needle, nneedle, fmt, nfmt);

		for (line = NULL; !feof_unlocked(fp); lno++) {
			ssize_t n;
			size_t len;
			struct dt_t_s t;
			const char *sp = NULL;
			const char *ep = NULL;

			n = getline(&line, &len, fp);
			if (n < 0) {
				break;
			}
			/* check if line matches,
			 * there is currently no way to specify NEEDLE */
			t = dt_io_find_strpt2(
				line, &ndlsoa, (char**)&sp, (char**)&ep);
			if (t.s >= 0 && matchp(t, reft, o)) {
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

out:
	cmdline_parser_free(argi);
	return res;
}

/* tgrep.c ends here */
