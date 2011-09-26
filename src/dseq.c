/*** dseq.c -- like seq(1) but for dates
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
#include <time.h>
#include <string.h>
#include "date-core.h"
#include "date-io.h"

/* idates are normally just YYYYMMDD just in an integer, but we haven't
 * switched to that system yet */
typedef int32_t idate_t;
/* iddurs are normally YYYYMMWWDD durations just in an integer, but we
 * haven't switched to that system yet */
typedef int32_t iddur_t;

typedef uint8_t __skipspec_t;


/* skip system */
static int
skipp(__skipspec_t ss, struct dt_d_s dt)
{
	dt_dow_t dow;
	/* common case first */
	if (ss == 0) {
		return 0;
	}
	dow = dt_get_wday(dt);
	/* just check if the bit in the bitset `skip' is set */
	return (ss & (1 << dow)) != 0;
}

#define SKIP_MON	(2)
#define SKIP_TUE	(4)
#define SKIP_WED	(8)
#define SKIP_THU	(16)
#define SKIP_FRI	(32)
#define SKIP_SAT	(64)
#define SKIP_SUN	(1)

static inline int
__toupper(int c)
{
	return c & ~0x20;
}

static dt_dow_t
__parse_wd(const char *str)
{
#define ILEA(a, b)	(((a) << 8) | (b))
	int s1 = __toupper(str[0]);
	int s2 = __toupper(str[1]);

	switch (ILEA(s1, s2)) {
	case ILEA('M', 'O'):
	case ILEA('M', 0):
		/* monday */
		return DT_MONDAY;
	case ILEA('T', 'U'):
		/* tuesday */
		return DT_TUESDAY;
	case ILEA('W', 'E'):
	case ILEA('W', 0):
		/* wednesday */
		return DT_WEDNESDAY;
	case ILEA('T', 'H'):
		/* thursday */
		return DT_THURSDAY;
	case ILEA('F', 'R'):
	case ILEA('F', 0):
		/* friday */
		return DT_FRIDAY;
	case ILEA('S', 'A'):
	case ILEA('A', 0):
		/* saturday */
		return DT_SATURDAY;
	case ILEA('S', 'U'):
	case ILEA('S', 0):
		/* sunday */
		return DT_SUNDAY;
	default:
		return DT_MIRACLEDAY;
	}
}

static __skipspec_t
__skip_dow(__skipspec_t ss, dt_dow_t wd)
{
	switch (wd) {
	case DT_MONDAY:
		/* monday */
		ss |= SKIP_MON;
		break;
	case DT_TUESDAY:
		/* tuesday */
		ss |= SKIP_TUE;
		break;
	case DT_WEDNESDAY:
		/* wednesday */
		ss |= SKIP_WED;
		break;
	case DT_THURSDAY:
		/* thursday */
		ss |= SKIP_THU;
		break;
	case DT_FRIDAY:
		/* friday */
		ss |= SKIP_FRI;
		break;
	case DT_SATURDAY:
		/* saturday */
		ss |= SKIP_SAT;
		break;
	case DT_SUNDAY:
		/* sunday */
		ss |= SKIP_SUN;
		break;
	}
	return ss;
}

static __skipspec_t
__skip_str(__skipspec_t ss, const char *str)
{
	dt_dow_t tmp;

	if ((tmp = __parse_wd(str)) < DT_MIRACLEDAY) {
		ss = __skip_dow(ss, tmp);
	} else {
		int s1 = __toupper(str[0]);
		int s2 = __toupper(str[1]);

		if (ILEA(s1, s2) == ILEA('S', 'S')) {
			/* weekend */
			ss |= SKIP_SAT;
			ss |= SKIP_SUN;
		}
	}
	return ss;
}

static __skipspec_t
__skip_1spec(__skipspec_t ss, char *spec)
{
	char *tmp;
	dt_dow_t from, till;

	if ((tmp = strchr(spec, '-')) == NULL) {
		return __skip_str(ss, spec);
	}
	/* otherwise it's a range */
	*tmp = '\0';
	from = __parse_wd(spec);
	till = __parse_wd(tmp + 1);
	for (int d = from, e = till >= from ? till : till + 7; d <= e; d++) {
		ss = __skip_dow(ss, (dt_dow_t)(d % 7));
	}
	return ss;
}

static __skipspec_t
set_skip(__skipspec_t ss, char *spec)
{
	char *tmp, *tm2;

	if ((tmp = strchr(spec, ',')) == NULL) {
		return __skip_1spec(ss, spec);
	}
	/* const violation */
	*tmp++ = '\0';
	ss = __skip_1spec(ss, spec);
	while ((tmp = strchr(tm2 = tmp, ','))) {
		*tmp++ = '\0';
		ss = __skip_1spec(ss, tm2);
	}
	return __skip_1spec(ss, tm2);
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#endif	/* __INTEL_COMPILER */
#include "dseq-clo.h"
#include "dseq-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
# pragma warning (default:181)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	struct dt_d_s fst, lst;
	iddur_t ite = 1;
	char **ifmt;
	size_t nifmt;
	char *ofmt;
	int res = 0;
	__skipspec_t ss = 0;

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

	for (size_t i = 0; i < argi->skip_given; i++) {
		ss = set_skip(ss, argi->skip_arg[i]);
	}

	switch (argi->inputs_num) {
	default:
		cmdline_parser_print_help();
		res = 1;
		goto out;
	case 1:
		if (!(fst = dt_io_strpd(argi->inputs[0], ifmt, nifmt)).typ) {
			if (!argi->quiet_given) {
				dt_io_warn_strpd(argi->inputs[0]);
			}
			res = 1;
			goto out;
		}
		lst = dt_date(DT_YMD);
		break;
	case 2:
		if (!(fst = dt_io_strpd(argi->inputs[0], ifmt, nifmt)).typ) {
			if (!argi->quiet_given) {
				dt_io_warn_strpd(argi->inputs[0]);
			}
			res = 1;
			goto out;
		}
		if (!(lst = dt_io_strpd(argi->inputs[1], ifmt, nifmt)).typ) {
			if (!argi->quiet_given) {
				dt_io_warn_strpd(argi->inputs[1]);
			}
			res = 1;
			goto out;
		}
		break;
	case 3:
		if (!(fst = dt_io_strpd(argi->inputs[0], ifmt, nifmt)).typ) {
			if (!argi->quiet_given) {
				dt_io_warn_strpd(argi->inputs[0]);
			}
			res = 1;
			goto out;
		}
		unfixup_arg(argi->inputs[1]);
		if ((ite = strtol(argi->inputs[1], NULL, 10)) == 0) {
			if (!argi->quiet_given) {
				fputs("increment must not be naught\n", stderr);
			}
			res = 1;
			goto out;
		}
		if (!(lst = dt_io_strpd(argi->inputs[2], ifmt, nifmt)).typ) {
			if (!argi->quiet_given) {
				dt_io_warn_strpd(argi->inputs[2]);
			}
			res = 1;
			goto out;
		}
		break;
	}

	/* convert to bizdas */
	if ((fst = dt_conv(DT_DAISY, fst)).typ != DT_DAISY ||
	    (lst = dt_conv(DT_DAISY, lst)).typ != DT_DAISY) {
		res = 1;
		fputs("cannot convert calendric system internally\n", stderr);
		goto out;
	}

	if (fst.daisy <= lst.daisy) {
		if (ite < 0) {
			/* different meaning now, we need to compute the
			 * beginning rather than the end */
			struct dt_d_s tmp = lst;

			ite = -ite;
			while (tmp.daisy >= fst.daisy) {
				if (!skipp(ss, tmp)) {
					tmp.daisy -= ite;
				} else {
					tmp.daisy--;
				}
			}
			fst.daisy = tmp.daisy + ite;
		}
		do {
			if (!skipp(ss, fst)) {
				dt_io_write(fst, ofmt);
				fst.daisy += ite;
			} else {
				fst.daisy++;
			}
		} while (fst.daisy <= lst.daisy);
	} else {
		if (ite > 0) {
			/* different meaning now, we need to compute the
			 * end rather than the beginning */
			struct dt_d_s tmp = lst;

			while (tmp.daisy <= fst.daisy) {
				if (!skipp(ss, tmp)) {
					tmp.daisy += ite;
				} else {
					tmp.daisy++;
				}
			}
			fst.daisy = tmp.daisy - ite;
		} else {
			ite = -ite;
		}
		do {
			if (!skipp(ss, fst)) {
				dt_io_write(fst, ofmt);
				fst.daisy -= ite;
			} else {
				fst.daisy--;
			}
		} while (fst.daisy >= lst.daisy);
	}
out:
	cmdline_parser_free(argi);
	return res;
}

/* dseq.c ends here */
