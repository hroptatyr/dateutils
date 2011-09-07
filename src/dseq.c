/*** dseq.c -- like seq(1) but for dates
 *
 * Copyright (C) 2009 - 2011 Sebastian Freundt
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
 ***/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

/* idates are normally just YYYYMMDD just in an integer, but we haven't
 * switched to that system yet */
typedef uint32_t idate_t;
/* iddurs are normally YYYYMMWWDD durations just in an integer, but we
 * haven't switched to that system yet */
typedef uint32_t iddur_t;

typedef uint8_t __skipspec_t;

typedef enum {
	DOW_SUNDAY,
	DOW_MONDAY,
	DOW_TUESDAY,
	DOW_WEDNESDAY,
	DOW_THURSDAY,
	DOW_FRIDAY,
	DOW_SATURDAY,
	DOW_MIRACLEDAY
} __dow_t;


static void
pr_ts(const char *fmt, idate_t dt)
{
	struct tm tm;
	static char b[32];
	time_t ts = dt * 86400;
	size_t len;

	memset(&tm, 0, sizeof(tm));
	(void)gmtime_r(&ts, &tm);
	len = strftime(b, sizeof(b), fmt, &tm);
	b[len++] = '\n';
	fwrite(b, sizeof(*b), len, stdout);
	return;
}

static idate_t
sc_ts(const char *s, const char *fmt)
{
	struct tm tm;

	/* basic sanity check */
	if (s == NULL) {
		return time(NULL) / 86400;
	}
	/* wipe tm */
	memset(&tm, 0, sizeof(tm));
	(void)strptime(s, fmt, &tm);
	return mktime(&tm) / 86400;
}

static inline __dow_t
__dayofweek(idate_t dt)
{
	/* we know that 15/01/1984 was a sunday, and this is 442972800, */
	const time_t anchor = 442972800 / 86400;
	/* fucking intel compiler insists on negative values for % so
	 * branch here */
	if (dt > anchor) {
		dt = dt - anchor;
	} else {
		dt = anchor - dt;
	}
	return (__dow_t)(dt % 7);
}


/* skip system */
static int
skipp(__skipspec_t ss, idate_t dt)
{
	__dow_t dow;
	/* common case first */
	if (ss == 0) {
		return 0;
	}
	dow = __dayofweek(dt);
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
__tolower(int c)
{
	return c | 0x20;
}

static __skipspec_t
__set_skip(__skipspec_t ss, const char *str)
{
/* unrolled trie */
#define ILEA(a, b)	(((a) << 8) | (b))
	int s1 = __tolower(str[0]);
	int s2 = __tolower(str[1]);

	switch (ILEA(s1, s2)) {
	case ILEA('m', 'o'):
	case ILEA('m', 0):
		/* monday */
		ss |= SKIP_MON;
		break;
	case ILEA('t', 'u'):
		/* tuesday */
		ss |= SKIP_TUE;
		break;
	case ILEA('w', 'e'):
		/* wednesday */
		ss |= SKIP_WED;
		break;
	case ILEA('t', 'h'):
		/* thursday */
		ss |= SKIP_THU;
		break;
	case ILEA('f', 'r'):
	case ILEA('f', 0):
		/* friday */
		ss |= SKIP_FRI;
		break;
	case ILEA('s', 'a'):
	case ILEA('a', 0):
		/* saturday */
		ss |= SKIP_SAT;
		break;
	case ILEA('s', 'u'):
		/* sunday */
		ss |= SKIP_SUN;
		break;
	case ILEA('s', 's'):
		/* weekend */
		ss |= SKIP_SAT;
		ss |= SKIP_SUN;
		break;
	default:
		break;
	}
	return ss;
}

static __skipspec_t
set_skip(__skipspec_t ss, const char *spec)
{
	char *tmp;

	if ((tmp = strchr(spec, ',')) == NULL) {
		return __set_skip(ss, spec);
	}
	/* const violation */
	*tmp++ = '\0';
	ss = __set_skip(ss, spec);
	for (char *tm2; (tmp = strchr(tm2 = tmp, ',')); tmp++) {
		*tmp = '\0';
		ss = __set_skip(ss, tm2);
	}
	ss = __set_skip(ss, tmp);
	return ss;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
#endif	/* __INTEL_COMPILER */
#include "dseq-clo.h"
#include "dseq-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	static const char dflt_fmt[] = "%F";
	idate_t fst, lst;
	iddur_t ite = 1;
	const char *ifmt = dflt_fmt;
	const char *ofmt = dflt_fmt;
	int res = 0;
	__skipspec_t ss = 0;

	if (cmdline_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	}
	if (argi->format_given) {
		ofmt = argi->format_arg;
	}
	if (argi->input_format_given) {
		ifmt = argi->input_format_arg;
	}
	for (size_t i = 0; i < argi->skip_given; i++) {
		ss = set_skip(ss, argi->skip_arg[i]);
	}

	switch (argi->inputs_num) {
	default:
		cmdline_parser_print_help();
		res = 1;
		goto out;
	case 1:
		fst = sc_ts(argi->inputs[0], ifmt);
		lst = sc_ts(NULL, ifmt);
		break;
	case 2:
		fst = sc_ts(argi->inputs[0], ifmt);
		lst = sc_ts(argi->inputs[1], ifmt);
		break;
	case 3:
		fst = sc_ts(argi->inputs[0], ifmt);
		ite = strtol(argi->inputs[1], NULL, 10);
		lst = sc_ts(argi->inputs[2], ifmt);
		break;
	}

	while (fst <= lst) {
		if (!skipp(ss, fst)) {
			pr_ts(ofmt, fst);
		}
		fst += ite;
	}
out:
	cmdline_parser_free(argi);
	return res;
}

/* dseq.c ends here */
