/*** tseq.c -- like seq(1) but for times
 *
 * Copyright (C) 2009 - 2011 Sebastian Freundt
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
 ***/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "time-core.h"
#include "time-io.h"

typedef void *__skipspec_t;

/* generic closure */
struct tseq_clo_s {
	struct dt_t_s fst;
	struct dt_t_s lst;
	struct dt_t_s ite;
	struct dt_t_s altite;
	__skipspec_t ss;
	/* direction, >0 if increasing, <0 if decreasing, 0 if undefined */
	int dir;
	int flags;
};


/* skip system */
static int
skipp(__skipspec_t UNUSED(ss), struct dt_t_s UNUSED(dt))
{
	return 0;
}


static struct dt_t_s
time_add(struct dt_t_s t, struct dt_t_s dur)
{
	signed int ite;

	ite = t.hms.s + dur.s;
	t.hms.s = ite % (signed int)SECS_PER_MINUTE;
	ite /= (signed int)SECS_PER_MINUTE;

	ite += t.hms.m;
	t.hms.m = ite % (signed int)MINS_PER_HOUR;
	ite /= (signed int)MINS_PER_HOUR;

	ite += t.hms.h;
	t.hms.h = ite % (signed int)HOURS_PER_DAY;
	return t;
}

static struct dt_t_s
time_neg_dur(struct dt_t_s dur)
{
	struct dt_t_s res = {.s = -dur.s};
	return res;
}

static bool
__in_range_p(struct dt_t_s now, struct tseq_clo_s *clo)
{
	return (now.u >= clo->fst.u && now.u <= clo->lst.u) ||
		(now.u <= clo->fst.u && now.u >= clo->lst.u);
}

static struct dt_t_s
__seq_altnext(struct dt_t_s now, struct tseq_clo_s *clo)
{
	do {
		now = time_add(now, clo->altite);
	} while (skipp(clo->ss, now) && __in_range_p(now, clo));
	return now;
}

static struct dt_t_s
__seq_this(struct dt_t_s now, struct tseq_clo_s *clo)
{
/* if NOW is on a skip date, find the next date according to ALTITE, then ITE */
	if (!skipp(clo->ss, now) && __in_range_p(now, clo)) {
		return now;
	} else if (clo->altite.u) {
		return __seq_altnext(now, clo);
	} else if (clo->ite.u) {
		do {
			now = time_add(now, clo->ite);
		} while (skipp(clo->ss, now) && __in_range_p(now, clo));
	} else {
		/* good question */
		;
	}
	return now;
}

static struct dt_t_s
__seq_next(struct dt_t_s now, struct tseq_clo_s *clo)
{
/* advance NOW, then fix it */
	return __seq_this(time_add(now, clo->ite), clo);
}

static int
__get_dir(struct dt_t_s t, struct tseq_clo_s *clo)
{
	struct dt_t_s tmp;

	/* trial addition to to see where it goes */
	tmp = __seq_next(t, clo);
	if (tmp.u > t.u) {
		return 1;
	} else if (tmp.u < t.u) {
		return -1;
	} else {
		return 0;
	}
}

static struct dt_t_s
__fixup_fst(struct tseq_clo_s *clo)
{
	struct dt_t_s tmp;
	struct dt_t_s old;

	/* get direction info first */
	if ((clo->dir = __get_dir(clo->fst, clo)) > 0) {
		/* wrong direction */
		return __seq_this(clo->fst, clo);
	} else if (clo->dir == 0) {
		return (struct dt_t_s){.u = 0};
	}
	tmp = clo->lst;
	while (__in_range_p(tmp, clo)) {
		old = tmp;
		tmp = __seq_next(tmp, clo);
	}
	/* final checks */
	old = __seq_this(old, clo);
	clo->ite = time_neg_dur(clo->ite);
	/* fixup again with negated dur */
	old = __seq_this(old, clo);
	return old;
}

static struct dt_t_s
tseq_guess_ite(struct dt_t_s beg, struct dt_t_s end)
{
	struct dt_t_s res = {.s = 0};

	if (beg.hms.h != end.hms.h &&
	    beg.hms.m == 0 && end.hms.m == 0&&
	    beg.hms.s == 0 && end.hms.s == 0) {
		if (beg.u < end.u) {
			res.sdur = SECS_PER_HOUR;
		} else {
			res.sdur = -SECS_PER_HOUR;
		}
	} else if (beg.hms.m != end.hms.m &&
		   beg.hms.s == 0 && end.hms.s == 0) {
		if (beg.u < end.u) {
			res.sdur = SECS_PER_MINUTE;
		} else {
			res.sdur = -SECS_PER_MINUTE;
		}
	} else {
		if (beg.u < end.u) {
			res.sdur = 1L;
		} else {
			res.sdur = -1L;
		}
	}
	return res;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#endif	/* __INTEL_COMPILER */
#include "tseq-clo.h"
#include "tseq-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
# pragma warning (default:181)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	struct dt_t_s fst, lst, tmp;
	char **ifmt;
	size_t nifmt;
	char *ofmt;
	int res = 0;
	struct tseq_clo_s clo = {
		.ite.s = 0,
		.altite.s = 0,
		.dir = 0,
		.flags = 0,
	};

	/* fixup negative numbers, A -1 B for dates A and B */
	fixup_argv(argc, argv, NULL);
	if (cmdline_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	}
	/* assign ofmt/ifmt */
	ofmt = argi->format_arg;
	if (argi->backslash_escapes_given) {
		dt_io_unescape(ofmt);
	}
	nifmt = argi->input_format_given;
	ifmt = argi->input_format_arg;

	switch (argi->inputs_num) {
	default:
		cmdline_parser_print_help();
		res = 1;
		goto out;
	case 1:
		if ((fst = dt_io_strpt(argi->inputs[0], ifmt, nifmt)).s < 0) {
			if (!argi->quiet_given) {
				dt_io_warn_strpt(argi->inputs[0]);
			}
			res = 1;
			goto out;
		}
		lst = dt_time();
		break;
	case 2:
		if ((fst = dt_io_strpt(argi->inputs[0], ifmt, nifmt)).s < 0) {
			if (!argi->quiet_given) {
				dt_io_warn_strpt(argi->inputs[0]);
			}
			res = 1;
			goto out;
		}
		if ((lst = dt_io_strpt(argi->inputs[1], ifmt, nifmt)).s < 0) {
			if (!argi->quiet_given) {
				dt_io_warn_strpt(argi->inputs[1]);
			}
			res = 1;
			goto out;
		}
		break;
	case 3: {
		struct __strptdur_st_s st = {0};
		if ((fst = dt_io_strpt(argi->inputs[0], ifmt, nifmt)).s < 0) {
			if (!argi->quiet_given) {
				dt_io_warn_strpt(argi->inputs[0]);
			}
			res = 1;
			goto out;
		}
		unfixup_arg(argi->inputs[1]);
		do {
			if (dt_io_strptdur(&st, argi->inputs[1]) < 0) {
				fprintf(stderr, "Error: \
cannot parse duration string `%s'\n", argi->inputs[1]);
				res = 1;
				goto out;
			}
		} while (__strptdur_more_p(&st));
		/* assign values */
		clo.ite = st.curr;
		if ((lst = dt_io_strpt(argi->inputs[2], ifmt, nifmt)).s < 0) {
			if (!argi->quiet_given) {
				dt_io_warn_strpt(argi->inputs[2]);
			}
			res = 1;
			goto out;
		}
		break;
	}
	}

	/* the actual sequence now, this isn't high-performance so we
	 * decided to go for readability */
	if (fst.u <= lst.u) {
		if (clo.ite.s == 0) {
			clo.ite = tseq_guess_ite(fst, lst);
		}
		clo.fst = fst;
		clo.lst = lst;
		tmp = fst = __fixup_fst(&clo);
	} else {
		if (clo.ite.s == 0) {
			clo.ite = tseq_guess_ite(fst, lst);
		}
		clo.fst = lst;
		clo.lst = fst;
		tmp = lst = __fixup_fst(&clo);
	}
	/* last checks */
	if (tmp.u == 0) {
		/* this is fucked */
		if (!argi->quiet_given) {
			fputs("\
increment must not be naught\n", stderr);
		}
		res = 1;
		goto out;
	} else if (clo.ite.s == 0) {
		if (!argi->quiet_given) {
			fputs("\
increment must not be naught\n", stderr);
		}
		res = 1;
		goto out;
	}

	while ((clo.fst.u <= tmp.u && tmp.u <= clo.lst.u) ||
	       (clo.fst.u >= tmp.u && tmp.u >= clo.lst.u)) {
		dt_io_write(tmp, ofmt);
		tmp = __seq_next(tmp, &clo);
	}

out:
	/* free strpdur resources */
	cmdline_parser_free(argi);
	return res;
}

/* dseq.c ends here */
