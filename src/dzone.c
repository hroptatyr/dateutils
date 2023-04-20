/*** dzone.c -- convert date/times between timezones
 *
 * Copyright (C) 2011-2022 Sebastian Freundt
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
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#include "dt-core.h"
#include "dt-io.h"
#include "dt-core-tz-glue.h"
#include "dt-locale.h"
#include "tzraw.h"

struct ztr_s {
	stamp_t trns;
	int offs;
};

const char *prog = "dzone";
static char gbuf[256U];

static const char never[] = "never";
static const char nindi[] = " -> ";
static const char pindi[] = " <- ";


static size_t
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

static struct dt_dt_s
dz_enrichz(struct dt_dt_s d, zif_t zone)
{
/* like dtz_enrichz() but with fix-up for date-only dt's */
	struct dt_dt_s x = d;

	if (UNLIKELY(zone == NULL)) {
		goto out;
	} else if (d.typ == DT_SEXY) {
		goto out;
	} else if (dt_sandwich_only_d_p(d)) {
		static struct dt_t_s mid = {.hms = {.h = 24}};
		dt_make_sandwich(&x, d.d.typ, DT_HMS);
		x.t = mid;
	}
	x = dtz_enrichz(x, zone);

	if (dt_sandwich_only_d_p(d)) {
		/* keep .zdiff and .neg */
		d.neg = x.neg;
		d.zdiff = x.zdiff;
		x = d;
	}
out:
	return x;
}

static int
dz_io_write(struct dt_dt_s d, zif_t zone, const char *name)
{
	static const char fmt[] = "%FT%T%Z\0%F%Z";
	char *restrict bp = gbuf;
	const char *const ep = gbuf + sizeof(gbuf);
	size_t fof = 0U;

	/* pick a format */
	fof += dt_sandwich_only_t_p(d) * 3U;
	fof += dt_sandwich_only_d_p(d) * 8U;

	/* let's go */
	d = dz_enrichz(d, zone);
	bp += dt_strfdt(bp, ep - bp, fmt + fof, d);
	/* append name */
	if (LIKELY(name != NULL)) {
		*bp++ = '\t';
		bp += xstrlcpy(bp, name, ep - bp);
	}
	*bp++ = '\n';
	__io_write(gbuf, bp - gbuf, stdout);
	return (bp > gbuf) - 1;
}

static size_t
dz_strftr(char *restrict buf, size_t bsz, struct ztr_s t)
{
	static const char fmt[] = "%FT%T%Z";
	struct dt_dt_s d = {DT_UNK};

	d.typ = DT_SEXY;
	d.sexy = t.trns + t.offs;
	if (t.offs > 0) {
		d.zdiff = (uint16_t)(t.offs / ZDIFF_RES);
	} else if (t.offs < 0) {
		d.neg = 1;
		d.zdiff = (uint16_t)(-t.offs / ZDIFF_RES);
	}
	return dt_strfdt(buf, bsz, fmt, d);
}

static int
dz_write_nxtr(struct zrng_s r, zif_t z, const char *zn)
{
	char *restrict bp = gbuf;
	const char *const ep = gbuf + sizeof(gbuf);
	size_t ntr = zif_ntrans(z);

	if (r.next >= STAMP_MAX) {
		bp += xstrlcpy(bp, never, bp - ep);
	} else {
		bp += dz_strftr(bp, ep - bp, (struct ztr_s){r.next, r.offs});
	}
	/* append next indicator */
	bp += xstrlcpy(bp, nindi, bp - ep);
	if (r.trno + 1U < ntr) {
		/* thank god there's another one */
		stamp_t zdo = zif_troffs(z, r.trno + 1);

		if (r.next >= STAMP_MAX) {
			goto never;
		}
		bp += dz_strftr(bp, ep - bp, (struct ztr_s){r.next, zdo});
	} else {
	never:
		bp += xstrlcpy(bp, never, bp - ep);
	}

	/* append name */
	if (LIKELY(zn != NULL)) {
		*bp++ = '\t';
		bp += xstrlcpy(bp, zn, ep - bp);
	}
	*bp++ = '\n';
	__io_write(gbuf, bp - gbuf, stdout);
	return (bp > gbuf) - 1;
}

static int
dz_write_prtr(struct zrng_s r, zif_t z, const char *zn)
{
	char *restrict bp = gbuf;
	const char *const ep = gbuf + sizeof(gbuf);

	if (r.trno >= 1) {
		/* there's one before that */
		stamp_t zdo = zif_troffs(z, r.trno - 1);

		bp += dz_strftr(bp, ep - bp, (struct ztr_s){r.prev, zdo});
	} else {
		bp += xstrlcpy(bp, never, bp - ep);
	}
	/* append prev indicator */
	bp += xstrlcpy(bp, pindi, bp - ep);
	if (r.prev <= STAMP_MIN) {
		bp += xstrlcpy(bp, never, bp - ep);
	} else {
		bp += dz_strftr(bp, ep - bp, (struct ztr_s){r.prev, r.offs});
	}

	/* append name */
	if (LIKELY(zn != NULL)) {
		*bp++ = '\t';
		bp += xstrlcpy(bp, zn, ep - bp);
	}
	*bp++ = '\n';
	__io_write(gbuf, bp - gbuf, stdout);
	return (bp > gbuf) - 1;
}


#include "dzone.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	int rc = 0;
	zif_t fromz = NULL;
	char **fmt;
	size_t nfmt;
	/* all them zones to consider */
	struct {
		zif_t zone;
		const char *name;
	} *z = NULL;
	size_t nz = 0U;
	/* all them datetimes to consider */
	struct dt_dt_s *d = NULL;
	size_t nd = 0U;
	bool trnsp;

	if (yuck_parse(argi, argc, argv)) {
		rc = 1;
		goto out;
	} else if (argi->nargs == 0U) {
		error("Need at least a ZONENAME or a DATE/TIME");
		rc = 1;
		goto out;
	}

	if (argi->from_locale_arg) {
		setilocale(argi->from_locale_arg);
	}

	/* try and read the from and to time zones */
	if (argi->from_zone_arg &&
	    (fromz = dt_io_zone(argi->from_zone_arg)) == NULL) {
		error("\
Error: cannot find zone specified in --from-zone: `%s'", argi->from_zone_arg);
		rc = 1;
		goto clear;
	}
	trnsp = argi->next_flag || argi->prev_flag;

	if (argi->base_arg) {
		struct dt_dt_s base = dt_strpdt(argi->base_arg, NULL, NULL);
		dt_set_base(base);
	}

	/* very well then */
	fmt = argi->input_format_args;
	nfmt = argi->input_format_nargs;

	/* initially size the two beef arrays as large as there is input
	 * we'll then sort them by traversing the input args and ass'ing
	 * to the one or the other */
	nz = 0U;
	if (UNLIKELY((z = malloc(argi->nargs * sizeof(*z))) == NULL)) {
		error("failed to allocate space for zone info");
		goto out;
	}
	nd = 0U;
	if (UNLIKELY((d = malloc(argi->nargs * sizeof(*d))) == NULL)) {
		error("failed to allocate space for date/times");
		goto out;
	}

	for (size_t i = 0U; i < argi->nargs; i++) {
		const char *inp = argi->args[i];

		/* try dt_strp'ing the input or assume it's a zone  */
		if (!dt_unk_p(d[nd] = dt_io_strpdt(inp, fmt, nfmt, fromz))) {
			if (UNLIKELY(d[nd].fix) && !argi->quiet_flag) {
				rc = 2;
			}
			nd++;
		} else if ((z[nz].zone = dt_io_zone(inp)) != NULL) {
			z[nz].name = inp;
			nz++;
		} else if (!argi->quiet_flag) {
			/* just bollocks */
			error("\
Cannot use `%s', it does not appear to be a zonename\n\
nor a date/time corresponding to the given input formats", inp);
			rc = 2;
		}
	}
	/* operate with default zone UTC and default time now */
	if (nz == 0U) {
		z[nz].zone = NULL;
		z[nz].name = NULL;
		nz++;
	}
	if (nd == 0U && !trnsp) {
		d[nd++] = dt_datetime((dt_dttyp_t)DT_YMD);
	} else if (nd == 0U) {
		d[nd++] = dt_datetime((dt_dttyp_t)DT_SEXY);
	}

	/* just go through them all now */
	if (LIKELY(!trnsp)) {
		for (size_t i = 0U; !trnsp && i < nd; i++) {
			for (size_t j = 0U; j < nz; j++) {
				dz_io_write(d[i], z[j].zone, z[j].name);
			}
		}
	} else {
		/* otherwise traverse the zones and determine transitions */
		for (size_t i = 0U; i < nd; i++) {
			struct dt_dt_s di = dt_dtconv(DT_SEXY, d[i]);

			for (size_t j = 0U; j < nz; j++) {
				const zif_t zj = z[j].zone;
				const char *zn = z[j].name;
				struct zrng_s r;

				if (UNLIKELY(zj == NULL)) {
					/* don't bother */
					continue;
				}
				/* otherwise find the range */
				r = zif_find_zrng(zj, di.sexy);

				if (argi->next_flag) {
					dz_write_nxtr(r, zj, zn);
				}

				if (argi->prev_flag) {
					dz_write_prtr(r, zj, zn);
				}
			}
		}
	}

clear:
	/* release the zones */
	dt_io_clear_zones();
	/* release those arrays */
	if (LIKELY(z != NULL)) {
		free(z);
	}
	if (LIKELY(d != NULL)) {
		free(d);
	}

	if (argi->from_locale_arg) {
		setilocale(NULL);
	}

out:
	yuck_free(argi);
	return rc;
}

/* dzone.c ends here */
