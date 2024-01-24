/*** tzraw.h -- reader for olson database zoneinfo files
 *
 * Copyright (C) 2009-2024 Sebastian Freundt
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
#include "leaps.h"

#if defined __cplusplus
extern "C" {
#endif	/* __cplusplus */

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
typedef int64_t stamp_t;

#define STAMP_MIN	(-140737488355328LL)
#define STAMP_MAX	(140737488355327LL)

typedef enum {
	TZCZ_UNK,
	TZCZ_UTC,
	TZCZ_TAI,
	TZCZ_GPS,
	TZCZ_NZONE,
} coord_zone_t;

/* for the one tool that needs raw transitions */
struct zrng_s {
	stamp_t prev, next;
	signed int offs:24;
	unsigned int trno:8;
} __attribute__((packed));


/**
 * Open the zoneinfo file FILE.
 * FILE can be absolute or relative to the configured TZDIR path.
 * FILE can also name virtual zones such as GPS or TAI. */
extern zif_t zif_open(const char *file);

/**
 * Close the zoneinfo file reader and free associated resources. */
extern void zif_close(zif_t);

/**
 * Copy the zoneinfo structure. */
extern zif_t zif_copy(zif_t);

/**
 * Find the most recent transition in Z before T. */
extern int zif_find_trans(zif_t z, stamp_t t);

/**
 * Find a range of transitions in Z that T belongs to. */
extern struct zrng_s zif_find_zrng(zif_t z, stamp_t t);

/**
 * Given T in local time specified by Z, return a T in UTC. */
extern stamp_t zif_utc_time(zif_t z, stamp_t t);

/**
 * Given T in UTC, return a T in local time specified by Z. */
extern stamp_t zif_local_time(zif_t z, stamp_t t);


/* exposure for specific zif-inspecting tools (dzone(1) for one) */
/**
 * Return the gmt offset (in seconds) after the N-th transition in Z. */
extern int zif_troffs(zif_t z, int n);

/**
 * Return the number of transitions in Z. */
extern size_t zif_ntrans(zif_t z);

#if defined __cplusplus
}
#endif	/* __cplusplus */

#endif	/* INCLUDED_tzraw_h_ */
