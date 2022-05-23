/*** tzmap.c -- zonename maps
 *
 * Copyright (C) 2014-2022 Sebastian Freundt
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
/* for fgetln() */
#define _NETBSD_SOURCE
#define _DARWIN_SOURCE
#define _ALL_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#if defined HAVE_SYS_STDINT_H
# include <sys/stdint.h>
#endif	/* HAVE_SYS_STDINT_H */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
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

#if !defined DEFUN
# define DEFUN
#endif	/* !DEFUN */

#if defined STANDALONE
# if defined TZDIR
static const char tzdir[] = TZDIR;
# else  /* !TZDIR */
static const char tzdir[] = "/usr/share/zoneinfo";
# endif	/* TZDIR */
#endif	/* STANDALONE */


#if defined STANDALONE
static __attribute__((format(printf, 1, 2))) void
error(const char *fmt, ...)
{
        va_list vap;
        va_start(vap, fmt);
        vfprintf(stderr, fmt, vap);
        va_end(vap);
        fputc('\n', stderr);
        return;
}

static __attribute__((format(printf, 1, 2))) void
serror(const char *fmt, ...)
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

static size_t
xstrlncpy(char *restrict dst, size_t dsz, const char *src, size_t ssz)
{
	if (ssz >= dsz) {
		ssz = dsz - 1U;
	}
	memcpy(dst, src, ssz);
	dst[ssz] = '\0';
	return ssz;
}
#endif	/* STANDALONE */

static const void*
align_to(size_t tz, const void *p)
{
	uintptr_t x = (uintptr_t)p;

	if (x % tz) {
		x -= x % tz;
	}
	return (const void*)x;
}
#define ALIGN_TO(tz, p)	align_to(sizeof(tz), p)


/* public API */
static inline size_t
tzm_file_size(tzmap_t m)
{
	return m->flags[1U];
}

static inline int
tzm_fd(tzmap_t m)
{
	return m->flags[0U];
}

static inline const char*
tzm_znames(tzmap_t m)
{
	return m->data;
}

static inline size_t
tzm_zname_size(tzmap_t m)
{
	return m->off;
}

static inline const char*
tzm_mnames(tzmap_t m)
{
	return m->data + tzm_zname_size(m);
}

static inline size_t
tzm_mname_size(tzmap_t m)
{
	size_t fz = tzm_file_size(m);
	return fz - tzm_zname_size(m) - sizeof(*m);
}

DEFUN tzmap_t
tzm_open(const char *fn)
{
#define FAIL	(tzmap_t)MAP_FAILED
#define TZMP	(PROT_READ | PROT_WRITE)
	struct stat st[1U];
	size_t fz;
	struct tzmap_s *m;
	int fd;

	if ((fd = open(fn, O_RDONLY)) < 0) {
		return NULL;
	} else if (fstat(fd, st) < 0) {
		goto clo;
	} else if ((fz = st->st_size) < sizeof(*m)) {
		goto clo;
	} else if ((m = mmap(0, fz, TZMP, MAP_PRIVATE, fd, 0)) == FAIL) {
		goto clo;
	} else if (memcmp(m->magic, TZM_MAGIC, sizeof(m->magic))) {
		goto mun;
	}
	/* turn offset into native endianness */
	m->off = be32toh(m->off);
	/* also put fd and map size into m */
	m->flags[0U] = (znoff_t)fd;
	m->flags[1U] = (znoff_t)st->st_size;
	/* and here we go */
	return m;

#undef FAIL
#undef TZMP
	/* failure cases, clean up and return NULL */
mun:
	munmap(m, st->st_size);
clo:
	close(fd);
	return NULL;
}

DEFUN void
tzm_close(tzmap_t m)
{
	size_t fz = tzm_file_size(m);
	int fd = tzm_fd(m);

	/* hopefully privately mapped */
	munmap(m, fz);
	close(fd);
	return;
}

DEFUN const char*
tzm_find(tzmap_t m, const char *mname)
{
/* lookup zname for MNAME */
	const znoff_t *sp = (const void*)tzm_mnames(m);
	const znoff_t *ep = sp + tzm_mname_size(m) / sizeof(*sp) - 1U;
	const char *zns = tzm_znames(m);

	/* do a bisection now */
	do {
		const char *mp = mname;
		const char *tp;
		const char *p;

		tp = (const char*)(sp + (ep - sp) / 2U);
		if (!*tp) {
			/* fast forward to the next entry */
			tp += sizeof(*sp);
		} else {
			while (tp[-1] != '\0') {
				/* rewind to beginning */
				tp--;
			}
		}
		/* store tp again */
		p = tp;
		/* now unroll a strcmp */
		for (; *mp && *mp == *tp; mp++, tp++);
		if (*mp - *tp < 0) {
			/* use lower half */
			ep = (const znoff_t*)p - 1U;
		} else {
			/* forward to the next znoff_t alignment */
			const znoff_t *op =
				(const znoff_t*)ALIGN_TO(znoff_t, tp - 1U) + 1U;

			if (*mp - *tp > 0) {
				/* use upper half */
				sp = op + 1U;
			} else {
				/* found it */
				return zns + (be32toh(*op) >> 8U);
			}
		}
	} while (sp < ep);
	return NULL;
}


#if defined STANDALONE
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

	for (; p < ep && *p && strncmp(p, zn, zz); p += strlen(p), p++);
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


static unsigned int exst_only_p;
static const char *check_fn;

static bool
tzdir_zone_p(const char *zn, size_t zz)
{
	struct stat st[1U];
	char fullzn[256U] = {0};

	if (*zn == '/') {
		/* absolute zonename? */
		;
	} else if (*zn) {
		/* relative zonename */
		char *zp = fullzn;
		const char *const ep = fullzn + sizeof(fullzn);

		zp += xstrlncpy(zp, ep - zp, tzdir, sizeof(tzdir) - 1U);
		*zp++ = '/';
		zp += xstrlncpy(zp, ep - zp, zn, zz);
		*zp = '\0';
	}
	/* finally the actual check */
	if (stat(fullzn, st) < 0) {
		return false;
	}
	return true;
}

static znoff_t
parse_line(char *ln, size_t lz)
{
	/* find the separator */
	char *lp;
	znoff_t znp;

	if (UNLIKELY(ln == NULL || lz == 0U)) {
		/* finalise */
		return NUL_ZNOFF;
	}
	if ((lp = memchr(ln, '\t', lz)) == NULL) {
		/* buggered line */
		return NUL_ZNOFF;
	} else if (lp == ln) {
		return NUL_ZNOFF;
	} else if (*lp++ = '\0', *lp == '\0') {
		/* huh? no zone name, cunt off */
		return NUL_ZNOFF;
	} else if (lp - ln > 256) {
		/* too long */
		return NUL_ZNOFF;
	} else if (exst_only_p && !tzdir_zone_p(lp, ln + lz - lp)) {
		error("\
Warning: zone `%.*s' skipped: not present in global zone database",
		      (int)(ln + lz - lp), lp);
		return NUL_ZNOFF;
	} else if ((znp = tzm_find_zn(lp, ln + lz - lp)) == -1U) {
		/* brilliant, can't add anything */
		return NUL_ZNOFF;
	}
	tzm_add_mn(ln, lp - ln - 1U, znp);
	return znp;
}

static int
check_line(char *ln, size_t lz)
{
	static char last[256U];
	static unsigned int lno;
	size_t cz;
	char *lp;
	int rc = 0;

	if (UNLIKELY(ln == NULL || lz == 0U)) {
		/* finalise */
		lno = 0U;
		*last = '\0';
		return 0;
	}
	/* advance line number */
	lno++;

#define CHECK_ERROR(fmt, args...)		\
	error("Error in %s:%u: " fmt, check_fn, lno, ## args)

	/* the actual checks here */
	if ((lp = memchr(ln, '\t', lz)) == NULL) {
		/* buggered line */
		CHECK_ERROR("no separator");
		return -1;
	} else if (lp == ln) {
		CHECK_ERROR("no code");
		return -1;
	} else if (*lp++ = '\0', *lp == '\0') {
		/* huh? no zone name, cunt off */
		CHECK_ERROR("no zone name");
		rc = -1;
	}
	if ((cz = lp - ln - 1U) >= sizeof(last)) {
		CHECK_ERROR("code too long (%zu chars, max is 255)", cz);
		rc = -1;
	} else if (strncmp(last, ln, cz) >= 0) {
		CHECK_ERROR("non-ascending order `%s' (after `%s')", ln, last);
		rc = -1;
	}
	/* make sure to memorise ln for the next run */
	xstrlncpy(last, sizeof(last), ln, cz);

	/* it's none of our business really, but go through the zonenames
	 * and check for their existence now */
	if (!*lp) {
		/* already warned about this */
		;
	} else if (!tzdir_zone_p(lp, lz - (lp - ln))) {
		lp[lz - (lp - ln)] = '\0';
		CHECK_ERROR("cannot find zone `%s' in TZDIR", lp);
		rc = -1;
	}
#undef CHECK_ERROR
	return rc;
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
		parse_line(line, nrd - 1);
	}
#elif defined HAVE_FGETLN
	while ((line = fgetln(fp, &llen)) != NULL && llen > 0U) {
		parse_line(line, llen - 1);
	}
#else
# error neither getline() nor fgetln() available, cannot read file line by line
#endif	/* GETLINE/FGETLN */

#if defined HAVE_GETLINE
	/* free line buffer resources */
	free(line);
#endif	/* HAVE_GETLINE */

	fclose(fp);
	return 0;
}

static bool
tzmccp(FILE *fp)
{
	static char buf[4U];

	if (fread(buf, sizeof(*buf), countof(buf), fp) < sizeof(buf)) {
		/* definitely buggered */
		;
	} else if (!memcmp(buf, TZM_MAGIC, sizeof(buf))) {
		return true;
	}
	/* otherwise, good try, seek back to the beginning */
	fseek(fp, 0, SEEK_SET);
	return false;
}

static int
tzm_check(const char *fn)
{
	tzmap_t m;
	int rc = 0;

	if ((m = tzm_open(fn)) == NULL) {
		serror("cannot open input file `%s'", fn);
		return -1;
	}

	{
		/* traverse them all */
		const znoff_t *p = (const void*)tzm_mnames(m);
		const znoff_t *const ep =
			(const void*)((const char*)p + tzm_mname_size(m));

		while (p < ep) {
			const char *mn = (const void*)p;
			size_t mz = strlen(mn);
			znoff_t off;
			const char *zn;
			size_t zz;

			p += (mz - 1U) / sizeof(*p) + 1U;
			off = be32toh(*p++) >> 8U;

			zn = m->data + off;
			zz = strlen(zn);
			if (!tzdir_zone_p(zn, zz)) {
				error("cannot find zone `%s' in TZDIR", zn);
				rc = -1;
			}
		}
	}

	tzm_close(m);
	return rc;
}

static int
check_file(const char *file)
{
	char *line = NULL;
	size_t llen = 0U;
	FILE *fp;
	int rc = 0;

	if (file == NULL) {
		fp = stdin;
		check_fn = "-";
	} else if ((fp = fopen(check_fn = file, "r")) == NULL) {
		serror("Cannot open file `%s'", file);
		return -1;
	} else if (tzmccp(fp)) {
		/* oh yikes, can't use the line reader can we */
		fclose(fp);
		return tzm_check(file);
	}

#if defined HAVE_GETLINE
	for (ssize_t nrd; (nrd = getline(&line, &llen, fp)) > 0;) {
		rc |= check_line(line, nrd - 1);
	}
#elif defined HAVE_FGETLN
	while ((line = fgetln(fp, &llen)) != NULL && llen > 0U) {
		rc |= check_line(line, llen - 1);
	}
#else
# error neither getline() nor fgetln() available, cannot read file line by line
#endif	/* GETLINE/FGETLN */

#if defined HAVE_GETLINE
	/* free line buffer resources */
	free(line);
#endif	/* HAVE_GETLINE */

	/* reset line checker */
	check_line(NULL, 0U);
	/* and clear resources */
	fclose(fp);
	return rc;
}
#endif	/* STANDALONE */


#if defined STANDALONE
#include "tzmap.yucc"

static int
cmd_cc(const struct yuck_cmd_cc_s argi[static 1U])
{
	const char *outf;
	int rc = 0;
	int ofd;

	/* reserve some space */
	init_tzm();
	/* establish environment */
	exst_only_p = argi->existing_only_flag;

	if (parse_file(argi->args[0U]) < 0) {
		error("cannot read file `%s'", *argi->args ?: "stdin");
		rc = 1;
		goto out;
	} else if ((outf = argi->output_arg ?: "tzcc.tzm", false)) {
		/* we used to make -o|--output mandatory */
		;
	} else if ((ofd = open(outf, O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0) {
		serror("cannot open output file `%s'", outf);
		rc = 1;
		goto out;
	}

	/* generate a disk version now */
	with (znoff_t off = zni) {
		static struct tzmap_s r = {TZM_MAGIC};
		ssize_t sz;

		off = (off + sizeof(off) - 1U) / sizeof(off) * sizeof(off);
		r.off = htobe32(off);
		if (sz = sizeof(r), write(ofd, &r, sz) < sz) {
			goto trunc;
		} else if (sz = off, write(ofd, zns, sz) < sz) {
			goto trunc;
		} else if (sz = mni * sizeof(*mns), write(ofd, mns, sz) < sz) {
			goto trunc;
		}
		close(ofd);
		break;

	trunc:
		/* some write failed, leave a 0 byte file around */
		close(ofd);
		unlink(outf);
		rc = 1;
	}

out:
	free_tzm();
	return rc;
}

static int
cmd_show(const struct yuck_cmd_show_s argi[static 1U])
{
	const char *fn;
	tzmap_t m;
	int rc = 0;

	if ((fn = argi->tzmap_arg ?: "tzcc.tzm", false)) {
		/* we used to make -f|--tzmap mandatory */
		;
	} else if ((m = tzm_open(fn)) == NULL) {
		serror("cannot open input file `%s'", fn);
		return 1;
	}

	if (!argi->nargs) {
		/* dump mode */
		const znoff_t *p = (const void*)tzm_mnames(m);
		const znoff_t *const ep =
			(const void*)((const char*)p + tzm_mname_size(m));

		while (p < ep) {
			const char *mn = (const void*)p;
			size_t mz = strlen(mn);
			znoff_t off;

			p += (mz - 1U) / sizeof(*p) + 1U;
			off = be32toh(*p++) >> 8U;

			/* actually print the strings */
			fputs(mn, stdout);
			fputc('\t', stdout);
			fputs(m->data + off, stdout);
			fputc('\n', stdout);
		}
	}
	/* otherwise */
	for (size_t i = 0U; i < argi->nargs; i++) {
		const char *zn;

		if ((zn = tzm_find(m, argi->args[i])) != NULL) {
			puts(zn);
		}
	}

	/* and off we go */
	tzm_close(m);
	return rc;
}

static int
cmd_check(const struct yuck_cmd_check_s argi[static 1U])
{
	int rc = 0;

	for (size_t i = 0U; i < argi->nargs || i == 0U; i++) {
		rc |= check_file(argi->args[i]);
	}
	return -rc;
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
	case TZMAP_CMD_SHOW:
		rc = cmd_show((void*)argi);
		break;
	case TZMAP_CMD_CHECK:
		rc = cmd_check((void*)argi);
		break;
	default:
		rc = 1;
		break;
	}

out:
	yuck_free(argi);
	return rc;
}
#endif	/* STANDALONE */

/* tzmap.c ends here */
