/*** tzraw.c -- reader for olson database zoneinfo files
 *
 * Copyright (C) 2009-2022 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of uterus and dateutils.
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
/* implementation part of tzraw.h */
#if !defined INCLUDED_tzraw_c_
#define INCLUDED_tzraw_c_

#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#if defined MAP_ANON_NEEDS_DARWIN_SOURCE
# define _DARWIN_C_SOURCE
#endif	/* MAP_ANON_NEEDS_DARWIN_SOURCE */
#if defined MAP_ANON_NEEDS_ALL_SOURCE
# define _ALL_SOURCE
#endif	/* MAP_ANON_NEEDS_ALL_SOURCE */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>
#include <assert.h>
#if defined HAVE_TZFILE_H
# include <tzfile.h>
#endif	/* HAVE_TZFILE_H */

/* for be/le conversions */
#include "boops.h"
/* for LIKELY/UNLIKELY/etc. */
#include "nifty.h"
/* me own header, innit */
#include "tzraw.h"
/* for leap corrections */
#include "leap-seconds.h"

#if !defined DEFUN
# define DEFUN
#endif	/* !DEFUN */

#if !defined MAP_ANONYMOUS && defined MAP_ANON
# define MAP_ANONYMOUS	(MAP_ANON)
#elif !defined MAP_ANON
# define MAP_ANON	(0x1000U)
#endif	/* MAP_ANON->MAP_ANONYMOUS */

typedef uint8_t zty_t;
typedef int zof_t;

/* this is tzhead but better */
struct zih_s {
	/* magic */
	char tzh_magic[4];
	/* must be '2' now, as of 2005 */
	char tzh_version[1];
	/* reserved--must be zero */
	char tzh_reserved[15];
	/* number of transition time flags in gmt */
	unsigned char tzh_ttisgmtcnt[4];
	/* number of transition time flags in local time */
	unsigned char tzh_ttisstdcnt[4];
	/* number of recorded leap seconds */
	unsigned char tzh_leapcnt[4];
	/* number of recorded transition times */
	unsigned char tzh_timecnt[4];
	/* number of local time type */
	unsigned char tzh_typecnt[4];
	/* number of abbreviation chars */
	unsigned char tzh_charcnt[4];
};

/* this one must be packed to account for the packed file layout */
struct ztrdtl_s {
	int32_t offs;
	uint8_t dstp;
	uint8_t abbr;
} __attribute__((packed));

/* convenience struct where we copy all the good things into one */
struct zspec_s {
	stamp_t since;
	unsigned int offs:31;
	unsigned int dstp:1;
	char *name;
} __attribute__((packed, aligned(16)));

/* for leap second transitions */
struct zlp_s {
	/* cut-off stamp */
	stamp_t t;
	/* cumulative correction since T */
	int32_t corr;
};

/* leap second support missing as we do our own, see leaps.[ch] */
struct zif_s {
	size_t ntr;
	size_t nty;
	size_t nlp;

	/* NTR transitions */
	stamp_t *trs;
	/* NTR types */
	zty_t *tys;
	/* NTY type array, transition details */
	zof_t *ofs;
	/* leaps */
	struct zlp_s *lps;

	/* for special zones */
	coord_zone_t cz;

	/* zone caching, between PREV and NEXT the offset is OFFS */
	struct zrng_s cache;

	stamp_t data[0] __attribute__((aligned(16)));
};


#if defined TZDIR
static const char tzdir[] = TZDIR;
#else  /* !TZDIR */
static const char tzdir[] = "/usr/share/zoneinfo";
#endif

#define PROT_MEMMAP	PROT_READ | PROT_WRITE
#define MAP_MEMMAP	MAP_PRIVATE | MAP_ANON

/* special zone names */
static const char coord_zones[][4] = {
	"",
	"UTC",
	"TAI",
	"GPS",
};

static inline uint32_t
RDU32(const unsigned char *x)
{
	uint32_t r = 0U;
	r ^= x[0U] << 24U;
	r ^= x[1U] << 16U;
	r ^= x[2U] << 8U;
	r ^= x[3U] << 0;
	return r;
}

static inline int32_t
RDI32(const unsigned char *x)
{
	int32_t r = 0U;
	r ^= x[0U] << 24U;
	r ^= x[1U] << 16U;
	r ^= x[2U] << 8U;
	r ^= x[3U] << 0;
	return r;
}

static inline int64_t
RDI64(const unsigned char *x)
{
	int64_t r = 0U;
	r ^= (uint64_t)x[0U] << 56U;
	r ^= (uint64_t)x[1U] << 48U;
	r ^= (uint64_t)x[2U] << 40U;
	r ^= (uint64_t)x[3U] << 32U;
	r ^= (uint64_t)x[4U] << 24U;
	r ^= (uint64_t)x[5U] << 16U;
	r ^= (uint64_t)x[6U] << 8U;
	r ^= (uint64_t)x[7U] << 0;
	return r;
}


/**
 * Return the transition time stamp of the N-th transition in Z. */
static inline stamp_t
zif_trans(const struct zif_s z[static 1U], int n)
{
	size_t ntr = z->ntr;

	if (UNLIKELY(!ntr || n < 0)) {
		/* return earliest possible stamp */
		return STAMP_MIN;
	} else if (UNLIKELY(n >= (ssize_t)ntr)) {
		/* return last known stamp */
		return z->trs[ntr - 1U];
	}
	/* otherwise return n-th stamp */
	return z->trs[n];
}

/**
 * Return the transition type index of the N-th transition in Z. */
static inline uint8_t
_zif_type(const struct zif_s z[static 1U], int n)
{
	size_t ntr = z->ntr;

	if (UNLIKELY(!ntr || n < 0)) {
		/* return unknown type */
		return 0;
	} else if (UNLIKELY(n >= (ssize_t)ntr)) {
		/* return last known type */
		return z->tys[ntr - 1U];
	}
	/* otherwise return n-th type */
	return z->tys[n];
}

/**
 * Return the gmt offset after the N-th transition in Z. */
static inline int
_zif_troffs(const struct zif_s z[static 1U], int n)
{
/* no bound check! */
	uint8_t idx = _zif_type(z, n);
	return z->ofs[idx];
}

/**
 * Return the gmt offset after the N-th transition in Z. */
DEFUN int
zif_troffs(zif_t z, int n)
{
/* no bound check! */
	uint8_t idx = _zif_type(z, n);
	return z->ofs[idx];
}

/**
 * Return the transition time stamp of the N-th transition in Z. */
DEFUN inline size_t
zif_ntrans(zif_t z)
{
	return z->ntr;
}


static coord_zone_t
coord_zone(const char *zone)
{
	for (coord_zone_t i = TZCZ_UTC; i < TZCZ_NZONE; i++) {
		if (strcmp(zone, coord_zones[i]) == 0) {
			return i;
		}
	}
	return TZCZ_UNK;
}

static int
__open_zif(const char *file)
{
	size_t len;

	if (UNLIKELY(file == NULL || file[0] == '\0')) {
		return -1;
	} else if (UNLIKELY((len = strlen(file)) >= 3071U)) {
		return -1;
	} else if (file[0] != '/') {
		/* not an absolute file name */
		size_t tzd_len = sizeof(tzdir) - 1;
		char new[tzd_len + 1U + len + 1U];
		char *tmp = new + tzd_len;

		memcpy(new, tzdir, tzd_len);
		*tmp++ = '/';
		memcpy(tmp, file, len + 1U);
		return open(new, O_RDONLY, 0644);
	}
	/* absolute file name, just try with that one then */
	return open(file, O_RDONLY, 0644);
}


DEFUN void
zif_close(zif_t z)
{
	if (!z->cz) {
		free(z);
	}
	return;
}

DEFUN zif_t
zif_open(const char *file)
{
	struct stat st;
	coord_zone_t cz;
	int fd;
	struct zif_s tmp;
	struct zif_s *res;
	unsigned char *map;
	const unsigned char *hdr, *beef;
	size_t real_ntr = 0U;

	/* check for special time zones */
	if ((cz = coord_zone(file)) > TZCZ_UNK) {
		/* we used to go to the UTC file hoping they're coordinated well,
		 * however seeing as we're dealing with leap seconds ourself we
		 * can just skip the file reading and return a static instance */
		static struct zif_s coord_zifs[] = {
			[TZCZ_UTC] = {.cz = TZCZ_UTC, .ofs = (void*)&coord_zifs},
			[TZCZ_TAI] = {.cz = TZCZ_TAI, .ofs = (void*)&coord_zifs},
			[TZCZ_GPS] = {.cz = TZCZ_GPS, .ofs = (void*)&coord_zifs},
		};
		return coord_zifs + cz;
	}

	if (UNLIKELY((fd = __open_zif(file)) < STDIN_FILENO)) {
		return NULL;
	} else if (fstat(fd, &st) < 0) {
		goto cout;
	} else if (st.st_size <= 20) {
		goto cout;
	}

	map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED) {
		goto cout;
	}

	if (UNLIKELY(memcmp(map, TZ_MAGIC, 4U))) {
		goto unmp;
	}
	/* read hdr with undefined alignment */
	hdr = map;
	switch (hdr[offsetof(struct zih_s, tzh_version)]) {
		const unsigned char *hds;
	case '2':
		/*@fallthrough@*/
	case '3':
		hds = hdr;
		tmp.nlp = RDU32(hdr + offsetof(struct zih_s, tzh_leapcnt));
		tmp.ntr = RDU32(hdr + offsetof(struct zih_s, tzh_timecnt));
		tmp.nty = RDU32(hdr + offsetof(struct zih_s, tzh_typecnt));
		hds += sizeof(struct zih_s);
		hds += tmp.ntr * 4U;
		hds += tmp.ntr;
		hds += tmp.nty * (4U + 1U + 1U);
		hds += RDU32(hdr + offsetof(struct zih_s, tzh_charcnt));
		hds += tmp.nlp * (4U + 4U);
		hds += RDU32(hdr + offsetof(struct zih_s, tzh_ttisstdcnt));
		hds += RDU32(hdr + offsetof(struct zih_s, tzh_ttisgmtcnt));

		if (UNLIKELY(memcmp(hds, TZ_MAGIC, 4U))) {
			goto unmp;
		}
		hdr = hds;
	case '\0':
		tmp.nlp = RDU32(hdr + offsetof(struct zih_s, tzh_leapcnt));
		tmp.ntr = RDU32(hdr + offsetof(struct zih_s, tzh_timecnt));
		tmp.nty = RDU32(hdr + offsetof(struct zih_s, tzh_typecnt));
		break;
	default:
		goto unmp;
	}
	/* alloc space, don't read leaps just transitions and types */
	res = malloc(sizeof(*res) +
		     tmp.ntr * sizeof(*res->trs) +
		     tmp.nty * sizeof(*res->ofs) +
		     tmp.ntr * sizeof(*res->tys) +
		     0);
	if (UNLIKELY(res == NULL)) {
		goto unmp;
	}
	/* otherwise fill */
	*res = tmp;
	res->trs = (stamp_t*)(res->data + 0);
	res->ofs = (zof_t*)(res->trs + tmp.ntr);
	res->tys = (zty_t*)(res->ofs + tmp.nty);
	res->lps = NULL;
	res->cz = cz;
	res->cache = (struct zrng_s){0};
	/* copy data (and bring to host order) */
	beef = hdr + sizeof(struct zih_s);
	switch (hdr[offsetof(struct zih_s, tzh_version)]) {
	case '2':
		/*@fallthrough@*/
	case '3':
		for (size_t i = 0U; i < tmp.ntr; i++) {
			res->trs[i] = RDI64(beef + 8U * i);
		}
		beef += 8U * tmp.ntr;
		memcpy(res->tys, beef, tmp.ntr);
		beef += tmp.ntr;
		for (size_t i = 0U; i < tmp.nty; i++) {
			res->ofs[i] = RDI32(beef + 6U * i);
		}
		break;
	case '\0':
		for (size_t i = 0U; i < tmp.ntr; i++) {
			res->trs[i] = RDI32(beef + 4U * i);
		}
		beef += 4U * tmp.ntr;
		memcpy(res->tys, beef, tmp.ntr);
		beef += tmp.ntr;
		for (size_t i = 0U; i < tmp.nty; i++) {
			res->ofs[i] = RDI32(beef + 6U * i);
		}
		break;
	}
	/* clean up */
	munmap(map, st.st_size);
	close(fd);
	/* compactify, we disallow transitions to the same type */
	real_ntr += res->ntr > 0U;
	for (size_t i = 1U; i < res->ntr; i++) {
		if (res->tys[i - 1U] != res->tys[i - 0U]) {
			res->trs[real_ntr] = res->trs[i];
			res->tys[real_ntr] = res->tys[i];
			real_ntr++;
		}
	}
	res->ntr = real_ntr;
	return res;
unmp:
	munmap(map, st.st_size);
cout:
	close(fd);
	return NULL;
}

DEFUN zif_t
zif_copy(zif_t z)
{
/* copy Z into a newly allocated zif_t object
 * if applicable also perform byte-order conversions */
	struct zif_s *res;

	res = malloc(sizeof(*z) +
		     z->ntr * sizeof(*z->trs) +
		     z->nty * sizeof(*z->ofs) +
		     z->ntr * sizeof(*z->tys) +
		     0);

	if (UNLIKELY(res == NULL)) {
		/* no need to bother */
		return NULL;
	}
	/* initialise */
	res->ntr = z->ntr;
	res->nty = z->nty;
	res->nlp = z->nlp;
	res->trs = (stamp_t*)(res->data + 0);
	res->ofs = (zof_t*)(res->trs + z->ntr);
	res->tys = (zty_t*)(res->ofs + z->nty);
	res->lps = NULL;
	res->cz = z->cz;
	res->cache = (struct zrng_s){0};
	/* ... and copy */
	memcpy(res->trs, z->trs, z->ntr * sizeof(*z->trs));
	memcpy(res->ofs, z->ofs, z->nty * sizeof(*z->ofs));
	memcpy(res->tys, z->tys, z->ntr * sizeof(*z->tys));
	return res;
}


/* for leap corrections */
#include "leap-seconds.def"

static inline int
__find_trno(const struct zif_s z[static 1U], stamp_t t, int min, int max)
{
/* find the last transition before T, T is expected to be UTC
 * if T is before any known transition return -1 */
	if (UNLIKELY(max == 0)) {
		/* special case */
		return -1;
	} else if (UNLIKELY(t < zif_trans(z, min))) {
		return -1;
	} else if (UNLIKELY(t > zif_trans(z, max))) {
		return max - 1;
	}

	do {
		stamp_t tl, tu;
		int this = (min + max) / 2;

		tl = zif_trans(z, this);
		tu = zif_trans(z, this + 1);

		if (t >= tl && t < tu) {
			/* found him */
			return this;
		} else if (t >= tu) {
			min = this;
		} else if (t < tl) {
			max = this;
		}
	} while (true);
	/* not reached */
}

DEFUN inline int
zif_find_trans(zif_t z, stamp_t t)
{
/* find the last transition before T, T is expected to be UTC
 * if T is before any known transition return -1 */
	int max = z->ntr;
	int min = 0;

	return __find_trno(z, t, min, max);
}

static struct zrng_s
__find_zrng(const struct zif_s z[static 1U], stamp_t t, int min, int max)
{
	struct zrng_s res;
	int trno;

	trno = __find_trno(z, t, min, max);
	res.prev = zif_trans(z, trno);
	if (UNLIKELY(trno <= 0 && t < res.prev)) {
		res.trno = 0U;
		res.prev = STAMP_MIN;
		/* assume the first offset has always been there */
		res.next = res.prev;
	} else if (UNLIKELY(trno < 0)) {
		/* special case where no transitions are recorded */
		res.trno = 0U;
		res.prev = STAMP_MIN;
		res.next = STAMP_MAX;
	} else {
		res.trno = (uint8_t)trno;
		if (LIKELY(trno + 1U < z->ntr)) {
			res.next = zif_trans(z, trno + 1U);
		} else {
			res.next = STAMP_MAX;
		}
	}
	res.offs = _zif_troffs(z, res.trno);
	return res;
}

DEFUN struct zrng_s
zif_find_zrng(zif_t z, stamp_t t)
{
/* find the last transition before time, time is expected to be UTC */
	int max = z->ntr;
	int min = 0;

	return __find_zrng(z, t, min, max);
}

static stamp_t
__tai_offs(stamp_t t)
{
	/* difference of TAI and UTC at epoch instant */
	zidx_t zi = leaps_before_si32(leaps_s, nleaps_corr, t);

	return leaps_corr[zi];
}

static stamp_t
__gps_offs(stamp_t t)
{
/* TAI - GPS = 19 on 1980-01-06, so use that identity here */
	const stamp_t gps_offs_epoch = 19;
	if (UNLIKELY(t < 315964800)) {
		return 0;
	}
	return __tai_offs(t) - gps_offs_epoch;
}

static stamp_t
__offs(struct zif_s z[static 1U], stamp_t t)
{
/* return the offset of T in Z and cache the result. */
	int min;
	size_t max;

	switch (z->cz) {
	default:
	case TZCZ_UNK:
		break;
	case TZCZ_UTC:
		return 0;
	case TZCZ_TAI:
		return __tai_offs(t);
	case TZCZ_GPS:
		return __gps_offs(t);
	}

	/* use the classic code */
	if (LIKELY(t >= z->cache.prev && t < z->cache.next)) {
		/* use the cached offset */
		return z->cache.offs;
	} else if (t >= z->cache.next) {
		min = z->cache.trno + 1;
		max = z->ntr;
	} else if (t < z->cache.prev) {
		max = z->cache.trno;
		min = 0;
	} else {
		/* we shouldn't end up here at all */
		min = 0;
		max = 0;
	}
	return (z->cache = __find_zrng(z, t, min, max)).offs;
}

DEFUN stamp_t
zif_utc_time(zif_t z, stamp_t t)
{
/* here's the setup, given t in local time, we denote the corresponding
 * UTC time by t' = t - x' where x' is the true offset
 * however, since we do not know the offset in advance, we have to solve
 * for an estimate of the offset x:
 * t - x + x' = t, or equivalently t - x = t' or as a root finding problem
 * x' - x = 0.
 * To make this iterative we just solve:
 * x_{i+1} - x_i = 0, where x_{i+1} = o(t - x_i) and o maps a given
 * time stamp to an offset. */
/* make me use the cache please! */
	/* let's go */
	stamp_t xi = 0;
	stamp_t xj;
	stamp_t old = -1;

	/* jump off the cliff if Z is nought */
	if (UNLIKELY(z == NULL)) {
		return t;
	}

	while ((xj = __offs(z, t - xi)) != xi && xi != old) {
		old = xi = xj;
	}
	return t - xj;
}

/* convert utc to local */
DEFUN stamp_t
zif_local_time(zif_t z, stamp_t t)
{
	/* jump off the cliff if Z is nought */
	if (UNLIKELY(z == NULL)) {
		return t;
	}
	return t + __offs(z, t);
}

#endif	/* INCLUDED_tzraw_c_ */

#if defined STANDALONE
#include <stdio.h>

int
main(int argc, char *argv[])
{
	int rc = 0;

	for (int i = 1; i < argc; i++) {
		zif_t z = zif_open(argv[i]);

		if (z == NULL) {
			rc++;
			continue;
		}

		puts(argv[i]);
		printf("  ntr\t%zu\n", z->ntr);
		printf("  nty\t%zu\n", z->nty);
		printf("  nlp\t%zu\n", z->nlp);

		for (size_t j = 0U; j < z->ntr; j++) {
			printf("    tr[%zu]\t%lld\t%hhu\n", j, z->trs[j], z->tys[j]);
		}
		for (size_t j = 0U; j < z->nty; j++) {
			printf("    of[%zu]\t%d\n", j, z->ofs[j]);
		}
		zif_close(z);
	}
	return rc;
}
#endif	/* STANDALONE */
/* tzraw.c ends here */
