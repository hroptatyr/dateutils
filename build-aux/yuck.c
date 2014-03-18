/*** yuck.c -- generate umbrella commands
 *
 * Copyright (C) 2013 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of yuck.
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
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#if defined WITH_SCMVER
# include <yuck-scmver.h>
#endif	/* WITH_SCMVER */

#if !defined LIKELY
# define LIKELY(_x)	__builtin_expect((_x), 1)
#endif	/* !LIKELY */
#if !defined UNLIKELY
# define UNLIKELY(_x)	__builtin_expect((_x), 0)
#endif	/* UNLIKELY */
#if !defined UNUSED
# define UNUSED(_x)	_x __attribute__((unused))
#endif	/* !UNUSED */

#if !defined countof
# define countof(x)	(sizeof(x) / sizeof(*x))
#endif	/* !countof */

#define _paste(x, y)	x ## y
#define paste(x, y)	_paste(x, y)
#if !defined with
# define with(args...)							\
	for (args, *paste(__ep, __LINE__) = (void*)1;			\
	     paste(__ep, __LINE__); paste(__ep, __LINE__) = 0)
#endif	/* !with */

#if !defined HAVE_GETLINE && !defined HAVE_FGETLN
/* as a service to people including this file in their project
 * but who might not necessarily run the corresponding AC_CHECK_FUNS
 * we assume that a getline() is available. */
# define HAVE_GETLINE	1
#endif	/* !HAVE_GETLINE && !HAVE_FGETLN */

typedef enum {
	YOPT_NONE,
	YOPT_ALLOW_UNKNOWN_DASH,
	YOPT_ALLOW_UNKNOWN_DASHDASH,
} yopt_t;

struct usg_s {
	char *umb;
	char *cmd;
	char *parg;
	char *desc;
};

struct opt_s {
	char sopt;
	char *lopt;
	char *larg;
	char *desc;
	unsigned int oarg:1U;
	unsigned int marg:1U;
};

#if !defined BOOTSTRAP && defined WITH_SCMVER
static const char *yscm_strs[] = {
	[YUCK_SCM_TARBALL] = "tarball",
	[YUCK_SCM_GIT] = "git",
	[YUCK_SCM_BZR] = "bzr",
	[YUCK_SCM_HG] = "hg",
};
#elif !defined BOOTSTRAP
/* just forward declare this type so function signatures will work */
struct yuck_version_s;
#endif	/* WITH_SCMVER */


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

static inline __attribute__((unused)) void*
deconst(const void *cp)
{
	union {
		const void *c;
		void *p;
	} tmp = {cp};
	return tmp.p;
}

static inline __attribute__((always_inline)) unsigned int
yfls(unsigned int x)
{
	return x ? sizeof(x) * 8U - __builtin_clz(x) : 0U;
}

static inline __attribute__((always_inline)) size_t
max_zu(size_t x, size_t y)
{
	return x > y ? x : y;
}

static size_t
xstrncpy(char *restrict dst, const char *src, size_t ssz)
{
	memcpy(dst, src, ssz);
	dst[ssz] = '\0';
	return ssz;
}

static __attribute__((unused)) size_t
xstrlcpy(char *restrict dst, const char *src, size_t dsz)
{
	size_t ssz = strlen(src);
	if (ssz > dsz) {
		ssz = dsz - 1U;
	}
	memcpy(dst, src, ssz);
	dst[ssz] = '\0';
	return ssz;
}

static __attribute__((unused)) size_t
xstrlncpy(char *restrict dst, size_t dsz, const char *src, size_t ssz)
{
	if (ssz > dsz) {
		ssz = dsz - 1U;
	}
	memcpy(dst, src, ssz);
	dst[ssz] = '\0';
	return ssz;
}

static __attribute__((unused)) bool
xstreqp(const char *s1, const char *s2)
{
	if (s1 == NULL && s2 == NULL) {
		return true;
	} else if (s1 == NULL || s2 == NULL) {
		/* one of them isn't NULL */
		return false;
	}
	/* resort to normal strcmp */
	return !strcasecmp(s1, s2);
}

static bool
only_whitespace_p(const char *line, size_t llen)
{
	for (const char *lp = line, *const ep = line + llen; lp < ep; lp++) {
		if (!isspace(*lp)) {
			return false;
		}
	}
	return true;
}

static bool
isdashdash(const char c)
{
	switch (c) {
	default:
		return true;
	case '=':
	case '[':
	case ']':
	case '.':
	case '\0' ... ' ':
		return false;
	}
}

static void
massage_desc(char *str)
{
/* kick final newline and escape m4 quoting characters */
	char *sp;

	for (sp = str; *sp; sp++) {
		switch (*sp) {
		default:
			break;
		case '[':
			/* map to STX (start of text) */
			*sp = '\002';
			break;
		case ']':
			/* map to ETX (end of text) */
			*sp = '\003';
			break;
		case '(':
			/* map to SO (shift out) */
			*sp = '\016';
			break;
		case ')':
			/* map to SI (shift in) */
			*sp = '\017';
			break;
		}
	}
	if (sp > str && sp[-1] == '\n') {
		*--sp = '\0';
	}
	return;
}

#if !defined BOOTSTRAP
static void
unmassage_buf(char *restrict buf, size_t bsz)
{
/* turn m4 quoting character substitutes into brackets again */
	for (char *restrict sp = buf, *const ep = buf + bsz; sp < ep; sp++) {
		switch (*sp) {
		default:
			break;
		case '\002':
			/* unmap STX (start of text) */
			*sp = '[';
			break;
		case '\003':
			/* unmap ETX (end of text) */
			*sp = ']';
			break;
		case '\016':
			/* unmap SO (shift out) */
			*sp = '(';
			break;
		case '\017':
			/* unmap SI (shift in) */
			*sp = ')';
			break;
		}
	}
	return;
}

static int
mktempp(char *restrict tmpl[static 1U], int prefixlen)
{
	char *bp = *tmpl + prefixlen;
	char *const ep = *tmpl + strlen(*tmpl);
	int fd;

	if (ep[-6] != 'X' || ep[-5] != 'X' || ep[-4] != 'X' ||
	    ep[-3] != 'X' || ep[-2] != 'X' || ep[-1] != 'X') {
		if ((fd = open(bp, O_RDWR | O_CREAT | O_EXCL, 0666)) < 0 &&
		    (bp -= prefixlen,
		     fd = open(bp, O_RDWR | O_CREAT | O_EXCL, 0666)) < 0) {
			/* fuck that then */
			return -1;
		}
	} else if (UNLIKELY((fd = mkstemp(bp)) < 0) &&
		   UNLIKELY((bp -= prefixlen,
			     /* reset to XXXXXX */
			     memset(ep - 6, 'X', 6U),
			     fd = mkstemp(bp)) < 0)) {
		/* at least we tried */
		return -1;
	}
	/* store result */
	*tmpl = bp;
	return fd;
}

static FILE*
mkftempp(char *restrict tmpl[static 1U], int prefixlen)
{
	int fd;

	if (UNLIKELY((fd = mktempp(tmpl, prefixlen)) < 0)) {
		return NULL;
	}
	return fdopen(fd, "w");
}

# if defined WITH_SCMVER
static bool
regfilep(const char *fn)
{
	struct stat st[1U];
	return stat(fn, st) == 0 && S_ISREG(st->st_mode);
}
# endif	/* WITH_SCMVER */
#endif	/* !BOOTSTRAP */


/* bang buffers */
typedef struct {
	/* the actual buffer (resizable) */
	char *s;
	/* current size */
	size_t z;
}  bbuf_t;

static char*
bbuf_cpy(bbuf_t b[static 1U], const char *str, size_t ssz)
{
	size_t nu = max_zu(yfls(ssz + 1U) + 1U, 6U);
	size_t ol = b->z ? max_zu(yfls(b->z) + 1U, 6U) : 0U;

	if (UNLIKELY(nu > ol)) {
		b->s = realloc(b->s, (1U << nu) * sizeof(*b->s));
	}
	xstrncpy(b->s, str, ssz);
	b->z += ssz;
	return b->s;
}

static char*
bbuf_cat(bbuf_t b[static 1U], const char *str, size_t ssz)
{
	size_t nu = max_zu(yfls(b->z + ssz + 1U) + 1U, 6U);
	size_t ol = b->z ? max_zu(yfls(b->z) + 1U, 6U) : 0U;

	if (UNLIKELY(nu > ol)) {
		b->s = realloc(b->s, (1U << nu) * sizeof(*b->s));
	}
	xstrncpy(b->s + b->z, str, ssz);
	b->z += ssz;
	return b->s;
}


static void yield_usg(const struct usg_s *arg);
static void yield_opt(const struct opt_s *arg);
static void yield_inter(const bbuf_t x[static 1U]);
static void yield_setopt(yopt_t);

#define DEBUG(args...)

static int
usagep(const char *line, size_t llen)
{
#define STREQLITP(x, lit)      (!strncasecmp((x), lit, sizeof(lit) - 1))
	static struct usg_s cur_usg;
	static bbuf_t umb[1U];
	static bbuf_t cmd[1U];
	static bbuf_t parg[1U];
	static bbuf_t desc[1U];
	static bool cur_usg_yldd_p;
	static bool umb_yldd_p;
	const char *sp;
	const char *up;
	const char *cp;
	const char *const ep = line + llen;

	if (UNLIKELY(line == NULL)) {
		goto yield;
	}

	DEBUG("USAGEP CALLED with %s", line);

	if (STREQLITP(line, "setopt")) {
		/* it's a setopt */
		return 0;
	} else if (!STREQLITP(line, "usage:")) {
		if (only_whitespace_p(line, llen) && !desc->z) {
			return 1;
		} else if (!isspace(*line) && !cur_usg_yldd_p) {
			/* append to description */
			cur_usg.desc = bbuf_cat(desc, line, llen);
			return 1;
		}
	yield:
#define RESET	cur_usg.cmd = cur_usg.parg = cur_usg.desc = NULL, desc->z = 0U

		if (!cur_usg_yldd_p) {
			yield_usg(&cur_usg);
			/* reset */
			RESET;
			cur_usg_yldd_p = true;
			umb_yldd_p = true;
		}
		return 0;
	} else if (!cur_usg_yldd_p) {
		/* can't just goto yield because they wander off */
		yield_usg(&cur_usg);
		/* reset */
		RESET;
		cur_usg_yldd_p = true;
		umb_yldd_p = true;
	}
	/* overread whitespace then */
	for (sp = line + sizeof("usage:") - 1; sp < ep && isspace(*sp); sp++);
	/* first thing should name the umbrella, find its end */
	for (up = sp; sp < ep && !isspace(*sp); sp++);

	if (cur_usg.umb && !strncasecmp(cur_usg.umb, up, sp - up)) {
		/* nothing new and fresh */
		;
	} else {
		cur_usg.umb = bbuf_cpy(umb, up, sp - up);
		umb_yldd_p = false;
	}

	/* overread more whitespace and [--BLA] decls then */
overread:
	for (; sp < ep && isspace(*sp); sp++);
	/* we might be strafed with option decls here */
	switch (*sp) {
	case '[':
		if (sp[1U] == '-') {
			/* might be option spec [-x], read on */
			;
		} else if (STREQLITP(sp + 1U, "OPTION")) {
			/* definitely an option marker innit? */
			;
		} else {
			/* could be posarg, better exit here */
			break;
		}
		/* otherwise read till closing bracket */
		for (sp++; sp < ep && *sp++ != ']';);
		/* and also read over `...' */
		for (sp++; sp < ep && *sp == '.'; sp++);
		goto overread;
	default:
		/* best leave the loop */
		break;
	}

	/* now it's time for the command innit */
	for (cp = sp; sp < ep && !isspace(*sp); sp++);

	if (cur_usg.cmd && !strncasecmp(cur_usg.cmd, cp, sp - cp)) {
		/* nothing new and fresh */
		;
	} else if ((*cp != '<' || (cp++, *--sp == '>')) &&
		   !strncasecmp(cp, "command", sp - cp)) {
		/* special command COMMAND or <command> */
		cur_usg.cmd = NULL;
	} else if (*cp >= 'a' && *cp <= 'z' && umb_yldd_p) {
		/* we mandate commands start with a lower case alpha char */
		cur_usg.cmd = bbuf_cpy(cmd, cp, sp - cp);
	} else {
		/* not a command, could be posarg innit, so rewind */
		sp = cp;
	}

	/* now there might be positional args, snarf them */
	for (; sp < ep && isspace(*sp); sp++);
	if (sp < ep) {
		cur_usg.parg = bbuf_cpy(parg, sp, ep - sp - 1U);
	}
	cur_usg_yldd_p = false;
	return 1;
}

static int
optionp(const char *line, size_t llen)
{
	static struct opt_s cur_opt;
	static bbuf_t desc[1U];
	static bbuf_t lopt[1U];
	static bbuf_t larg[1U];
	const char *sp = line;
	const char *const ep = line + llen;

	if (UNLIKELY(line == NULL)) {
		goto yield;
	}

	DEBUG("OPTIONP CALLED with %s", line);

	/* overread whitespace */
	for (; sp < ep && isspace(*sp); sp++);
	if (sp - line >= 2 && *sp != '-' && (cur_opt.sopt || cur_opt.lopt)) {
		/* should be description */
		goto desc;
	}

yield:
	/* must yield the old current option before it's too late */
	if (cur_opt.sopt || cur_opt.lopt) {
		yield_opt(&cur_opt);
	}
	/* complete reset */
	memset(&cur_opt, 0, sizeof(cur_opt));
	if (sp - line < 2) {
		/* can't be an option, can it? */
		return 0;
	} else if (!*sp) {
		/* not an option either */
		return 0;
	}

	/* no yield pressure anymore, try parsing the line */
	sp++;
	if (*sp >= '0') {
		char sopt = *sp++;

		/* eat a comma as well */
		if (*sp == ',') {
			sp++;
		}
		if (!isspace(*sp)) {
			/* dont know -x.SOMETHING? */
			return 0;
		}
		/* start over with the new option */
		sp++;
		cur_opt.sopt = sopt;
		if (*sp == '\0') {
			/* just the short option then innit? */
			return 1;
		} else if (isspace(*sp)) {
			/* no arg name, no longopt */
			;
		} else if (*sp == '-') {
			/* must be a --long now, maybe */
			sp++;
		} else {
			/* just an arg name */
			const char *ap;

			if (*sp == '[') {
				cur_opt.oarg = 1U;
				sp++;
			}
			if (*sp == '=') {
				sp++;
			}
			for (ap = sp; sp < ep && isdashdash(*sp); sp++);
			cur_opt.larg = bbuf_cpy(larg, ap, sp - ap);
			if (cur_opt.oarg && *sp++ != ']') {
				/* maybe not an optarg? */
				;
			}
			if (*sp == '.') {
				/* could be mularg */
				if (sp[1U] == '.' && sp[2U] == '.') {
					/* yay, 3 dots, read over dots */
					for (sp += 3U; *sp == '.'; sp++);
					cur_opt.marg = 1U;
				}
			}
		}
	} else if (*sp == '-') {
		/* --option */
		;
	} else {
		/* dont know what this is */
		return 0;
	}

	/* --option */
	if (*sp++ == '-') {
		const char *op;

		for (op = sp; sp < ep && isdashdash(*sp); sp++);
		cur_opt.lopt = bbuf_cpy(lopt, op, sp - op);

		switch (*sp++) {
		case '[':
			if (*sp++ != '=') {
				/* just bullshit then innit? */
				break;
			}
			/* otherwise optarg, fall through */
			cur_opt.oarg = 1U;
		case '=':;
			/* has got an arg */
			const char *ap;
			for (ap = sp; sp < ep && isdashdash(*sp); sp++);
			cur_opt.larg = bbuf_cpy(larg, ap, sp - ap);
			if (cur_opt.oarg && *sp++ != ']') {
				/* maybe not an optarg? */
				;
			}
			if (*sp == '.') {
				/* could be mularg */
				if (sp[1U] == '.' && sp[2U] == '.') {
					/* yay, 3 dots, read over dots */
					for (sp += 3U; *sp == '.'; sp++);
					cur_opt.marg = 1U;
				}
			}
		default:
			break;
		}
	}
	/* require at least one more space? */
	;
	/* space eater */
	for (; sp < ep && isspace(*sp); sp++);
	/* dont free but reset the old guy */
	desc->z = 0U;
desc:
	with (size_t sz = llen - (sp - line)) {
		if (LIKELY(sz > 0U)) {
			cur_opt.desc = bbuf_cat(desc, sp, sz);
		}
	}
	return 1;
}

static int
interp(const char *line, size_t llen)
{
	static bbuf_t desc[1U];
	bool only_ws_p = only_whitespace_p(line, llen);

	if (UNLIKELY(line == NULL)) {
		goto yield;
	}

	DEBUG("INTERP CALLED with %s", line);
	if (only_ws_p && desc->z) {
	yield:
		yield_inter(desc);
		/* reset */
		desc->z = 0U;
	} else if (!only_ws_p) {
		if (STREQLITP(line, "setopt")) {
			/* not an inter */
			return 0;
		}
		/* snarf the line */
		bbuf_cat(desc, line, llen);
		return 1;
	}
	return 0;
}

static int
setoptp(const char *line, size_t UNUSED(llen))
{
	if (UNLIKELY(line == NULL)) {
		return 0;
	}

	DEBUG("SETOPTP CALLED with %s", line);
	if (STREQLITP(line, "setopt")) {
		/* 'nother option */
		const char *lp = line + sizeof("setopt");

		if (0) {
			;
		} else if (STREQLITP(lp, "allow-unknown-dash-options")) {
			yield_setopt(YOPT_ALLOW_UNKNOWN_DASH);
		} else if (STREQLITP(lp, "allow-unknown-dashdash-options")) {
			yield_setopt(YOPT_ALLOW_UNKNOWN_DASHDASH);
		} else {
			/* unknown setopt option */
		}
	}
	return 0;
}


static const char nul_str[] = "";
static const char *const auto_types[] = {"auto", "flag"};
static FILE *outf;

static struct {
	unsigned int no_auto_flags:1U;
	unsigned int no_auto_action:1U;
} global_tweaks;

static void
__identify(char *restrict idn)
{
	for (char *restrict ip = idn; *ip; ip++) {
		switch (*ip) {
		case '0' ... '9':
		case 'A' ... 'Z':
		case 'a' ... 'z':
			break;
		default:
			*ip = '_';
		}
	}
	return;
}

static size_t
count_pargs(const char *parg)
{
/* return max posargs as helper for auto-dashdash commands */
	const char *pp;
	size_t res;

	for (res = 0U, pp = parg; *pp;) {
		/* allow [--] or -- as auto-dashdash declarators */
		if (*pp == '[') {
			pp++;
		}
		if (*pp++ == '-') {
			if (*pp++ == '-') {
				if (*pp == ']' || isspace(*pp)) {
					/* found him! */
					return res;
				}
			}
			/* otherwise not the declarator we were looking for
			 * fast forward to the end */
			for (; *pp && !isspace(*pp); pp++);
		} else {
			/* we know it's a bog-standard posarg for sure */
			res++;
			/* check for ellipsis */
			for (; *pp && *pp != '.' && !isspace(*pp); pp++);
			if (!*pp) {
				/* end of parg string anyway */
				break;
			}
			if (*pp++ == '.' && *pp++ == '.' && *pp++ == '.') {
				/* ellipsis, set res to infinity and bog off */
				break;
			}
		}
		/* fast forward over all the whitespace */
		for (; *pp && isspace(*pp); pp++);
	}
	return 0U;
}

static char*
make_opt_ident(const struct opt_s *arg)
{
	static bbuf_t i[1U];

	if (arg->lopt != NULL) {
		bbuf_cpy(i, arg->lopt, strlen(arg->lopt));
	} else if (arg->sopt) {
		bbuf_cpy(i, "dash.", 5U);
		i->s[4U] = arg->sopt;
	} else {
		static unsigned int cnt;
		bbuf_cpy(i, "idnXXXX", 7U);
		snprintf(i->s + 3U, 5U, "%u", cnt++);
	}
	__identify(i->s);
	return i->s;
}

static char*
make_ident(const char *str)
{
	static bbuf_t buf[1U];

	bbuf_cpy(buf, str, strlen(str));
	__identify(buf->s);
	return buf->s;
}

static void
yield_help(void)
{
	const char *type = auto_types[global_tweaks.no_auto_action];

	fprintf(outf, "yuck_add_option([help], [h], [help], [%s])\n", type);
	fprintf(outf, "yuck_set_option_desc([help], [\
display this help and exit])\n");
	return;
}

static void
yield_version(void)
{
	const char *type = auto_types[global_tweaks.no_auto_action];

	fprintf(outf,
		"yuck_add_option([version], [V], [version], [%s])\n", type);
	fprintf(outf, "yuck_set_option_desc([version], [\
output version information and exit])\n");
	return;
}

static void
yield_usg(const struct usg_s *arg)
{
	const char *parg = arg->parg ?: nul_str;
	size_t nparg = count_pargs(parg);

	if (arg->desc != NULL) {
		/* kick last newline */
		massage_desc(arg->desc);
	}
	if (arg->cmd != NULL) {
		const char *idn = make_ident(arg->cmd);

		fprintf(outf, "\nyuck_add_command([%s], [%s], [%s])\n",
			idn, arg->cmd, parg);
		if (nparg) {
			fprintf(outf,
				"yuck_set_command_max_posargs([%s], [%zu])\n",
				idn, nparg);
		}
		if (arg->desc != NULL) {
			fprintf(outf, "yuck_set_command_desc([%s], [%s])\n",
				idn, arg->desc);
		}
	} else if (arg->umb != NULL) {
		const char *idn = make_ident(arg->umb);

		fprintf(outf, "\nyuck_set_umbrella([%s], [%s], [%s])\n",
			idn, arg->umb, parg);
		if (nparg) {
			fprintf(outf,
				"yuck_set_umbrella_max_posargs([%s], [%zu])\n",
				idn, nparg);
		}
		if (arg->desc != NULL) {
			fprintf(outf, "yuck_set_umbrella_desc([%s], [%s])\n",
				idn, arg->desc);
		}
		/* insert auto-help and auto-version */
		if (!global_tweaks.no_auto_flags) {
			yield_help();
			yield_version();
		}
	}
	return;
}

static void
yield_opt(const struct opt_s *arg)
{
	char sopt[2U] = {arg->sopt, '\0'};
	const char *opt = arg->lopt ?: nul_str;
	const char *idn = make_opt_ident(arg);

	if (arg->larg == NULL) {
		fprintf(outf, "yuck_add_option([%s], [%s], [%s], "
			"[flag]);\n", idn, sopt, opt);
	} else {
		const char *asufs[] = {
			nul_str, ", opt", ", mul", ", mul, opt"
		};
		const char *asuf = asufs[arg->oarg | arg->marg << 1U];
		fprintf(outf, "yuck_add_option([%s], [%s], [%s], "
			"[arg, %s%s]);\n", idn, sopt, opt, arg->larg, asuf);
	}
	if (arg->desc != NULL) {
		massage_desc(arg->desc);
		fprintf(outf,
			"yuck_set_option_desc([%s], [%s])\n", idn, arg->desc);
	}
	return;
}

static void
yield_inter(const bbuf_t x[static 1U])
{
	if (x->z) {
		if (x->s[x->z - 1U] == '\n') {
			x->s[x->z - 1U] = '\0';
		}
		massage_desc(x->s);
		fprintf(outf, "yuck_add_inter([%s])\n", x->s);
	}
	return;
}

static void
yield_setopt(yopt_t yo)
{
	switch (yo) {
	default:
	case YOPT_NONE:
		break;
	case YOPT_ALLOW_UNKNOWN_DASH:
		fputs("yuck_setopt_allow_unknown_dash\n", outf);
		break;
	case YOPT_ALLOW_UNKNOWN_DASHDASH:
		fputs("yuck_setopt_allow_unknown_dashdash\n", outf);
		break;
	}
	return;
}


static enum {
	UNKNOWN,
	SET_INTER,
	SET_UMBCMD,
	SET_OPTION,
	SET_SETOPT,
}
snarf_ln(char *line, size_t llen)
{
	static unsigned int st;

	switch (st) {
	case UNKNOWN:
	case SET_UMBCMD:
	usage:
		/* first keep looking for Usage: lines */
		if (usagep(line, llen)) {
			st = SET_UMBCMD;
			break;
		} else if (st == SET_UMBCMD) {
			/* reset state, go on with option parsing */
			st = UNKNOWN;
			goto option;
		}
	case SET_OPTION:
	option:
		/* check them option things */
		if (optionp(line, llen)) {
			st = SET_OPTION;
			break;
		} else if (st == SET_OPTION) {
			/* reset state, go on with usage parsing */
			st = UNKNOWN;
			goto usage;
		}
	case SET_INTER:
		/* check for some intro texts */
		if (interp(line, llen)) {
			st = SET_INTER;
			break;
		} else {
			/* reset state, go on with setopt parsing */
			st = UNKNOWN;
		}
	case SET_SETOPT:
		/* check for setopt BLA lines */
		if (setoptp(line, llen)) {
			st = SET_SETOPT;
			break;
		} else {
			/* reset state, go on with option parsing */
			st = UNKNOWN;
		}
	default:
		break;
	}
	return UNKNOWN;
}

static int
snarf_f(FILE *f)
{
	char *line = NULL;
	size_t llen = 0U;

#if defined HAVE_GETLINE
	for (ssize_t nrd; (nrd = getline(&line, &llen, f)) > 0;) {
		if (*line == '#') {
			continue;
		}
		snarf_ln(line, nrd);
	}
#elif defined HAVE_FGETLN
	while ((line = fgetln(f, &llen)) != NULL) {
		if (*line == '#') {
			continue;
		}
		snarf_ln(line, llen);
	}
#else
# error neither getline() nor fgetln() available, cannot read file line by line
#endif	/* GETLINE/FGETLN */
	/* drain */
	snarf_ln(NULL, 0U);

#if defined HAVE_GETLINE
	free(line);
#endif	/* HAVE_GETLINE */
	return 0;
}


#if defined BOOTSTRAP
static FILE*
get_fn(int argc, char *argv[])
{
	FILE *res;

	if (argc > 1) {
		const char *fn = argv[1];
		if (UNLIKELY((res = fopen(fn, "r")) == NULL)) {
			error("cannot open file `%s'", fn);
		}
	} else {
		res = stdin;
	}
	return res;
}

int
main(int argc, char *argv[])
{
	int rc = 0;
	FILE *yf;

	if (UNLIKELY((yf = get_fn(argc, argv)) == NULL)) {
		rc = -1;
	} else {
		/* always use stdout */
		outf = stdout;

		fputs("\
changequote([,])dnl\n\
divert([-1])\n", outf);

		/* let the snarfing begin */
		rc = snarf_f(yf);

		fputs("\n\
changecom([//])\n\
divert[]dnl\n", outf);

		/* clean up */
		fclose(yf);
	}

	return -rc;
}
#endif	/* BOOTSTRAP */


#if !defined BOOTSTRAP
#if !defined PATH_MAX
# define PATH_MAX	(256U)
#endif	/* !PATH_MAX */
static char dslfn[PATH_MAX];

static bool
aux_in_path_p(const char *aux, const char *path, size_t pathz)
{
	char fn[PATH_MAX];
	char *restrict fp = fn;
	struct stat st[1U];

	fp += xstrlncpy(fn, sizeof(fn), path, pathz);
	*fp++ = '/';
	xstrlcpy(fp, aux, sizeof(fn) - (fp - fn));

	if (stat(fn, st) < 0) {
		return false;
	}
	return S_ISREG(st->st_mode);
}

static ssize_t
get_myself(char *restrict buf, size_t bsz)
{
	ssize_t off;
	char *mp;

	if ((off = readlink("/proc/self/exe", buf, bsz)) < 0) {
		return -1;
	}
	/* go back to the dir bit */
	for (mp = buf + off - 1U; mp > buf && *mp != '/'; mp--);
	/* should be bin/, go up one level */
	*mp = '\0';
	for (; mp > buf && *mp != '/'; mp--);
	/* check if we're right */
	if (UNLIKELY(strcmp(++mp, "bin"))) {
		/* oh, it's somewhere but not bin/? */
		return -1;
	}
	/* now just use share/yuck/ */
	xstrlcpy(mp, "share/yuck/", bsz - (mp - buf));
	mp += sizeof("share/yuck");
	return mp - buf;
}

static int
find_aux(char *restrict buf, size_t bsz, const char *aux)
{
	/* look up path relative to binary position */
	static char pkgdatadir[PATH_MAX];
	static ssize_t pkgdatalen;
	static const char *tmplpath;
	static ssize_t tmplplen;
	const char *path;
	size_t plen;

	/* start off by snarfing the environment */
	if (tmplplen == 0U) {
		if ((tmplpath = getenv("YUCK_TEMPLATE_PATH")) != NULL) {
			tmplplen = strlen(tmplpath);
		} else {
			/* just set it to something non-0 to indicate initting
			 * and that also works with the loop below */
			tmplplen = -1;
			tmplpath = (void*)0x1U;
		}
	}

	/* snarf pkgdatadir */
	if (pkgdatalen == 0U) {
		pkgdatalen = get_myself(pkgdatadir, sizeof(pkgdatadir));
	}

	/* go through the path first */
	for (const char *pp = tmplpath, *ep, *const end = tmplpath + tmplplen;
	     pp < end; pp = ep + 1U) {
		ep = strchr(pp, ':') ?: end;
		if (aux_in_path_p(aux, pp, ep - pp)) {
			path = pp;
			plen = ep - pp;
			goto bang;
		}
	}
	/* no luck with the env path then aye */
	if (pkgdatalen > 0 && aux_in_path_p(aux, pkgdatadir, pkgdatalen)) {
		path = pkgdatadir;
		plen = pkgdatalen;
		goto bang;
	}
#if defined YUCK_TEMPLATE_PATH
	path = YUCK_TEMPLATE_PATH;
	plen = sizeof(YUCK_TEMPLATE_PATH);
	if (plen-- > 0U && aux_in_path_p(aux, path, plen)) {
		goto bang;
	}
#endif	/* YUCK_TEMPLATE_PATH */
	/* not what we wanted at all, must be christmas */
	return -1;

bang:
	with (size_t z) {
		z = xstrlncpy(buf, bsz, path, plen);
		buf[z++] = '/';
		xstrlcpy(buf + z, aux, bsz - z);
	}
	return 0;
}

static int
find_dsl(void)
{
	return find_aux(dslfn, sizeof(dslfn), "yuck.m4");
}

static void
unmassage_fd(int tgtfd, int srcfd)
{
	static char buf[4096U];

	for (ssize_t nrd; (nrd = read(srcfd, buf, sizeof(buf))) > 0;) {
		const char *bp = buf;
		const char *const ep = buf + nrd;

		unmassage_buf(buf, nrd);
		for (ssize_t nwr;
		     bp < ep && (nwr = write(tgtfd, bp, ep - bp)) > 0;
		     bp += nwr);
	}
	return;
}


static char *m4_cmdline[16U] = {
	"m4",
};
static size_t cmdln_idx;

static int
prep_m4(void)
{
	char *p;

	/* checkout the environment, look for M4 */
	if ((p = getenv("M4")) == NULL) {
		cmdln_idx = 1U;
		return 0;
	}
	/* otherwise it's big string massaging business */
	do {
		m4_cmdline[cmdln_idx++] = p;

		/* mimic a shell's IFS */
		for (; *p && !isspace(*p); p++) {
			const char this = *p;

			switch (this) {
			default:
				break;
			case '"':
			case '\'':
				/* fast forward then */
				while (*++p != this) {
					if (*p == '\\') {
						p++;
					}
				}
				break;
			}
		}
		if (!*p) {
			break;
		}
		/* otherwise it's an IFS */
		for (*p++ = '\0'; isspace(*p); p++);
	} while (1);
	return 0;
}

static __attribute__((noinline)) int
run_m4(const char *outfn, ...)
{
	pid_t m4p;
	/* to snarf off traffic from the child */
	int intfd[2];

	if (pipe(intfd) < 0) {
		error("pipe setup to/from m4 failed");
		return -1;
	} else if (!cmdln_idx && prep_m4() < 0) {
		error("m4 preparations failed");
		return -1;
	}

	switch ((m4p = vfork())) {
	case -1:
		/* i am an error */
		error("vfork for m4 failed");
		return -1;

	default:;
		/* i am the parent */
		int rc;
		int st;

		if (outfn != NULL) {
			/* --output given */
			const int outfl = O_RDWR | O_CREAT | O_TRUNC;
			int outfd;

			if ((outfd = open(outfn, outfl, 0666)) < 0) {
				/* bollocks */
				error("cannot open outfile `%s'", outfn);
				goto bollocks;
			}

			/* really redir now */
			dup2(outfd, STDOUT_FILENO);
		}

		close(intfd[1]);
		unmassage_fd(STDOUT_FILENO, intfd[0]);

		rc = 2;
		while (waitpid(m4p, &st, 0) != m4p);
		if (WIFEXITED(st)) {
			rc = WEXITSTATUS(st);
		}
		/* clean up the rest of the pipe */
		close(intfd[0]);
		return rc;

	case 0:;
		/* i am the child */
		break;
	}

	/* child code here */
	with (va_list vap) {
		va_start(vap, outfn);
		for (size_t i = cmdln_idx;
		     i < countof(m4_cmdline) &&
			     (m4_cmdline[i] = va_arg(vap, char*)) != NULL; i++);
		va_end(vap);
	}

	dup2(intfd[1], STDOUT_FILENO);
	close(intfd[0]);

	execvp(m4_cmdline[0U], m4_cmdline);
	error("execvp(m4) failed");
bollocks:
	_exit(EXIT_FAILURE);
}


static int
wr_pre(void)
{
	fputs("\
changequote`'changequote([,])dnl\n\
divert([-1])\n", outf);
	return 0;
}

static int
wr_suf(void)
{
	fputs("\n\
changequote`'dnl\n\
divert`'dnl\n", outf);
	return 0;
}

static int
wr_intermediary(char *const args[], size_t nargs)
{
	int rc = 0;

	wr_pre();

	if (nargs == 0U) {
		if (snarf_f(stdin) < 0) {
			error("cannot interpret directives on stdin");
			rc = 1;
		}
	}
	for (unsigned int i = 0U; i < nargs && rc == 0; i++) {
		const char *fn = args[i];
		FILE *yf;

		if (UNLIKELY((yf = fopen(fn, "r")) == NULL)) {
			error("cannot open file `%s'", fn);
			rc = 1;
			break;
		} else if (snarf_f(yf) < 0) {
			error("cannot interpret directives from `%s'", fn);
			rc = 1;
		}

		/* clean up */
		fclose(yf);
	}
	/* reset to sane values */
	wr_suf();
	return rc;
}

static int
wr_header(const char hdr[static 1U])
{
	/* massage the hdr bit a bit */
	if (strcmp(hdr, "/dev/null")) {
		/* /dev/null just means ignore the header aye? */
		const char *hp;

		if ((hp = strrchr(hdr, '/')) == NULL) {
			hp = hdr;
		} else {
			hp++;
		};
		wr_pre();
		fprintf(outf, "define([YUCK_HEADER], [%s])dnl\n", hp);
		wr_suf();
	}
	return 0;
}

static int
wr_man_date(void)
{
	time_t now;
	const struct tm *tp;
	char buf[32U];
	int rc = 0;

	if ((now = time(NULL)) == (time_t)-1) {
		rc = -1;
	} else if ((tp = gmtime(&now)) == NULL) {
		rc = -1;
	} else if (!strftime(buf, sizeof(buf), "%B %Y", tp)) {
		rc = -1;
	} else {
		fprintf(outf, "define([YUCK_MAN_DATE], [%s])dnl\n", buf);
	}
	return rc;
}

static int
wr_man_pkg(const char *pkg)
{
	fprintf(outf, "define([YUCK_PKG_STR], [%s])dnl\n", pkg);
	return 0;
}

static int
wr_man_nfo(const char *nfo)
{
	fprintf(outf, "define([YUCK_NFO_STR], [%s])dnl\n", nfo);
	return 0;
}

static int
wr_man_incln(FILE *fp, char *restrict ln, size_t lz)
{
	static int verbp;
	static int parap;

	if (UNLIKELY(ln == NULL)) {
		/* drain mode */
		if (verbp) {
			fputs(".fi\n", fp);
		}
	} else if (lz <= 1U/*has at least a newline?*/) {
		if (verbp) {
			/* close verbatim mode */
			fputs(".fi\n", fp);
			verbp = 0;
		}
		if (!parap) {
			fputs(".PP\n", fp);
			parap = 1;
		}
	} else if (*ln == '[' && ln[lz - 2U] == ']') {
		/* section */
		char *restrict lp = ln + 1U;

		for (const char *const eol = ln + lz - 2U; lp < eol; lp++) {
			*lp = (char)toupper(*lp);
		}
		*lp = '\0';
		fputs(".SH ", fp);
		fputs(ln + 1U, fp);
		fputs("\n", fp);
		/* reset state */
		parap = 0;
		verbp = 0;
	} else if (ln[0U] == ' ' && ln[1U] == ' ' && !verbp) {
		fputs(".nf\n", fp);
		verbp = 1;
		goto cp;
	} else {
	cp:
		/* otherwise copy  */
		fwrite(ln, lz, sizeof(*ln), fp);
		parap = 0;
	}
	return 0;
}

static int
wr_man_include(char **const inc)
{
	char _ofn[] = P_tmpdir "/" "yuck_XXXXXX";
	char *ofn = _ofn;
	FILE *ofp;
	char *line = NULL;
	size_t llen = 0U;
	FILE *fp;

	if (UNLIKELY((fp = fopen(*inc, "r")) == NULL)) {
		error("Cannot open include file `%s', ignoring", *inc);
		*inc = NULL;
		return -1;
	} else if (UNLIKELY((ofp = mkftempp(&ofn, sizeof(P_tmpdir))) == NULL)) {
		error("Cannot open output file `%s', ignoring", ofn);
		*inc = NULL;
		return -1;
	}

	/* make sure we pass on ofn */
	*inc = strdup(ofn);

#if defined HAVE_GETLINE
	for (ssize_t nrd; (nrd = getline(&line, &llen, fp)) > 0;) {
		wr_man_incln(ofp, line, nrd);
	}
#elif defined HAVE_FGETLN
	while ((line = fgetln(f, &llen)) != NULL) {
		wr_man_incln(ofp, line, nrd);
	}
#else
# error neither getline() nor fgetln() available, cannot read file line by line
#endif	/* GETLINE/FGETLN */
	/* drain */
	wr_man_incln(ofp, NULL, 0U);

#if defined HAVE_GETLINE
	free(line);
#endif	/* HAVE_GETLINE */

	/* close files properly */
	fclose(fp);
	fclose(ofp);
	return 0;
}

static int
wr_man_includes(char *incs[], size_t nincs)
{
	for (size_t i = 0U; i < nincs; i++) {
		/* massage file */
		if (wr_man_include(incs + i) < 0) {
			continue;
		} else if (incs[i] == NULL) {
			/* something else is wrong */
			continue;
		}
		/* otherwise make a note to include this file */
		fprintf(outf, "\
append([YUCK_INCLUDES], [%s], [,])dnl\n", incs[i]);
	}
	return 0;
}

static int
wr_version(const struct yuck_version_s *v, const char *vlit)
{
	wr_pre();

	if (v != NULL) {
#if defined WITH_SCMVER
		const char *yscm = yscm_strs[v->scm];

		fprintf(outf, "define([YUCK_SCMVER_VTAG], [%s])\n", v->vtag);
		fprintf(outf, "define([YUCK_SCMVER_SCM], [%s])\n", yscm);
		fprintf(outf, "define([YUCK_SCMVER_DIST], [%u])\n", v->dist);
		fprintf(outf, "define([YUCK_SCMVER_RVSN], [%08x])\n", v->rvsn);
		if (!v->dirty) {
			fputs("define([YUCK_SCMVER_FLAG_CLEAN])\n", outf);
		} else {
			fputs("define([YUCK_SCMVER_FLAG_DIRTY])\n", outf);
		}

		/* for convenience */
		fputs("define([YUCK_SCMVER_VERSION], [", outf);
		fputs(v->vtag, outf);
		if (v->scm > YUCK_SCM_TARBALL && v->dist) {
			fputc('.', outf);
			fputs(yscm_strs[v->scm], outf);
			fprintf(outf, "%u.%08x", v->dist, v->rvsn);
		}
		if (v->dirty) {
			fputs(".dirty", outf);
		}
		fputs("])\n", outf);
#else  /* !WITH_SCMVER */
		errno = 0;
		error("\
scmver support not built in but ptr %p given to wr_version()", v);
#endif	/* WITH_SCMVER */
	}
	if (vlit != NULL) {
		fputs("define([YUCK_VERSION], [", outf);
		fputs(vlit, outf);
		fputs("])\n", outf);
	}
	wr_suf();
	return 0;
}

static int
rm_intermediary(const char *fn, int keepp)
{
	if (!keepp) {
		if (unlink(fn) < 0) {
			error("cannot remove intermediary `%s'", fn);
			return -1;
		}
	} else {
		/* otherwise print a nice message so users know
		 * the file we created */
		errno = 0;
		error("intermediary `%s' kept", fn);
	}
	return 0;
}

static int
rm_includes(char *const incs[], size_t nincs, int keepp)
{
	int rc = 0;

	errno = 0;
	for (size_t i = 0U; i < nincs; i++) {
		char *restrict fn;

		if ((fn = incs[i]) != NULL) {
			if (!keepp && unlink(fn) < 0) {
				error("cannot remove intermediary `%s'", fn);
				rc = -1;
			} else if (keepp) {
				/* otherwise print a nice message so users know
				 * the file we created */
				error("intermediary `%s' kept", fn);
			}
			free(fn);
		}
	}
	return rc;
}
#endif	/* !BOOTSTRAP */


#if !defined BOOTSTRAP
#include "yuck.yucc"

static int
cmd_gen(const struct yuck_cmd_gen_s argi[static 1U])
{
	static char _deffn[] = P_tmpdir "/" "yuck_XXXXXX";
	static char gencfn[PATH_MAX];
	static char genhfn[PATH_MAX];
	char *deffn = _deffn;
	int rc = 0;

	if (argi->no_auto_flags_flag) {
		global_tweaks.no_auto_flags = 1U;
	}
	if (argi->no_auto_actions_flag) {
		global_tweaks.no_auto_action = 1U;
	}

	/* deal with the output first */
	if (UNLIKELY((outf = mkftempp(&deffn, sizeof(P_tmpdir))) == NULL)) {
		error("cannot open intermediate file `%s'", deffn);
		return -1;
	}
	/* write up our findings in DSL language */
	rc = wr_intermediary(argi->args, argi->nargs);
	/* deal with hard wired version numbers */
	if (argi->version_arg) {
		rc += wr_version(NULL, argi->version_arg);
	}
	/* special directive for the header or is it */
	if (argi->header_arg != NULL) {
		rc += wr_header(argi->header_arg);
	}
	/* and we're finished with the intermediary */
	fclose(outf);

	/* only proceed if there has been no error yet */
	if (rc) {
		goto out;
	} else if (find_dsl() < 0) {
		/* error whilst finding our DSL and things */
		error("cannot find yuck dsl file");
		rc = 2;
		goto out;
	} else if (find_aux(gencfn, sizeof(gencfn), "yuck-coru.c.m4") < 0 ||
		   find_aux(genhfn, sizeof(genhfn), "yuck-coru.h.m4") < 0) {
		error("cannot find yuck template files");
		rc = 2;
		goto out;
	}
	/* now route that stuff through m4 */
	with (const char *outfn = argi->output_arg, *hdrfn) {
		if ((hdrfn = argi->header_arg) != NULL) {
			/* run a special one for the header */
			if ((rc = run_m4(hdrfn, dslfn, deffn, genhfn, NULL))) {
				break;
			}
			/* now run the whole shebang for the beef code */
			rc = run_m4(outfn, dslfn, deffn, gencfn, NULL);
			break;
		}
		/* standard case: pipe directives, then header, then code */
		rc = run_m4(outfn, dslfn, deffn, genhfn, gencfn, NULL);
	}
out:
	/* unlink include files */
	rm_intermediary(deffn, argi->keep_flag);
	return rc;
}

static int
cmd_genman(const struct yuck_cmd_genman_s argi[static 1U])
{
	static char _deffn[] = P_tmpdir "/" "yuck_XXXXXX";
	static char genmfn[PATH_MAX];
	char *deffn = _deffn;
	int rc = 0;

	/* deal with the output first */
	if (UNLIKELY((outf = mkftempp(&deffn, sizeof(P_tmpdir))) == NULL)) {
		error("cannot open intermediate file `%s'", deffn);
		return -1;
	}
	/* write up our findings in DSL language */
	rc = wr_intermediary(argi->args, argi->nargs);
	if (argi->version_string_arg) {
		rc += wr_version(NULL, argi->version_string_arg);
	} else if (argi->version_file_arg) {
#if defined WITH_SCMVER
		struct yuck_version_s v[1U];
		const char *verfn = argi->version_file_arg;

		if (yuck_version_read(v, verfn) < 0) {
			error("cannot read version number from `%s'", verfn);
			rc--;
		} else {
			rc += wr_version(v, NULL);
		}
#else  /* !WITH_SCMVER */
		errno = 0;
		error("\
scmver support not built in, --version-file cannot be used");
#endif	/* WITH_SCMVER */
	}
	/* reset to sane values */
	wr_pre();
	/* at least give the man page template an idea for YUCK_MAN_DATE */
	rc += wr_man_date();
	if (argi->package_arg) {
		/* package != umbrella */
		rc += wr_man_pkg(argi->package_arg);
	}
	if (argi->info_page_arg) {
		const char *nfo;

		if ((nfo = argi->info_page_arg) == YUCK_OPTARG_NONE) {
			nfo = "YUCK_PKG_STR";
		}
		rc += wr_man_nfo(nfo);
	}
	/* go through includes */
	wr_man_includes(argi->include_args, argi->include_nargs);
	/* reset to sane values */
	wr_suf();
	/* and we're finished with the intermediary */
	fclose(outf);

	/* only proceed if there has been no error yet */
	if (rc) {
		goto out;
	} else if (find_dsl() < 0) {
		/* error whilst finding our DSL and things */
		error("cannot find yuck dsl and template files");
		rc = 2;
		goto out;
	} else if (find_aux(genmfn, sizeof(genmfn), "yuck.man.m4") < 0) {
		error("cannot find yuck template for man pages");
		rc = 2;
		goto out;
	}
	/* now route that stuff through m4 */
	with (const char *outfn = argi->output_arg) {
		/* standard case: pipe directives, then header, then code */
		rc = run_m4(outfn, dslfn, deffn, genmfn, NULL);
	}
out:
	rm_includes(argi->include_args, argi->include_nargs, argi->keep_flag);
	rm_intermediary(deffn, argi->keep_flag);
	return rc;
}

static int
cmd_gendsl(const struct yuck_cmd_gendsl_s argi[static 1U])
{
	int rc = 0;

	if (argi->no_auto_flags_flag) {
		global_tweaks.no_auto_flags = 1U;
	}
	if (argi->no_auto_actions_flag) {
		global_tweaks.no_auto_action = 1U;
	}

	/* bang to stdout or argi->output_arg */
	with (const char *outfn = argi->output_arg) {
		if (outfn == NULL) {
			outf = stdout;
		} else if ((outf = fopen(outfn, "w")) == NULL) {
			error("cannot open outfile `%s'", outfn);
			return 1;
		}
	}
	rc += wr_intermediary(argi->args, argi->nargs);
	if (argi->version_arg) {
		rc += wr_version(NULL, argi->version_arg);
	}
	return rc;
}

static int
cmd_scmver(const struct yuck_cmd_scmver_s argi[static 1U])
{
#if defined WITH_SCMVER
	struct yuck_version_s v[1U];
	struct yuck_version_s ref[1U];
	const char *const reffn = argi->reference_arg;
	const char *const infn = argi->args[0U];
	int rc = 0;

	/* read the reference file before it goes out of fashion */
	if (reffn && yuck_version_read(ref, reffn) < 0 &&
	    /* only be fatal if we actually want to use the reference file */
	    argi->use_reference_flag) {
		error("cannot read reference file `%s'", reffn);
		return 1;
	} else if (reffn == NULL && argi->use_reference_flag) {
		errno = 0, error("\
flag -n|--use-reference requires -r|--reference parameter");
		return 1;
	} else if (!argi->use_reference_flag && yuck_version(v, infn) < 0) {
		if (argi->ignore_noscm_flag) {
			/* allow graceful exit through --ignore-noscm */
			return 0;
		}
		error("cannot determine SCM");
		return 1;
	}

	if (reffn && argi->use_reference_flag) {
		/* must populate v then */
		*v = *ref;
	} else if (reffn && yuck_version_cmp(v, ref)) {
		if (argi->verbose_flag) {
			errno = 0;
			error("scm version differs from reference");
		}
		/* version stamps differ */
		yuck_version_write(argi->reference_arg, v);
		/* reserve exit code 3 for `updated reference file' */
		rc = 3;
	} else if (reffn && !argi->force_flag) {
		/* don't worry about anything then */
		return 0;
	}

	if (infn != NULL && regfilep(infn)) {
		static char _scmvfn[] = P_tmpdir "/" "yscm_XXXXXX";
		static char tmplfn[PATH_MAX];
		char *scmvfn = _scmvfn;

		/* try the local dir first */
		if ((outf = mkftempp(&scmvfn, sizeof(P_tmpdir))) == NULL) {
			error("cannot open intermediate file `%s'", scmvfn);
			rc = 1;
		} else if (find_aux(tmplfn, sizeof(tmplfn),
				    "yuck-scmver.m4") < 0) {
			error("cannot find yuck template for version strings");
			rc = 1;
		} else {
			const char *outfn = argi->output_arg;

			/* write the actual version info */
			rc += wr_version(v, NULL);
			/* and we're finished with the intermediary */
			fclose(outf);
			/* macro massage, vtmpfn is the template file */
			rc = run_m4(outfn, scmvfn, tmplfn, infn, NULL);

			rm_intermediary(scmvfn, argi->keep_flag);
		}
	} else {
		fputs(v->vtag, stdout);
		if (v->scm > YUCK_SCM_TARBALL && v->dist) {
			fputc('.', stdout);
			fputs(yscm_strs[v->scm], stdout);
			fprintf(stdout, "%u.%08x", v->dist, v->rvsn);
		}
		if (v->dirty) {
			fputs(".dirty", stdout);
		}
		fputc('\n', stdout);
	}
	return rc;
#else  /* !WITH_SCMVER */
	fputs("scmver support not built in\n", stderr);
	return argi->cmd == YUCK_CMD_SCMVER;
#endif	/* WITH_SCMVER */
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
	default:
		fputs("\
No valid command specified.\n\
See --help to obtain a list of available commands.\n", stderr);
		rc = 1;
		goto out;
	case YUCK_CMD_GEN:
		if ((rc = cmd_gen((const void*)argi)) < 0) {
			rc = 1;
		}
		break;
	case YUCK_CMD_GENDSL:
		if ((rc = cmd_gendsl((const void*)argi)) < 0) {
			rc = 1;
		}
		break;
	case YUCK_CMD_GENMAN:
		if ((rc = cmd_genman((const void*)argi)) < 0) {
			rc = 1;
		}
		break;
	case YUCK_CMD_SCMVER:
		if ((rc = cmd_scmver((const void*)argi)) < 0) {
			rc = 1;
		}
		break;
	}

out:
	yuck_free(argi);
	return rc;
}
#endif	/* !BOOTSTRAP */

/* yuck.c ends here */
