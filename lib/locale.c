/*** locale.c -- locale light
 *
 * Copyright (C) 2015-2016 Sebastian Freundt
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
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "locale.h"
#include "date-core.h"
#include "date-core-strpf.h"
#include "nifty.h"

#if defined LOCALE_FILE
static const char locfn[] = LOCALE_FILE;
#else  /* !LOCALE_FILE */
static const char ldir[] = "locale";
#endif	/* LOCALE_FILE */

struct lst_s {
	const char *s[GREG_MONTHS_P_YEAR + 1U];
	char str[];
};


static inline __attribute__((unused)) void*
deconst(const void *cp)
{
	union {
		const void *c;
		void *p;
	} tmp = {cp};
	return tmp.p;
}

static char*
xmemmem(const char *hay, const size_t hayz, const char *ndl, const size_t ndlz)
{
	const char *const eoh = hay + hayz;
	const char *const eon = ndl + ndlz;
	const char *hp;
	const char *np;
	const char *cand;
	unsigned int hsum;
	unsigned int nsum;
	unsigned int eqp;

	/* trivial checks first
         * a 0-sized needle is defined to be found anywhere in haystack
         * then run strchr() to find a candidate in HAYSTACK (i.e. a portion
         * that happens to begin with *NEEDLE) */
	if (ndlz == 0UL) {
		return deconst(hay);
	} else if ((hay = memchr(hay, *ndl, hayz)) == NULL) {
		/* trivial */
		return NULL;
	}

	/* First characters of haystack and needle are the same now. Both are
	 * guaranteed to be at least one character long.  Now computes the sum
	 * of characters values of needle together with the sum of the first
	 * needle_len characters of haystack. */
	for (hp = hay + 1U, np = ndl + 1U, hsum = *hay, nsum = *hay, eqp = 1U;
	     hp < eoh && np < eon;
	     hsum ^= *hp, nsum ^= *np, eqp &= *hp == *np, hp++, np++);

	/* HP now references the (NZ + 1)-th character. */
	if (np < eon) {
		/* haystack is smaller than needle, :O */
		return NULL;
	} else if (eqp) {
		/* found a match */
		return deconst(hay);
	}

	/* now loop through the rest of haystack,
	 * updating the sum iteratively */
	for (cand = hay; hp < eoh; hp++) {
		hsum ^= *cand++;
		hsum ^= *hp;

		/* Since the sum of the characters is already known to be
		 * equal at that point, it is enough to check just NZ - 1
		 * characters for equality,
		 * also CAND is by design < HP, so no need for range checks */
		if (hsum == nsum && memcmp(cand, ndl, ndlz - 1U) == 0) {
			return deconst(cand);
		}
	}
	return NULL;
}

static struct lst_s*
tokenise(const char *ln, size_t lz)
{
/* we expect \t separation */
	struct lst_s *r;

	if (UNLIKELY((r = malloc(sizeof(*r) + lz)) == NULL)) {
		return NULL;
	}
	/* no replacement for miraclemonth and miracleday here */
	r->s[0U] = r->str + lz - 1U;
	r->s[1U] = r->str;
	memcpy(r->str, ln, lz);
	for (size_t i = 0U, j = 1U; i < lz && j < countof(r->s); i++) {
		/* just map all ascii ctrl characters to NUL */
		r->str[i] &= (char)(((unsigned char)r->str[i] < ' ') - 1U);
		if (UNLIKELY(!r->str[i])) {
			r->s[j++] = r->str + i + 1U;
		}
	}
	return r;
}

static void
snarf_ln(const char *buf, size_t bsz)
{
	const char *bp;
	const char *ep;
	struct lst_s *x;
	const char **old;

	/* first one */
	bp = buf;
	if (UNLIKELY(!(ep = memchr(bp, '\n', bsz - (bp - buf))))) {
		return;
	} else if (UNLIKELY((x = tokenise(bp, ++ep - bp)) == NULL)) {
		return;
	}
	/* got him */
	old = __strp_set_abbr_wday(x->s);
	if (old) {
		free(deconst(old));
	}

	/* and again */
	bp = ep;
	if (UNLIKELY(!(ep = memchr(bp, '\n', bsz - (bp - buf))))) {
		return;
	} else if (UNLIKELY((x = tokenise(bp, ++ep - bp)) == NULL)) {
		return;
	}
	/* got him */
	old = __strp_set_long_wday(x->s);
	if (old) {
		free(deconst(old));
	}

	/* two to go */
	bp = ep;
	if (UNLIKELY(!(ep = memchr(bp, '\n', bsz - (bp - buf))))) {
		return;
	} else if (UNLIKELY((x = tokenise(bp, ++ep - bp)) == NULL)) {
		return;
	}
	/* got him */
	old = __strp_set_abbr_mon(x->s);
	if (old) {
		free(deconst(old));
	}

	/* just one more */
	bp = ep;
	if (UNLIKELY(!(ep = memchr(bp, '\n', bsz - (bp - buf))))) {
		return;
	} else if (UNLIKELY((x = tokenise(bp, ++ep - bp)) == NULL)) {
		return;
	}
	/* got him */
	old = __strp_set_long_mon(x->s);
	if (old) {
		free(deconst(old));
	}
	return;
}


int
setilocale(const char *ln)
{
	struct stat st[1U];
	const char *fn;
	size_t lz;
	ssize_t fz;
	int fd;
	int rc = 0;

	if (UNLIKELY(ln == NULL)) {
		/* reset to defaults */
		const char **old;

		if ((old = __strp_set_abbr_wday(NULL))) {
			free(deconst(old));
		}
		if ((old = __strp_set_long_wday(NULL))) {
			free(deconst(old));
		}
		if ((old = __strp_set_abbr_mon(NULL))) {
			free(deconst(old));
		}
		if ((old = __strp_set_long_mon(NULL))) {
			free(deconst(old));
		}
		return 0;
	}
	/* otherwise obtain length */
	lz = strlen(ln);

	/* we shall assume locale file is LOCALE_FILE */
	fn = getenv("LOCALE_FILE") ?: locfn;

	if ((fd = open(fn, O_RDONLY)) < 0) {
		/* buggery */
		return -1;
	} else if (UNLIKELY(fstat(fd, st) < 0)) {
		/* knew it! */
		rc = -1;
		goto clo;
	} else if (UNLIKELY((fz = st->st_size) <= 0)) {
		/* it's not our lucky day today */
		rc = -1;
		goto clo;
	}
	/* map him entirely */
	with (const char *m = mmap(NULL, fz, PROT_READ, MAP_SHARED, fd, 0)) {
		const char *l;

		if (UNLIKELY(m == MAP_FAILED)) {
			/* good one */
			rc = -1;
			goto clo;
		}

		/* none of the locales should be a prefix to another */
		if (LIKELY((l = xmemmem(m, fz, ln, lz)) != NULL &&
			   l[lz++] == '\n')) {
			/* ... so we've found the one match
			 * first line is abday
			 * second is day
			 * third is abmon
			 * fourth is mon */
			snarf_ln(l + lz, fz - (l + lz - m));
		}

		munmap(deconst(m), fz);
	}
clo:
	close(fd);
	return rc;
}

/* locale.c ends here */
