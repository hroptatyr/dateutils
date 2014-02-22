/*** dzone.c -- convert date/times between timezones
 *
 * Copyright (C) 2011-2014 Sebastian Freundt
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
#include "tzraw.h"

struct ztr_s {
	int32_t trns;
	int32_t offs;
};

const char *prog = "dzone";
static char buf[256U];


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

static int
dz_io_write(struct dt_dt_s d, zif_t zone, const char *name)
{
	static const char fmt[] = "%FT%T%Z";
	char *restrict bp = buf;
	const char *const ep = buf + sizeof(buf);

	if (LIKELY(zone != NULL)) {
		d = dtz_enrichz(d, zone);
	}
	bp += dt_strfdt(bp, ep - bp, fmt, d);
	/* append name */
	if (LIKELY(name != NULL)) {
		*bp++ = '\t';
		bp += xstrlcpy(bp, name, ep - bp);
	}
	*bp++ = '\n';
	__io_write(buf, bp - buf, stdout);
	return (bp > buf) - 1;
}

static int
dz_tr_write(struct ztr_s t, const char *name)
{
	static const char fmt[] = "%FT%T%Z";
	char *restrict bp = buf;
	const char *const ep = buf + sizeof(buf);
	struct dt_dt_s d = dt_dt_initialiser();

	d.typ = DT_SEXY;
	d.sexy = t.trns + t.offs;
	if (t.offs > 0) {
		d.zdiff = (uint16_t)(t.offs / ZDIFF_RES);
	} else if (t.offs < 0) {
		d.neg = 1;
		d.zdiff = (uint16_t)(-t.offs / ZDIFF_RES);
	}
	bp += dt_strfdt(bp, ep - bp, fmt, d);
	/* append name */
	if (LIKELY(name != NULL)) {
		*bp++ = '\t';
		bp += xstrlcpy(bp, name, ep - bp);
	}
	*bp++ = '\n';
	__io_write(buf, bp - buf, stdout);
	return (bp > buf) - 1;
}


#include "dzone.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	int res = 0;
	zif_t fromz = NULL;
	char **fmt;
	size_t nfmt;
	/* all them zones to consider */
	struct {
		zif_t zone;
		const char *name;
	} *z;
	size_t nz;
	/* all them datetimes to consider */
	struct dt_dt_s *d;
	size_t nd;
	bool trnsp = false;

	if (yuck_parse(argi, argc, argv)) {
		res = 1;
		goto out;
	} else if (argi->nargs == 0U) {
		error("Need at least a ZONENAME or a DATE/TIME");
		res = 1;
		goto out;
	}

	/* try and read the from and to time zones */
	if (argi->from_zone_arg) {
		fromz = dt_io_zone(argi->from_zone_arg);
	}
	if (argi->next_flag || argi->prev_flag) {
		trnsp = true;
	}

	/* very well then */
	fmt = argi->input_format_args;
	nfmt = argi->input_format_nargs;

	/* initially size the two beef arrays as large as there is input
	 * we'll then sort them by traversing the input args and ass'ing
	 * to the one or the other */
	nz = 0U;
	z = malloc(argi->nargs * sizeof(*z));
	nd = 0U;
	d = malloc(argi->nargs * sizeof(*d));

	for (size_t i = 0U; i < argi->nargs; i++) {
		const char *inp = argi->args[i];

		/* try dt_strp'ing the input or assume it's a zone  */
		if (!dt_unk_p(d[nd] = dt_io_strpdt(inp, fmt, nfmt, fromz))) {
			nd++;
		} else if ((z[nz].zone = dt_io_zone(inp)) != NULL) {
			z[nz].name = inp;
			nz++;
		} else if (!argi->quiet_flag) {
			/* just bollocks */
			error("\
Cannot use `%s', it does not appear to be a zonename\n\
nor a date/time corresponding to the given input formats", inp);
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
		goto clear;
	}
	/* otherwise traverse the zones and determine transitions */
	for (size_t i = 0U; i < nd; i++) {
		struct dt_dt_s di = dt_dtconv(DT_SEXY, d[i]);

		for (size_t j = 0U; j < nz; j++) {
			const zif_t zj = z[j].zone;
			const char *zn = z[j].name;
			struct zrng_s r = zif_find_zrng(zj, di.sexy);

			if (argi->next_flag && r.next == INT_MIN) {
				printf("never->never\t%s\n", zn);
			} else if (argi->next_flag) {
				dz_tr_write((struct ztr_s){r.next, r.offs}, zn);
			}
			if (argi->prev_flag && r.prev == INT_MIN) {
				printf("never<-never\t%s\n", zn);
			} else if (argi->prev_flag) {
				dz_tr_write((struct ztr_s){r.prev, r.offs}, zn);
			}
		}
	}

clear:
	/* release the zones */
	dt_io_clear_zones();
	/* release those arrays */
	free(z);
	free(d);

	if (argi->from_zone_arg) {
		zif_close(fromz);
	}

out:
	yuck_free(argi);
	return res;
}

/* dzone.c ends here */
