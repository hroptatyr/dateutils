/*** ddiff.c -- perform simple date arithmetic, date minus date
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

#if !defined UNUSED
# define UNUSED(_x)	__attribute__((unused)) _x
#endif	/* !UNUSED */


static dt_dtyp_t
determine_durtype(const char *fmt)
{
	struct {
		unsigned int has_year:1;
		unsigned int has_mon:1;
		unsigned int has_week:1;
		unsigned int has_day:1;
	} flags = {0};

	if (fmt == NULL) {
		return DT_DAISY;
	} else if (strcasecmp(fmt, "ymd") == 0) {
		return DT_YMD;
	} else if (strcasecmp(fmt, "ymcw") == 0) {
		return DT_YMCW;
	} else if (strcasecmp(fmt, "daisy") == 0) {
		return DT_DAISY;
	} else if (strcasecmp(fmt, "bizda") == 0) {
		return DT_BIZDA;
	} else if (strcasecmp(fmt, "bizsi") == 0) {
		return DT_BIZSI;
	}
	/* go through the fmt specs */
	for (const char *fp = fmt; *fp;) {
		const char *fp_sav = fp;
		struct dt_spec_s spec = __tok_spec(fp_sav, (char**)&fp);

		switch (spec.spfl) {
		case DT_SPFL_UNK:
		default:
			/* nothing changes */
			break;
		case DT_SPFL_N_YEAR:
			flags.has_year = 1;
			break;
		case DT_SPFL_N_MON:
		case DT_SPFL_S_MON:
			flags.has_mon = 1;
			break;
		case DT_SPFL_N_MDAY:
		case DT_SPFL_N_STD:
		case DT_SPFL_N_CNT_YEAR:
			flags.has_day = 1;
			break;
		case DT_SPFL_N_CNT_MON:
		case DT_SPFL_N_CNT_WEEK:
		case DT_SPFL_S_WDAY:
			flags.has_week = 1;
			break;
		}
	}
	if (flags.has_week) {
		return DT_YMCW;
	} else if (flags.has_mon || flags.has_year) {
		return DT_YMD;
	} else if (flags.has_day) {
		return DT_DAISY;
	} else {
		return DT_UNK;
	}
}

static int
ddiff_prnt(struct dt_d_s dur, const char *fmt)
{
	char buf[256];
	size_t res = dt_strfddur(buf, sizeof(buf), fmt, dur);

	if (res > 0 && buf[res - 1] != '\n') {
		/* auto-newline */
		buf[res++] = '\n';
	}
	if (res > 0) {
		__io_write(buf, res, stdout);
	}
	return (res > 0) - 1;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#endif	/* __INTEL_COMPILER */
#include "ddiff-clo.h"
#include "ddiff-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
# pragma warning (default:181)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	struct dt_d_s d;
	const char *ofmt;
	char **fmt;
	size_t nfmt;
	int res = 0;
	dt_dtyp_t difftyp;

	if (cmdline_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	}
	/* unescape sequences, maybe */
	if (argi->backslash_escapes_given) {
		dt_io_unescape(argi->format_arg);
	}

	ofmt = argi->format_arg;
	fmt = argi->input_format_arg;
	nfmt = argi->input_format_given;

	if (argi->inputs_num == 0 ||
	    (d = dt_io_strpd(argi->inputs[0], fmt, nfmt)).typ == DT_UNK) {
		fputs("Error: reference DATE must be specified\n\n", stderr);
		cmdline_parser_print_help();
		res = 1;
		goto out;
	}

	/* try and guess the diff tgttype most suitable for user's FMT */
	difftyp = determine_durtype(ofmt);

	if (argi->inputs_num > 1) {
		for (size_t i = 1; i < argi->inputs_num; i++) {
			struct dt_d_s d2;
			struct dt_d_s dur;
			const char *inp = argi->inputs[i];

			if ((d2 = dt_io_strpd(inp, fmt, nfmt)).typ > DT_UNK &&
			    (dur = dt_ddiff(difftyp, d, d2)).typ > DT_DUR_UNK) {
				ddiff_prnt(dur, ofmt);
			} else if (!argi->quiet_given) {
				dt_io_warn_strpd(inp);
			}
		}
	} else {
		/* read from stdin */
		FILE *fp = stdin;
		char *line;
		size_t lno = 0;

		/* no threads reading this stream */
		__io_setlocking_bycaller(fp);
		__io_setlocking_bycaller(stdout);

		for (line = NULL; !__io_eof_p(fp); lno++) {
			ssize_t n;
			size_t len;
			struct dt_d_s d2;
			struct dt_d_s dur;

			n = getline(&line, &len, fp);
			if (n < 0) {
				break;
			}
			/* terminate the string accordingly */
			line[n - 1] = '\0';
			/* perform addition now */
			if ((d2 = dt_io_strpd(line, fmt, nfmt)).typ > DT_UNK &&
			    (dur = dt_ddiff(difftyp, d, d2)).typ > DT_DUR_UNK) {
				ddiff_prnt(dur, ofmt);
			} else if (!argi->quiet_given) {
				dt_io_warn_strpd(line);
			}
		}
		/* get rid of resources */
		free(line);
		goto out;
	}

out:
	cmdline_parser_free(argi);
	return res;
}

/* ddiff.c ends here */
