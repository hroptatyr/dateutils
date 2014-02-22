/*** tzraw.c -- reader for olson database zoneinfo files
 *
 * Copyright (C) 2009, 2012 Sebastian Freundt
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
#include "leapseconds.h"

#if !defined DEFUN
# define DEFUN
#endif	/* !DEFUN */

#if !defined MAP_ANONYMOUS && defined MAP_ANON
# define MAP_ANONYMOUS	(MAP_ANON)
#endif	/* MAP_ANON->MAP_ANONYMOUS */

typedef struct zih_s *zih_t;
typedef int32_t *ztr_t;
typedef uint8_t *zty_t;
typedef struct ztrdtl_s *ztrdtl_t;
typedef char *znam_t;

/* this is tzhead but better */
struct zih_s {
	/* magic */
	char tzh_magic[4];
	/* must be '2' now, as of 2005 */
	char tzh_version[1];
	/* reserved--must be zero */
	char tzh_reserved[15];
	/* number of transition time flags in gmt */
	uint32_t tzh_ttisgmtcnt;
	/* number of transition time flags in local time */
	uint32_t tzh_ttisstdcnt;
	/* number of recorded leap seconds */
	uint32_t tzh_leapcnt;
	/* number of recorded transition times */
	uint32_t tzh_timecnt;
	/* number of local time type */
	uint32_t tzh_typecnt;
	/* number of abbreviation chars */
	uint32_t tzh_charcnt;
};

/* this one must be packed to account for the packed file layout */
struct ztrdtl_s {
	int32_t offs;
	uint8_t dstp;
	uint8_t abbr;
} __attribute__((packed));

/* convenience struct where we copy all the good things into one */
struct zspec_s {
	int32_t since;
	unsigned int offs:31;
	unsigned int dstp:1;
	znam_t name;
} __attribute__((packed, aligned(16)));

/* for leap second transitions */
struct zleap_tr_s {
	/* cut-off stamp */
	int32_t t;
	/* cumulative correction since T */
	int32_t corr;
};

/* leap second support missing */
struct zif_s {
	size_t mpsz;
	zih_t hdr;

	/* transitions */
	ztr_t trs;
	/* types */
	zty_t tys;
	/* type array, deser'd, transition details array */
	ztrdtl_t tda;
	/* zonename array */
	znam_t zn;

	/* file descriptor, if >0 this also means all data is in BE */
	int fd;

	/* for special zones */
	coord_zone_t cz;

	/* zone caching, between PREV and NEXT the offset is OFFS */
	struct zrng_s cache;
};


#if defined TZDIR
static const char tzdir[] = TZDIR;
#else  /* !TZDIR */
static const char tzdir[] = "/usr/share/zoneinfo";
#endif

#if defined ZONEINFO_UTC_RIGHT
/* where can we deduce some info for our coordinated zones */
static const char coord_fn[] = ZONEINFO_UTC_RIGHT;
#else  /* !ZONEINFO_UTC_RIGHT */
static const char coord_fn[] = "/usr/share/zoneinfo/right/UTC";
#endif	/* ZONEINFO_UTC_RIGHT */

#define PROT_MEMMAP	PROT_READ | PROT_WRITE
#define MAP_MEMMAP	MAP_PRIVATE | MAP_ANONYMOUS

#define AS_MUT_ZIF(x)	((struct zif_s*)deconst(x))

/* special zone names */
static const char coord_zones[][4] = {
	"",
	"UTC",
	"TAI",
	"GPS",
};

static void*
deconst(const void *ptr)
{
	return (char*)1 + ((const char*)ptr - (char*)1U);
}

/**
 * Return the total number of transitions in zoneinfo file Z. */
DEFUN inline size_t
zif_ntrans(const struct zif_s z[static 1U])
{
	return z->hdr->tzh_timecnt;
}

/**
 * Return the total number of transition types in zoneinfo file Z. */
static inline size_t
zif_ntypes(const struct zif_s z[static 1U])
{
	return z->hdr->tzh_typecnt;
}

/**
 * Return the total number of transitions in zoneinfo file Z. */
static inline size_t
zif_nchars(const struct zif_s z[static 1U])
{
	return z->hdr->tzh_charcnt;
}


/**
 * Return the total number of leap second transitions. */
static __attribute__((unused)) inline size_t
zif_nleaps(const struct zif_s z[static 1U])
{
	return z->hdr->tzh_leapcnt;
}

/**
 * Return the transition time stamp of the N-th transition in Z. */
static inline int32_t
zif_trans(const struct zif_s z[static 1U], int n)
{
/* no bound check! */
	return zif_ntrans(z) > 0UL ? z->trs[n] : INT_MIN;
}

/**
 * Return the transition type index of the N-th transition in Z. */
static inline uint8_t
zif_type(const struct zif_s z[static 1U], int n)
{
/* no bound check! */
	return (uint8_t)(zif_ntrans(z) > 0UL ? z->tys[n] : 0U);
}

/**
 * Return the transition details after the N-th transition in Z. */
inline struct ztrdtl_s
zif_trdtl(const struct zif_s z[static 1U], int n)
{
/* no bound check! */
	struct ztrdtl_s res;
	uint8_t idx = zif_type(z, n);
	res = z->tda[idx];
	return res;
}

/**
 * Return the gmt offset the N-th transition in Z. */
static inline int32_t
zif_troffs(const struct zif_s z[static 1U], int n)
{
/* no bound check! */
	uint8_t idx = zif_type(z, n);
	return z->tda[idx].offs;
}

/**
 * Return the zonename after the N-th transition in Z. */
static __attribute__((unused)) inline znam_t
zif_trname(const struct zif_s z[static 1U], int n)
{
/* no bound check! */
	uint8_t idx = zif_type(z, n);
	uint8_t jdx = z->tda[idx].abbr;
	return z->zn + jdx;
}

/**
 * Return a succinct summary of the situation after transition N in Z. */
static __attribute__((unused)) inline struct zspec_s
zif_spec(const struct zif_s z[static 1U], int n)
{
	struct zspec_s res;
	uint8_t idx = zif_type(z, n);
	uint8_t jdx = z->tda[idx].abbr;

	res.since = zif_trans(z, n);
	res.offs = z->tda[idx].offs;
	res.dstp = z->tda[idx].dstp;
	res.name = z->zn + jdx;
	return res;
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
	if (file == NULL || file[0] == '\0') {
		return -1;
	} else if (file[0] != '/') {
		/* not an absolute file name */
		size_t len = strlen(file);
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

static void
__init_zif(struct zif_s z[static 1U])
{
	size_t ntr;
	size_t nty;

	if (z->fd > STDIN_FILENO) {
		/* probably in BE then, eh? */
		ntr = be32toh(zif_ntrans(z));
		nty = be32toh(zif_ntypes(z));
	} else {
		ntr = zif_ntrans(z);
		nty = zif_ntypes(z);
	}

	z->trs = (ztr_t)(z->hdr + 1);
	z->tys = (zty_t)(z->trs + ntr);
	z->tda = (ztrdtl_t)(z->tys + ntr);
	z->zn = (char*)(z->tda + nty);
	return;
}

static int
__read_zif(struct zif_s tgt[static 1U], int fd)
{
	struct stat st;

	if (fstat(fd, &st) < 0) {
		return -1;
	} else if (st.st_size <= 4) {
		return -1;
	}
	tgt->mpsz = st.st_size;
	tgt->fd = fd;
	tgt->hdr = mmap(NULL, tgt->mpsz, PROT_READ, MAP_SHARED, fd, 0);
	if (tgt->hdr == MAP_FAILED) {
		return -1;
	}
	/* all clear so far, populate */
	__init_zif(tgt);
	return 0;
}

static void
__conv_hdr(struct zih_s tgt[static 1U], const struct zih_s src[static 1U])
{
/* copy SRC to TGT doing byte-order conversions on the way */
	memcpy(tgt, src, offsetof(struct zih_s, tzh_ttisgmtcnt));
	tgt->tzh_ttisgmtcnt = be32toh(src->tzh_ttisgmtcnt);
	tgt->tzh_ttisstdcnt = be32toh(src->tzh_ttisstdcnt);
	tgt->tzh_leapcnt = be32toh(src->tzh_leapcnt);
	tgt->tzh_timecnt = be32toh(src->tzh_timecnt);
	tgt->tzh_typecnt = be32toh(src->tzh_typecnt);
	tgt->tzh_charcnt = be32toh(src->tzh_charcnt);
	return;
}

static void
__conv_zif(struct zif_s tgt[static 1U], const struct zif_s src[static 1U])
{
	size_t ntr;
	size_t nty;
	size_t nch;

	/* convert header to hbo */
	__conv_hdr(tgt->hdr, src->hdr);

	/* everything in host byte-order already */
	ntr = zif_ntrans(tgt);
	nty = zif_ntypes(tgt);
	nch = zif_nchars(tgt);
	__init_zif(tgt);

	/* transition vector */
	for (size_t i = 0; i < ntr; i++) {
		tgt->trs[i] = be32toh(src->trs[i]);
	}

	/* type vector, nothing to byte-swap here */
	memcpy(tgt->tys, src->tys, ntr * sizeof(*tgt->tys));

	/* transition details vector */
	for (size_t i = 0; i < nty; i++) {
		tgt->tda[i].offs = be32toh(src->tda[i].offs);
		tgt->tda[i].dstp = src->tda[i].dstp;
		tgt->tda[i].abbr = src->tda[i].abbr;
	}

	/* zone name array */
	memcpy(tgt->zn, src->zn, nch * sizeof(*tgt->zn));
	return;
}

static struct zif_s*
__copy_conv(const struct zif_s z[static 1U])
{
/* copy Z and do byte-order conversions */
	size_t mpsz;
	struct zif_s *res = NULL;

	/* compute a size */
	mpsz = z->mpsz + sizeof(*z);

	/* we'll mmap ourselves a slightly larger struct so
	 * res + 1 points to the header, while res + 0 is the zif_t */
	res = mmap(NULL, mpsz, PROT_MEMMAP, MAP_MEMMAP, -1, 0);
	if (UNLIKELY(res == MAP_FAILED)) {
		return NULL;
	}
	/* great, now to some initial assignments */
	res->mpsz = mpsz;
	res->hdr = (void*)(res + 1);
	/* make sure we denote that this isnt connected to a file */
	res->fd = -1;
	/* copy the flags though */
	res->cz = z->cz;

	/* convert the header and payload now */
	__conv_zif(res, z);

	/* that's all :) */
	return res;
}

static struct zif_s*
__copy(const struct zif_s z[static 1U])
{
/* copy Z into a newly allocated zif_t object
 * if applicable also perform byte-order conversions */
	struct zif_s *res;

	if (z->fd > STDIN_FILENO) {
		return __copy_conv(z);
	}
	/* otherwise it's a plain copy */
	res = mmap(NULL, z->mpsz, PROT_MEMMAP, MAP_MEMMAP, -1, 0);
	if (UNLIKELY(res == MAP_FAILED)) {
		return NULL;
	}
	memcpy(res, z, z->mpsz);
	__init_zif(res);
	return res;
}

DEFUN zif_t
zif_copy(zif_t z)
{
/* copy Z into a newly allocated zif_t object
 * if applicable also perform byte-order conversions */
	if (UNLIKELY(z == NULL)) {
		/* no need to bother */
		return NULL;
	}
	return __copy(z);
}

static void
__close(const struct zif_s z[static 1U])
{
	if (z->fd > STDIN_FILENO) {
		close(z->fd);
	}
	/* check if z is in mmap()'d space */
	if (z->hdr == MAP_FAILED) {
		/* not sure what to do */
		;
	} else if ((z + 1) != (void*)z->hdr) {
		/* z->hdr is mmapped, z is not */
		munmap((void*)z->hdr, z->mpsz);
	} else {
		munmap(deconst(z), z->mpsz);
	}
	return;
}

DEFUN void
zif_close(zif_t z)
{
	if (UNLIKELY(z == NULL)) {
		/* nothing to do */
		return;
	}
	__close((const void*)z);
	return;
}

DEFUN zif_t
zif_open(const char *file)
{
	coord_zone_t cz;
	int fd;
	struct zif_s tmp[1];
	struct zif_s *res;

	/* check for special time zones */
	if ((cz = coord_zone(file)) > TZCZ_UNK) {
		file = coord_fn;
	}

	if (UNLIKELY((fd = __open_zif(file)) <= STDIN_FILENO)) {
		return NULL;
	} else if (UNLIKELY(__read_zif(tmp, fd) < 0)) {
		return NULL;
	}
	/* otherwise all's fine, it's still BE
	 * assign the coord zone type if any and convert to host byte-order */
	tmp->cz = cz;
	res = __copy(tmp);
	__close(tmp);
	return res;
}


/* for leap corrections */
#include "leapseconds.def"

static inline int
__find_trno(
	const struct zif_s z[static 1U],
	int32_t t, int this, int min, int max)
{
	do {
		int32_t tl, tu;

		if (UNLIKELY(max == 0)) {
			/* special case */
			return 0;
		}

		tl = zif_trans(z, this);
		tu = zif_trans(z, this + 1);

		if (t >= tl && t < tu) {
			/* found him */
			return this;
		} else if (max - 1 <= min) {
			/* nearly found him */
			return this + 1;
		} else if (t >= tu) {
			min = this + 1;
			this = (this + max) / 2;
		} else if (t < tl) {
			max = this - 1;
			this = (this + min) / 2;
		}
	} while (true);
	/* not reached */
}

DEFUN inline int
zif_find_trans(zif_t z, int32_t t)
{
/* find the last transition before time, time is expected to be UTC */
	int max = zif_ntrans(z);
	int min = 0;
	int this = max / 2;

	return __find_trno(z, t, this, min, max);
}

static struct zrng_s
__find_zrng(
	const struct zif_s z[static 1U],
	int32_t t, int this, int min, int max)
{
	struct zrng_s res;
	size_t ntr = zif_ntrans(z);
	unsigned int trno = __find_trno(z, t, this, min, max);

	res.trno = (uint8_t)trno;
	res.prev = zif_trans(z, trno);
	if (LIKELY(trno < ntr - 1)) {
		res.next = zif_trans(z, trno + 1);
	} else {
		res.next = INT_MAX;
		/* use the last transition */
		trno = ntr - 1;
	}
	res.offs = zif_troffs(z, trno);
	return res;
}

DEFUN inline struct zrng_s
zif_find_zrng(zif_t z, int32_t t)
{
/* find the last transition before time, time is expected to be UTC */
	int max = zif_ntrans(z);
	int min = 0;
	int this = max / 2;

	return __find_zrng(z, t, this, min, max);
}

static int32_t
__tai_offs(int32_t t)
{
	/* difference of TAI and UTC at epoch instant */
	const int32_t tai_offs_epoch = 10;
	zidx_t zi = leaps_before_si32(leaps_s, nleaps_corr, t);

	return tai_offs_epoch + leaps_corr[zi];
}

static int32_t
__gps_offs(int32_t t)
{
/* TAI - GPS = 19 on 1980-01-06, so use that identity here */
	const int32_t gps_offs_epoch = 19;
	if (UNLIKELY(t < 315964800)) {
		return 0;
	}
	return __tai_offs(t) - gps_offs_epoch;
}

static int32_t
__offs(struct zif_s z[static 1U], int32_t t)
{
/* return the offset of T in Z and cache the result. */
	int this;
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
		this = z->cache.trno + 1;
		min = this;
		max = zif_ntrans(z);
	} else if (t < z->cache.prev) {
		max = z->cache.trno;
		this = max - 1;
		min = 0;
	} else {
		/* we shouldn't end up here at all */
		this = 0;
		min = 0;
		max = 0;
	}
	return (z->cache = __find_zrng(z, t, this, min, max)).offs;
}

DEFUN int32_t
zif_utc_time(zif_t z, int32_t t)
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
	int32_t xi = 0;
	int32_t xj;
	int32_t old = -1;

	/* jump off the cliff if Z is nought */
	if (UNLIKELY(z == NULL)) {
		return t;
	}

	while ((xj = __offs(AS_MUT_ZIF(z), t - xi)) != xi && xi != old) {
		old = xi = xj;
	}
	return t - xj;
}

/* convert utc to local */
DEFUN int32_t
zif_local_time(zif_t z, int32_t t)
{
	/* jump off the cliff if Z is nought */
	if (UNLIKELY(z == NULL)) {
		return t;
	}
	return t + __offs(AS_MUT_ZIF(z), t);
}

#endif	/* INCLUDED_tzraw_c_ */
/* tzraw.c ends here */
