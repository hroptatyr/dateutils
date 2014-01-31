/*** tzmap.c -- zonename maps
 *
 * Copyright (C) 2014 Sebastian Freundt
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
#include <stddef.h>
#if defined HAVE_SYS_STDINT_H
# include <sys/stdint.h>
#endif	/* HAVE_SYS_STDINT_H */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include "tzmap.h"
#include "boops.h"

#if !defined LIKELY
# define LIKELY(_x)     __builtin_expect((_x), 1)
#endif  /* !LIKELY */
#if !defined UNLIKELY
# define UNLIKELY(_x)   __builtin_expect((_x), 0)
#endif  /* UNLIKELY */
#if !defined UNUSED
# define UNUSED(_x)     _x __attribute__((unused))
#endif  /* !UNUSED */

#if !defined countof
# define countof(x)     (sizeof(x) / sizeof(*x))
#endif  /* !countof */

#define _paste(x, y)    x ## y
#define paste(x, y)     _paste(x, y)
#if !defined with
# define with(args...)                                                  \
        for (args, *paste(__ep, __LINE__) = (void*)1;                   \
             paste(__ep, __LINE__); paste(__ep, __LINE__) = 0)
#endif  /* !with */

#if !defined HAVE_GETLINE && !defined HAVE_FGETLN
/* as a service to people including this file in their project
 * but who might not necessarily run the corresponding AC_CHECK_FUNS
 * we assume that a getline() is available. */
# define HAVE_GETLINE   1
#endif  /* !HAVE_GETLINE && !HAVE_FGETLN */


static __attribute__((format(printf, 1, 2))) void
error(const char *fmt, ...)
{
        va_list vap;
        va_start(vap, fmt);
        vfprintf(stderr, fmt, vap);
        va_end(vap);
        if (errno) {
                fputc(':', stderr);
                fputc(' ', stderr);
                fputs(strerror(errno), stderr);
        }
        fputc('\n', stderr);
        return;
}


/* array of all zone names */
static char *zns;
static size_t znz;
static ptrdiff_t zni;
/* array for all mappee strings */
static znoff_t *mns;
static size_t mnz;
static ptrdiff_t mni;

static void
init_tzm(void)
{
	zns = calloc(znz = 64U, sizeof(*zns));
	mns = calloc(mnz = 64U, sizeof(*mns));
	return;
}

static void
free_tzm(void)
{
	free(zns);
	free(mns);
	return;
}

static znoff_t
tzm_find_zn(const char *zn, size_t zz)
{
	char *restrict p = zns;
	const char *const ep = zns + znz;

	for (; p < ep && *p && strcmp(p, zn); p += strlen(p), p++);
	if (*p) {
		/* found it, yay */
		return p - zns;
	}
	/* otherwise append, first check if there's room */
	if (p + zz + 4U >= ep) {
		/* compute new p */
		ptrdiff_t d = p - zns;
		/* resize, double the size */
		p = (zns = realloc(zns, znz *= 2U)) + d;
		memset(p, 0, (znz - (p - zns)) * sizeof(*zns));
	}
	/* really append now */
	memcpy(p, zn, zz);
	zni = (p - zns) + zz + 1U;
	return p - zns;
}

static void
tzm_add_mn(const char *mn, size_t mz, znoff_t off)
{
	znoff_t *restrict p = mns + mni;

	if (UNLIKELY(mz == 0U)) {
		/* useless */
		return;
	}
	/* first check if there's room */
	if (p + 1U + (mz + 4U/*alignment*/) / sizeof(off) >= mns + mnz) {
		/* resize, double the size */
		p = (mns = realloc(mns, (mnz *= 2U) * sizeof(*mns))) + mni;
		memset((char*)p, 0, (mnz - (p - mns)) * sizeof(*mns));
	}
	/* really append now */
	memcpy(p, mn, mz);
	p += (mz - 1U) / sizeof(off) + 1U;
	*p++ = htobe32((off & 0xffffU)<< 8U);
	mni = p - mns;
	return;
}


static void
parse_line(char *ln, size_t lz)
{
	/* find the separator */
	char *lp;
	znoff_t znp;

	if (UNLIKELY(ln == NULL || lz == 0U)) {
		/* finalise */
		return;
	}
	if ((lp = strchr(ln, '\t')) == NULL) {
		/* buggered line */
		return;
	} else if (*lp++ = '\0', *lp == '\0') {
		/* huh? no zone name, cunt off */
		return;
	} else if ((znp = tzm_find_zn(lp, ln + lz - lp)) == -1U) {
		/* brilliant, can't add anything */
		return;
	}
	tzm_add_mn(ln, lp - ln - 1U, znp);
	return;
}

static int
parse_file(const char *file)
{
	char *line = NULL;
	size_t llen = 0U;
	FILE *fp;

	if (file == NULL) {
		fp = stdin;
	} else if ((fp = fopen(file, "r")) == NULL) {
		return -1;
	}

#if defined HAVE_GETLINE
	for (ssize_t nrd; (nrd = getline(&line, &llen, fp)) > 0;) {
		line[--nrd] = '\0';
		parse_line(line, nrd);
	}
#elif defined HAVE_FGETLN
	while ((line = fgetln(f, &llen)) != NULL) {
		line[--llen] = '\0';
		parse_line(line, llen);
	}
#else
# error neither getline() nor fgetln() available, cannot read file line by line
#endif	/* GETLINE/FGETLN */
	return 0;
}


#include "tzmap.yucc"

static int
cmd_cc(struct yuck_cmd_cc_s argi[static 1U])
{
	int rc = 0;

	/* reserver some space */
	init_tzm();

	if (parse_file(argi->args[0U]) < 0) {
		error("cannot read file `%s'", *argi->args ?: "stdin");
		rc = 1;
		goto out;
	}

	/* generate a disk version now */
	with (int ofd = open("schnabel", O_RDWR | O_CREAT | O_TRUNC, 0666)) {
		znoff_t off = zni;

		if (ofd < 0) {
			break;
		}

		write(ofd, TZM_MAGIC, sizeof(TZM_MAGIC) - 1U);
		off = (off + sizeof(off) - 1U) / sizeof(off) * sizeof(off);
		with (znoff_t off_be = htobe32(off)) {
			write(ofd, &off_be, sizeof(off_be));
		}
		write(ofd, zns, off);
		write(ofd, mns, mni * sizeof(*mns));
		close(ofd);
	}

out:
	free_tzm();
	return rc;
}

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	int rc = 0;

	if (yuck_parse(argi, argc, argv) < 0) {
		rc = 1;
		goto out;
	}

	switch (argi->cmd) {
	case TZMAP_CMD_CC:
		rc = cmd_cc((void*)argi);
		break;
	default:
		rc = 1;
		break;
	}

out:
	yuck_free(argi);
	return rc;
}

/* tzmap.c ends here */
