/*** ltrcc.c -- leapseconds materialiser
 *
 * Copyright (C) 2012-2022 Sebastian Freundt
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
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#if defined HAVE_SYS_STDINT_H
# include <sys/stdint.h>
#endif	/* HAVE_SYS_STDINT_H */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "leaps.h"
#include "date-core.h"
#include "time-core.h"
#include "nifty.h"

#include "version.c"


static __attribute__((const)) unsigned long int
ntp_to_unix_epoch(unsigned long int x)
{
	return x - 25567U * 86400U;
}


#define PROLOGUE	(-1UL)
#define EPILOGUE	(0UL)

static int
pr_line_corr(const char *line, size_t llen, va_list UNUSED(vap))
{
	static unsigned long int cor;
	char *sp, *ep;

	if (llen == PROLOGUE) {
		/* prologue */
		fprintf(stdout, "\
const int32_t %s[] = {\n\
	10,\n", line);
		return 0;
	} else if (llen == EPILOGUE) {
		fprintf(stdout, "\
	%ld\n\
};\n", cor);
		cor = 0;
		return 0;
	} else if (line == NULL) {
		/* grrrr */
		return -1;
	} else if (line[0] == '#') {
		/* comment line */
		return 0;
	} else if (line[0] == '\n') {
		/* empty line */
		return 0;
	}
	/* otherwise process */
	if ((sp = memchr(line, '\t', llen)) == NULL) {
		return -1;
	} else if ((ep = NULL, cor = strtoul(++sp, &ep, 10), ep == NULL || cor == ULONG_MAX)) {
		return -1;
	}

	/* output the correction then */
	fprintf(stdout, "\t%ld,\n", cor);
	return 0;
}

static int
pr_line_d(const char *line, size_t llen, va_list vap)
{
	static unsigned long int cor;
	struct dt_d_s d;
	dt_dtyp_t typ;
	unsigned long int val;
	int colp;
	char *ep;

	/* extract type from inner list */
	typ = va_arg(vap, dt_dtyp_t);
	colp = va_arg(vap, int);

	if (llen == PROLOGUE) {
		/* prologue */
		if (!colp) {
			fprintf(stdout, "\
const struct zleap_s %s[] = {\n\
	{0x00U/* 0 */, 0},\n", line);
		} else {
			fprintf(stdout, "\
const uint32_t %s[] = {\n\
	0x00U/* 0 */,\n", line);
		}
		return 0;
	} else if (llen == EPILOGUE) {
		/* epilogue */
		if (!colp) {
			fprintf(stdout, "\
	{UINT32_MAX, %li}\n\
};\n", cor);
		} else {
			fputs("\
	UINT32_MAX\n\
};\n", stdout);
		}
		cor = 0;
		return 0;
	} else if (line == NULL) {
		/* something's fucked */
		return -1;
	} else if (line[0] == '#') {
		/* comment line */
		return 0;
	} else if (line[0] == '\n') {
		/* empty line */
		return 0;
	}
	/* otherwise process */
	if ((ep = NULL, val = strtoul(line, &ep, 10), ep == NULL || val == ULONG_MAX)) {
		return -1;
	}

	/* fix up and convert to target type */
	d = (struct dt_d_s){DT_DAISY, .daisy = val / 86400 + 109207};
	d = dt_dconv(typ, d);

	if (!colp) {
		if ((cor = strtoul(ep, &ep, 10), ep == NULL || val == ULONG_MAX)) {
			return -1;
		}
		/* just output the line then */
		fprintf(stdout, "\t{0x%xU/* %i */, %li},\n",
			d.u, (int32_t)d.u, cor);
	} else {
		fprintf(stdout, "\t0x%xU/* %i */,\n", d.u, (int32_t)d.u);
	}
	return 0;
}

static int
pr_line_dt(const char *line, size_t llen, va_list vap)
{
	static unsigned long int cor;
	dt_dtyp_t __attribute__((unused)) typ;
	unsigned long int val;
	int colp;
	char *ep;

	/* extract type from inner list */
	typ = va_arg(vap, dt_dtyp_t);
	colp = va_arg(vap, int);

	if (llen == PROLOGUE) {
		/* prologue */
		if (!colp) {
			fprintf(stdout, "\
const struct zleap_s %s[] = {\n\
	{INT32_MIN, 10},\n", line);
		} else {
			fprintf(stdout, "\
const int32_t %s[] = {\n\
	INT32_MIN,\n", line);
		}
		return 0;
	} else if (llen == EPILOGUE) {
		/* epilogue */
		if (!colp) {
			fprintf(stdout, "\
	{INT32_MAX, %li}\n\
};\n", cor);
		} else {
			fputs("\
	INT32_MAX\n\
};\n", stdout);
		}
		cor = 0;
		return 0;
	} else if (line == NULL) {
		/* buggre */
		return -1;
	} else if (line[0] == '#') {
		/* comment line */
		return 0;
	} else if (line[0] == '\n') {
		/* empty line */
		return 0;
	}
	/* otherwise process */
	if ((ep = NULL, val = strtoul(line, &ep, 10), ep == NULL || val == ULONG_MAX)) {
		return -1;
	}

	/* fix up and convert to target type */
	val--;
	val = ntp_to_unix_epoch(val);

	if (!colp) {
		if ((cor = strtoul(ep, &ep, 10), ep == NULL || cor == ULONG_MAX)) {
			return -1;
		}
		/* just output the line then */
		fprintf(stdout, "\t{0x%lxU/* %li */, %li},\n",
			val, val, cor);
	} else {
		/* column-oriented mode */
		fprintf(stdout, "\t0x%lxU/* %li */,\n", val, val);
	}
	return 0;
}

static int
pr_line_t(const char *line, size_t llen, va_list vap)
{
	static unsigned long int cor;
	struct dt_t_s t = {DT_TUNK};
	dt_dtyp_t typ;
	int colp;
	char *ep;
	unsigned long int val;

	/* extract type from inner list */
	typ = va_arg(vap, dt_dtyp_t);
	colp = va_arg(vap, int);

	/* column-oriented mode only */
	if (!colp) {
		return 0;
	}

	if (llen == PROLOGUE) {
		/* prologue */
		fprintf(stdout, "\
const uint32_t %s[] = {\n\
	UINT32_MAX,\n", line);
		return 0;
	} else if (llen == EPILOGUE) {
		/* epilogue */
		fputs("\
	UINT32_MAX\n\
};\n", stdout);
		cor = 0;
		return 0;
	} else if (typ != (dt_dtyp_t)DT_HMS || !colp) {
		return 0;
	} else if (line == NULL) {
		/* do fuckall */
		return -1;
	} else if (line[0] == '#') {
		/* comment line */
		return 0;
	} else if (line[0] == '\n') {
		/* empty line */
		return 0;
	}
	/* otherwise process */
	if ((ep = NULL, val = strtoul(line, &ep, 10), ep == NULL || val == ULONG_MAX)) {
		return -1;
	}
	val--;
	t.hms.s = val % 60L;
	val /= 60L;
	t.hms.m = val % 60L;
	val /= 60L;
	t.hms.h = val % 24L;

	/* read correction */
	if ((val = strtoul(ep, &ep, 10), ep == NULL || val == ULONG_MAX)) {
		return -1;
	}

	/* fix up and convert to target type */
	with (uint32_t ual = t.hms.u24) {
		ual += val >= cor;
		fprintf(stdout, "\t0x%xU/* %u */,\n", ual, ual);
		cor = val;
	}
	return 0;
}

static int
pr_file(FILE *fp, const char *var, int(*cb)(const char*, size_t, va_list), ...)
{
	va_list vap;
	char *line = NULL;
	size_t len = 0;
	ssize_t nrd;

	/* prologue */
	va_start(vap, cb);
	cb(var, PROLOGUE, vap);
	va_end(vap);
	/* main loop */
	while ((nrd = getline(&line, &len, fp)) >= 0) {
		va_start(vap, cb);
		if (cb(line, nrd, vap) < 0) {
			fprintf(stderr, "line buggered: %s", line);
		}
		va_end(vap);
	}
	/* epilogue */
	va_start(vap, cb);
	cb(var, EPILOGUE, vap);
	va_end(vap);
	/* standard epilogue */
	fprintf(stdout, "\
const size_t n%s = countof(%s);\n\n", var, var);

	if (line != NULL) {
		free(line);
	}
	return 0;
}


static int col = 0;

static int
parse_file(const char *file)
{
	FILE *fp;

	if ((fp = fopen(file, "r")) == NULL) {
		return -1;
	}

	fprintf(stdout, "\
/*** autogenerated by: ltrcc %s */\n\
\n\
#include <stdint.h>\n\
#include <limits.h>\n\
#include \"leaps.h\"\n\
#include \"leap-seconds.h\"\n\
\n\
#if !defined INCLUDED_ltrcc_generated_def_\n\
#define INCLUDED_ltrcc_generated_def_\n\
\n\
#if !defined countof\n\
# define countof(x)	(sizeof(x) / sizeof(*x))\n\
#endif	/* !countof */\n\
\n", file);

	if (col) {
		pr_file(fp, "leaps_corr", pr_line_corr);
		rewind(fp);
	}

	pr_file(fp, "leaps_ymd", pr_line_d, DT_YMD, col);
	rewind(fp);
	pr_file(fp, "leaps_ymcw", pr_line_d, DT_YMCW, col);
	rewind(fp);
	pr_file(fp, "leaps_d", pr_line_d, DT_DAISY, col);
	rewind(fp);
	pr_file(fp, "leaps_s", pr_line_dt, DT_YMD, col);
	rewind(fp);
	pr_file(fp, "leaps_hms", pr_line_t, DT_HMS, col);

	fputs("\
/* exported number of leap transitions */\n\
const size_t nleaps = countof(leaps_corr);\n\
\n\
#endif  /* INCLUDED_ltrcc_generated_def_ */\n", stdout);
	return 0;
}


#include "ltrcc.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	int rc = 0;

	if (yuck_parse(argi, argc, argv) < 0) {
		rc = 1;
		goto out;
	} else if (!argi->nargs) {
		fputs("LEAP-SECONDSS.LIST argument is mandatory\n", stderr);
		rc = 1;
		goto out;
	}

	/* assign params */
	col = argi->column_oriented_flag;

	if (parse_file(argi->args[0U]) < 0) {
		perror("Cannot parse file");
		rc = 1;
	}

out:
	yuck_free(argi);
	return rc;
}

/* ltrcc.c ends here */
