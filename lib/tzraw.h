/*** tzraw.h -- reader for olson database zoneinfo files
 *
 * Copyright (C) 2009 Sebastian Freundt
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

#if !defined INCLUDED_tzraw_h_
#define INCLUDED_tzraw_h_

#include <stdint.h>
/* for lil endian conversions */
#include <endian.h>
#include <byteswap.h>
#include <limits.h>

#if defined __cplusplus
extern "C" {
#endif	/* __cplusplus */

#if !defined DECLF
# define DECLF	static __attribute__((unused))
# define DEFUN	static
# define INCLUDE_TZRAW_IMPL
#elif !defined DEFUN
# define DEFUN
#endif	/* !DECLF */
#if !defined restrict
# define restrict	__restrict
#endif	/* !restrict */

/*
** Information about time zone files.
*/
/* CHANGED! */
#ifndef TZDIR
#define TZDIR	"/usr/share/zoneinfo"
#endif /* !defined TZDIR */

/*
** Each file begins with. . .
*/
#define	TZ_MAGIC	"TZif"

#if 0
struct tzhead {
	char	tzh_magic[4];		/* TZ_MAGIC */
	char	tzh_version[1];		/* '\0' or '2' as of 2005 */
	char	tzh_reserved[15];	/* reserved--must be zero */
	char	tzh_ttisgmtcnt[4];	/* coded number of trans. time flags */
	char	tzh_ttisstdcnt[4];	/* coded number of trans. time flags */
	char	tzh_leapcnt[4];		/* coded number of leap seconds */
	char	tzh_timecnt[4];		/* coded number of transition times */
	char	tzh_typecnt[4];		/* coded number of local time types */
	char	tzh_charcnt[4];		/* coded number of abbr. chars */
};

/*
** . . .followed by. . .
**
**	tzh_timecnt (char [4])s		coded transition times a la time(2)
**	tzh_timecnt (unsigned char)s	types of local time starting at above
**	tzh_typecnt repetitions of
**		one (char [4])		coded UTC offset in seconds
**		one (unsigned char)	used to set tm_isdst
**		one (unsigned char)	that's an abbreviation list index
**	tzh_charcnt (char)s		'\0'-terminated zone abbreviations
**	tzh_leapcnt repetitions of
**		one (char [4])		coded leap second transition times
**		one (char [4])		total correction after above
**	tzh_ttisstdcnt (char)s		indexed by type; if TRUE, transition
**					time is standard time, if FALSE,
**					transition time is wall clock time
**					if absent, transition times are
**					assumed to be wall clock time
**	tzh_ttisgmtcnt (char)s		indexed by type; if TRUE, transition
**					time is UTC, if FALSE,
**					transition time is local time
**					if absent, transition times are
**					assumed to be local time
*/

/*
** If tzh_version is '2' or greater, the above is followed by a second instance
** of tzhead and a second instance of the data in which each coded transition
** time uses 8 rather than 4 chars,
** then a POSIX-TZ-environment-variable-style string for use in handling
** instants after the last transition time stored in the file
** (with nothing between the newlines if there is no POSIX representation for
** such instants).
*/
#endif	/* 0 */


/* now our view on things */
typedef struct zif_s *zif_t;
typedef struct zih_s *zih_t;
typedef struct ztrdtl_s *ztrdtl_t;
typedef const char *znam_t;

/* convenience struct where we copy all the good things into one */
struct zspec_s {
	int32_t since;
	unsigned int offs:31;
	unsigned int dstp:1;
	znam_t name;
} __attribute__((packed, aligned(16)));

/* that's tzhead but better */
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

/* for internal use only, fuck off */
struct zrng_s {
	int32_t prev, next;
	signed int offs:24;
	unsigned int trno:8;
} __attribute__((packed));

/* leap second support missing */
struct zif_s {
	size_t mpsz;
	zih_t hdr;

	/* transitions */
	int32_t *trs;
	/* types */
	uint8_t *tys;
	/* type array, deser'd, transition details array */
	ztrdtl_t tda;
	/* zonename array */
	znam_t zn;

	/* file descriptor */
	int fd;

	/* zone caching, between PREV and NEXT the offset is OFFS */
	struct zrng_s cache;
};


/**
 * Read the zoneinfo file FILE. */
DECLF zif_t zif_read(const char *file);
/**
 * Read and instantiate the zoneinfo file FILE. */
DECLF zif_t zif_read_inst(const char *file);
/**
 * Close the zoneinfo file reader and free associated resources. */
DECLF void zif_close(zif_t);
/**
 * Instantiate the zoneinfo structure, so it is purely in memory. */
DECLF zif_t zif_inst(zif_t);
/**
 * Free an instantiated zoneinfo structure. */
DECLF inline void zif_free(zif_t);
/**
 * Copy the zoneinfo structure. */
#define zif_copy	zif_inst
/**
 * Find the most recent transition in Z before T. */
DECLF int zif_find_trans(zif_t z, int32_t t);
/**
 * Find a range of transitions in Z that T belongs to. */
DECLF struct zrng_s zif_find_zrng(zif_t z, int32_t t);
/**
 * Given T in local time specified by Z, return a T in UTC. */
DECLF int32_t zif_utc_time(zif_t z, int32_t t);
/**
 * Given T in UTC, return a T in local time specified by Z. */
DECLF int32_t zif_local_time(zif_t z, int32_t t);


/**
 * Return the total number of transitions in zoneinfo file Z. */
static inline size_t
zif_ntrans(zif_t z)
{
	return bswap_32(z->hdr->tzh_timecnt);
}

/**
 * Return the transition time stamp of the N-th transition in Z. */
static inline int32_t
zif_trans(zif_t z, int n)
{
/* no bound check! */
	return zif_ntrans(z) > 0 ? (int32_t)bswap_32(z->trs[n]) : INT_MIN;
}

/**
 * Return the total number of transition types in zoneinfo file Z. */
static inline size_t
zif_ntypes(zif_t z)
{
	return bswap_32(z->hdr->tzh_typecnt);
}

/**
 * Return the transition type index of the N-th transition in Z. */
static inline uint8_t
zif_type(zif_t z, int n)
{
/* no bound check! */
	return (uint8_t)(zif_ntrans(z) > 0 ? z->tys[n] : 0);
}

/**
 * Return the transition details after the N-th transition in Z. */
static inline struct ztrdtl_s
zif_trdtl(zif_t z, int n)
{
/* no bound check! */
	struct ztrdtl_s res;
	uint8_t idx = zif_type(z, n);
	res = z->tda[idx];
	res.offs = bswap_32(z->tda[idx].offs);
	return res;
}

/**
 * Return the gmt offset the N-th transition in Z. */
static inline int32_t
zif_troffs(zif_t z, int n)
{
/* no bound check! */
	uint8_t idx = zif_type(z, n);
	return bswap_32(z->tda[idx].offs);
}

/**
 * Return the zonename after the N-th transition in Z. */
static inline znam_t
zif_trname(zif_t z, int n)
{
/* no bound check! */
	uint8_t idx = zif_type(z, n);
	uint8_t jdx = z->tda[idx].abbr;
	return z->zn + jdx;
}

/**
 * Return a succinct summary of the situation after transition N in Z. */
static inline struct zspec_s
zif_spec(zif_t z, int n)
{
	struct zspec_s res;
	uint8_t idx = zif_type(z, n);
	uint8_t jdx = z->tda[idx].abbr;

	res.since = zif_trans(z, n);
	res.offs = bswap_32(z->tda[idx].offs);
	res.dstp = z->tda[idx].dstp;
	res.name = z->zn + jdx;
	return res;
}

static inline void
zif_free(zif_t z)
{
	/* we can use zif_close() here as we mmapped our memory */
	zif_close(z);
	return;
}


#if defined INCLUDE_TZRAW_IMPL
# include "tzraw.c"
#endif	/* INCLUDE_TZRAW_IMPL */

#if defined __cplusplus
}
#endif	/* __cplusplus */

#endif	/* INCLUDED_tzraw_h_ */
