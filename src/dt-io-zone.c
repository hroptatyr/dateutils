/*** dt-io-zone.c -- abstract from raw zone interface
 *
 * Copyright (C) 2010-2022 Sebastian Freundt
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
#include <string.h>
#include "tzmap.h"
#include "dt-io.h"
#include "dt-io-zone.h"
#include "alist.h"

#if defined TZMAP_DIR
static const char tmdir[] = TZMAP_DIR;
#else  /* !TZMAP_DIR */
static const char tmdir[] = ".";
#endif	/* TZMAP_DIR */
#define TZMAP_SUF	".tzmcc"

static size_t
xstrlncpy(char *restrict dst, size_t dsz, const char *src, size_t ssz)
{
	if (UNLIKELY(!dsz)) {
		return 0U;
	} else if (ssz > dsz) {
		ssz = dsz - 1U;
	}
	memcpy(dst, src, ssz);
	dst[ssz] = '\0';
	return ssz;
}

static size_t
xstrlcpy(char *restrict dst, const char *src, size_t dsz)
{
	size_t ssz = strlen(src);

	if (UNLIKELY(!dsz)) {
		return 0U;
	} else if (ssz > dsz) {
		ssz = dsz - 1U;
	}
	memcpy(dst, src, ssz);
	dst[ssz] = '\0';
	return ssz;
}


/* extended zone handling, tzmaps and stuff */
#if !defined PATH_MAX
# define PATH_MAX	256U
#endif	/* !PATH_MAX */

static struct alist_s zones[1U];
static struct alist_s tzmaps[1U];

static tzmap_t
find_tzmap(const char *mnm, size_t mnz)
{
	static const char tzmap_suffix[] = TZMAP_SUF;
	char tzmfn[PATH_MAX];
	char *tp = tzmfn;
	size_t tz = sizeof(tzmfn);
	const char *p;
	size_t z;

	/* prefer TZMAP_DIR */
	if ((p = getenv("TZMAP_DIR")) != NULL) {
		z = xstrlcpy(tp, p, tz);
	} else {
		z = xstrlncpy(tp, tz, tmdir, sizeof(tmdir) - 1U);
	}
	tp += z, tz -= z;
	*tp++ = '/', tz--;
	if (UNLIKELY(tz < mnz + sizeof(tzmap_suffix))) {
		/* GHSL-2020-090 */
		return NULL;
	}

	/* try and find it the hard way */
	xstrlncpy(tp, tz, mnm, mnz);
	tp += mnz, tz -= mnz;
	xstrlncpy(tp, tz, tzmap_suffix, sizeof(tzmap_suffix) - 1U);

	/* try and open the thing, then try and look up SPEC */
	return tzm_open(tzmfn);
}

static zif_t
__io_zone(const char *spec)
{
	zif_t res;

	/* try looking up SPEC first */
	if ((res = alist_assoc(zones, spec)) == NULL) {
		/* open 'im */
		if ((res = zif_open(spec)) != NULL) {
			/* cache 'im */
			alist_put(zones, spec, res);
		}
	}
	return res;
}

zif_t
dt_io_zone(const char *spec)
{
	char *p;

	if (spec == NULL) {
		/* safety net */
		return NULL;
	}
	/* see if SPEC is a MAP:KEY */
	if ((p = strchr(spec, ':')) != NULL) {
		char tzmfn[PATH_MAX];
		tzmap_t tzm;

		xstrlncpy(tzmfn, sizeof(tzmfn), spec, p - spec);

		/* check tzmaps alist first */
		if ((tzm = alist_assoc(tzmaps, tzmfn)) != NULL) {
			;
		} else if ((tzm = find_tzmap(tzmfn, p - spec)) != NULL) {
			/* cache the instance */
			alist_put(tzmaps, tzmfn, tzm);
		} else {
			error("\
Cannot find `%s" TZMAP_SUF "' in the tzmaps search path\n\
Set TZMAP_DIR environment variable to where " TZMAP_SUF " files reside", tzmfn);
			return NULL;
		}
		/* look up key bit in tzmap and use that if found */
		if ((spec = tzm_find(tzm, ++p)) == NULL) {
			return NULL;
		}
	}
	return __io_zone(spec);
}

void
dt_io_clear_zones(void)
{
	if (tzmaps->data != NULL) {
		for (acons_t c; (c = alist_next(tzmaps)).val;) {
			tzm_close(c.val);
		}
		free_alist(tzmaps);
	}
	if (zones->data != NULL) {
		for (acons_t c; (c = alist_next(zones)).val;) {
			zif_close(c.val);
		}
		free_alist(zones);
	}
	return;
}

/* dt-io-zone.c ends here */
