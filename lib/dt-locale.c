/*** locale.c -- locale light
 *
 * Copyright (C) 2015-2022 Sebastian Freundt
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
#include "dt-locale.h"
#include "date-core.h"
#include "date-core-strpf.h"
#include "nifty.h"

#if defined LOCALE_FILE
static const char locfn[] = LOCALE_FILE;
#else  /* !LOCALE_FILE */
static const char locfn[] = "locale";
#endif	/* LOCALE_FILE */

#if !defined DEFUN
# define DEFUN
#endif	/* !DEFUN */
#if !defined DEFVAR
# define DEFVAR
#endif	/* !DEFVAR */

struct lst_s {
	const char *s[GREG_MONTHS_P_YEAR + 2U];
	size_t min;
	size_t max;
	char str[];
};

struct loc_s {
	struct lst_s *long_wday;
	struct lst_s *abbr_wday;
	struct lst_s *long_mon;
	struct lst_s *abbr_mon;
};

static const char *__long_wday[] = {
	"Miracleday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
	"Sunday",
};
static const struct strprng_s __rlong_wday = {6, 9};
DEFVAR const char **dut_long_wday = __long_wday;
DEFVAR const char **duf_long_wday = __long_wday;
DEFVAR const ssize_t dut_nlong_wday = countof(__long_wday);
DEFVAR struct strprng_s dut_rlong_wday = {6, 9};

static const char *__abbr_wday[] = {
	"Mir", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun",
};
static const struct strprng_s __rabbr_wday = {3, 3};
DEFVAR const char **dut_abbr_wday = __abbr_wday;
DEFVAR const char **duf_abbr_wday = __abbr_wday;
DEFVAR const ssize_t dut_nabbr_wday = countof(__abbr_wday);
DEFVAR struct strprng_s dut_rabbr_wday = {3, 3};

static const char __abab_wday[] = "XMTWRFAS";
DEFVAR const char *dut_abab_wday = __abab_wday;
DEFVAR const ssize_t dut_nabab_wday = countof(__abab_wday);

static const char *__long_mon[] = {
	"Miraculary",
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December",
};
static const struct strprng_s __rlong_mon = {3, 9};
DEFVAR const char **dut_long_mon = __long_mon;
DEFVAR const char **duf_long_mon = __long_mon;
DEFVAR const ssize_t dut_nlong_mon = countof(__long_mon);
DEFVAR struct strprng_s dut_rlong_mon = {3, 9};

static const char *__abbr_mon[] = {
	"Mir",
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
};
static const struct strprng_s __rabbr_mon = {3, 3};
DEFVAR const char **dut_abbr_mon = __abbr_mon;
DEFVAR const char **duf_abbr_mon = __abbr_mon;
DEFVAR const ssize_t dut_nabbr_mon = countof(__abbr_mon);
DEFVAR struct strprng_s dut_rabbr_mon = {3, 3};

/* futures expiry codes, how convenient */
static const char __abab_mon[] = "_FGHJKMNQUVXZ";
DEFVAR const char *dut_abab_mon = __abab_mon;
DEFVAR const ssize_t dut_nabab_mon = countof(__abab_mon);


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


/* locale business */
static inline void
__strp_reset_long_wday(void)
{
	if (dut_long_wday != __long_wday) {
		free(deconst(dut_long_wday));
	}
	dut_long_wday = __long_wday;
	dut_rlong_wday = __rlong_wday;
	return;
}

static inline void
__strp_reset_abbr_wday(void)
{
	if (dut_abbr_wday != __abbr_wday) {
		free(deconst(dut_abbr_wday));
	}
	dut_abbr_wday = __abbr_wday;
	dut_rabbr_wday = __rabbr_wday;
	return;
}

static inline void
__strp_reset_long_mon(void)
{
	if (dut_long_mon != __long_mon) {
		free(deconst(dut_long_mon));
	}
	dut_long_mon = __long_mon;
	dut_rlong_mon = __rlong_mon;
	return;
}

static inline void
__strp_reset_abbr_mon(void)
{
	if (dut_abbr_mon != __abbr_mon) {
		free(deconst(dut_abbr_mon));
	}
	dut_abbr_mon = __abbr_mon;
	dut_rabbr_mon = __rabbr_mon;
	return;
}

static void
__strp_set_long_wday(struct lst_s *new)
{
	__strp_reset_long_wday();
	dut_long_wday = new->s;
	dut_rlong_wday = (struct strprng_s){new->min, new->max};
	return;
}

static void
__strp_set_abbr_wday(struct lst_s *new)
{
	__strp_reset_abbr_wday();
	dut_abbr_wday = new->s;
	dut_rabbr_wday = (struct strprng_s){new->min, new->max};
	return;
}

static void
__strp_set_long_mon(struct lst_s *new)
{
	__strp_reset_long_mon();
	dut_long_mon = new->s;
	dut_rlong_mon = (struct strprng_s){new->min, new->max};
	return;
}

static void
__strp_set_abbr_mon(struct lst_s *new)
{
	__strp_reset_abbr_mon();
	dut_abbr_mon = new->s;
	dut_rabbr_mon = (struct strprng_s){new->min, new->max};
	return;
}

static inline void
__strf_reset_long_wday(void)
{
	if (duf_long_wday != __long_wday) {
		free(deconst(duf_long_wday));
	}
	duf_long_wday = __long_wday;
	return;
}

static inline void
__strf_reset_abbr_wday(void)
{
	if (duf_abbr_wday != __abbr_wday) {
		free(deconst(duf_abbr_wday));
	}
	duf_abbr_wday = __abbr_wday;
	return;
}

static inline void
__strf_reset_long_mon(void)
{
	if (duf_long_mon != __long_mon) {
		free(deconst(duf_long_mon));
	}
	duf_long_mon = __long_mon;
	return;
}

static inline void
__strf_reset_abbr_mon(void)
{
	if (dut_abbr_mon != __abbr_mon) {
		free(deconst(duf_abbr_mon));
	}
	duf_abbr_mon = __abbr_mon;
	return;
}

static void
__strf_set_long_wday(struct lst_s *new)
{
	__strf_reset_long_wday();
	duf_long_wday = new->s;
	return;
}

static void
__strf_set_abbr_wday(struct lst_s *new)
{
	__strp_reset_abbr_wday();
	duf_abbr_wday = new->s;
	return;
}

static void
__strf_set_long_mon(struct lst_s *new)
{
	__strp_reset_long_mon();
	duf_long_mon = new->s;
	return;
}

static void
__strf_set_abbr_mon(struct lst_s *new)
{
	__strp_reset_abbr_mon();
	duf_abbr_mon = new->s;
	return;
}

static void
reset_il(void)
{
	__strp_reset_long_wday();
	__strp_reset_abbr_wday();
	__strp_reset_long_mon();
	__strp_reset_abbr_mon();
	return;
}

static void
reset_fl(void)
{
	__strf_reset_long_wday();
	__strf_reset_abbr_wday();
	__strf_reset_long_mon();
	__strf_reset_abbr_mon();
	return;
}

static void
set_il(struct loc_s l)
{
	__strp_set_long_wday(l.long_wday);
	__strp_set_abbr_wday(l.abbr_wday);
	__strp_set_long_mon(l.long_mon);
	__strp_set_abbr_mon(l.abbr_mon);
	return;
}

static void
set_fl(struct loc_s l)
{
	__strf_set_long_wday(l.long_wday);
	__strf_set_abbr_wday(l.abbr_wday);
	__strf_set_long_mon(l.long_mon);
	__strf_set_abbr_mon(l.abbr_mon);
	return;
}


static struct lst_s*
tokenise(const char *ln, size_t lz)
{
/* we expect \t separation */
	struct lst_s *r;

	if (UNLIKELY((r = malloc(sizeof(*r) + lz)) == NULL)) {
		return NULL;
	}
	/* just have him point to something */
	r->s[1U] = r->str;
	r->min = -1ULL;
	r->max = 0ULL;
	memcpy(r->str, ln, lz);
	for (size_t i = 0U, j = 2U, o = 0U; i < lz && j < countof(r->s); i++) {
		/* just map all ascii ctrl characters to NUL */
		r->str[i] &= (char)(((unsigned char)r->str[i] < ' ') - 1U);
		if (UNLIKELY(!r->str[i])) {
			const size_t len = i - o;
			r->s[j++] = r->str + (o = i + 1U);
			if (len > r->max) {
				r->max = len;
			}
			if (len < r->min) {
				r->min = len;
			}
		}
	}
	r->s[0U] = r->s[13U];
	return r;
}

static int
snarf_ln(struct loc_s *restrict tgt, const char *buf, size_t bsz)
{
	const char *bp;
	const char *ep;
	struct lst_s *x;
	struct loc_s r;

	/* first one */
	bp = buf;
	if (UNLIKELY(!(ep = memchr(bp, '\n', bsz - (bp - buf))))) {
		goto uhoh;
	} else if (UNLIKELY((x = tokenise(bp, ++ep - bp)) == NULL)) {
		goto uhoh;
	}
	/* got him */
	r.abbr_wday = x;

	/* and again */
	bp = ep;
	if (UNLIKELY(!(ep = memchr(bp, '\n', bsz - (bp - buf))))) {
		goto uhoh;
	} else if (UNLIKELY((x = tokenise(bp, ++ep - bp)) == NULL)) {
		goto uhoh;
	}
	/* got him */
	r.long_wday = x;

	/* two to go */
	bp = ep;
	if (UNLIKELY(!(ep = memchr(bp, '\n', bsz - (bp - buf))))) {
		goto uhoh;
	} else if (UNLIKELY((x = tokenise(bp, ++ep - bp)) == NULL)) {
		goto uhoh;
	}
	/* got him */
	r.abbr_mon = x;

	/* just one more */
	bp = ep;
	if (UNLIKELY(!(ep = memchr(bp, '\n', bsz - (bp - buf))))) {
		goto uhoh;
	} else if (UNLIKELY((x = tokenise(bp, ++ep - bp)) == NULL)) {
		goto uhoh;
	}
	/* got him */
	r.long_mon = x;
	*tgt = r;
	return 0;

uhoh:
	if (r.long_wday) {
		free(r.long_wday);
	}
	if (r.abbr_wday) {
		free(r.abbr_wday);
	}
	if (r.long_mon) {
		free(r.long_mon);
	}
	if (r.abbr_mon) {
		free(r.abbr_mon);
	}
	return -1;
}

static int
__setlocale(const char *ln, size_t lz, void(*setf)(struct loc_s))
{
	struct stat st[1U];
	const char *fn;
	ssize_t fz;
	int fd;
	int rc = 0;

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
		struct loc_s loc;

		if (UNLIKELY(m == MAP_FAILED)) {
			/* good one */
			rc = -1;
			goto clo;
		}

		/* none of the locales should be a prefix to another */
		if (UNLIKELY((l = xmemmem(m, fz, ln, lz)) == NULL)) {
			;
		} else if (UNLIKELY(l[lz++] != '\n')) {
			;
		} else if (snarf_ln(&loc, l + lz, fz - (l + lz - m)) < 0) {
			/* ... so we've found the one match
			 * but reading the locale lines went pearshaped */
			;
		} else {
			/* finally set them */
			setf(loc);
		}

		munmap(deconst(m), fz);
	}
clo:
	close(fd);
	return rc;
}


int
setilocale(const char *ln)
{
	size_t lz;

	if (UNLIKELY(ln == NULL || !(lz = strlen(ln)))) {
		reset_il();
		return 0;
	}

	return __setlocale(ln, lz, set_il);
}

int
setflocale(const char *ln)
{
	size_t lz;

	if (UNLIKELY(ln == NULL || !(lz = strlen(ln)))) {
		reset_fl();
		return 0;
	}

	return __setlocale(ln, lz, set_fl);
}

/* locale.c ends here */
