/*** dt-core.h -- our universe of datetimes
 *
 * Copyright (C) 2011-2024 Sebastian Freundt
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

#if !defined INCLUDED_dt_core_h_
#define INCLUDED_dt_core_h_

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include "boops.h"
#include "date-core.h"
#include "time-core.h"

typedef enum {
	/* this one's our own version of UNK */
	DT_UNK = 0,
	/* the lower date types come from date-core.h */
	DT_PACK = DT_NDTYP,
	DT_YMDHMS = DT_PACK,
	DT_SEXY,
	DT_SEXYTAI,
	DT_NDTTYP,
} dt_dttyp_t;

typedef enum {
	DT_DURH = DT_NDURTYP + 0U,
	DT_DURM = DT_NDURTYP + 1U,
	DT_DURS = DT_NDURTYP + 2U,
	DT_DURNANO = DT_NDURTYP + 3U,
} dt_dtdurtyp_t;
/* this will be 16 and hence must be #defined as we're using
 * a bitfield of size 4 to store the dtdurtyp. */
#define DT_NDTDURTYP	(DT_NDURTYP + 4U)

/** packs
 * packs are just packs of dates and times */
typedef union {
	uint64_t u:53;
	struct {
#if BYTE_ORDER == BIG_ENDIAN
#define DT_YEAR_OFFS	(1900)
		/* offset by the year 1900 */
		unsigned int y:12;
		unsigned int m:4;
		unsigned int d:6;
		/* round up to 32 bits, remaining bits are seconds east */
		unsigned int offs:10;

		/* time part */
		unsigned int H:5;
		unsigned int M:8;
		unsigned int S:8;
#elif BYTE_ORDER == LITTLE_ENDIAN
		unsigned int d:6;
		unsigned int m:4;
		/* offset by the year 1900 */
#define DT_YEAR_OFFS	(1900)
		unsigned int y:12;
		/* round up to 32 bits, remaining bits are seconds east */
		unsigned int offs:10;

		/* time part */
		unsigned int S:8;
		unsigned int M:8;
		unsigned int H:5;
#else
# warning unknown byte order
#endif	/* BYTE_ORDER */
	};
} dt_ymdhms_t;

/** sexy
 * sexy is really, secsi, seconds since X, 1970-01-01T00:00:00 here */
typedef int64_t dt_sexy_t;
typedef int64_t dt_ssexy_t;
#define DT_SEXY_BASE_YEAR	(1917)

struct dt_dt_s {
	union {
		/* packs */
		struct {
			/* dt type, or date type */
			dt_dttyp_t typ:4;
			/* sandwich indicator (use d and t slots below) */
			uint16_t sandwich:1;
			/* whether we had zone info already but fixed it */
			uint16_t znfxd:1;
			/* whether to be aware of leap-seconds */
			uint16_t tai:1;
			/* error indicator to denote date has been fixed up */
			uint16_t fix:1;
			/* was duration indicator */
			uint16_t xxx:1;
			/* negation indicator */
			uint16_t neg:1;
			/* we've got 6 bits left here to coincide with dt_d_s
			 * use that and the neg flag for zdiffs
			 * zdiff itself has 15-minute resolution,
			 * range [0, 63] aka [00:00 16:00]
			 * The policy is to store the time always in UTC
			 * but keep the difference in this slot. */
			uint16_t zdiff:6;
#define ZDIFF_RES	(15U * 60U)

			union {
				uint64_t u:48;
				dt_ymdhms_t ymdhms;
				dt_sexy_t sexy:48;
				dt_ssexy_t sxepoch:48;
				struct {
#if BYTE_ORDER == BIG_ENDIAN
					int32_t corr:16;
					int32_t soft:32;
#elif BYTE_ORDER == LITTLE_ENDIAN
					int32_t soft:32;
					int32_t corr:16;
#else
# warning unknown byte order
#endif	/* BYTE_ORDER */
				};
			};
		} __attribute__((packed));
		/* sandwich types */
		struct {
			struct dt_d_s d;
			struct dt_t_s t;
		};
	};
};

struct dt_dtdur_s {
	union {
		/* packs */
		struct {
			/* dt type, or date type */
			dt_dtdurtyp_t durtyp:4;
			/* was sandwich indicator */
			uint16_t:1;
			/* whether we had zone info already but fixed it */
			uint16_t znfxd:1;
			/* whether to be aware of leap-seconds */
			uint16_t tai:1;
			/* error indicator to denote date has been fixed up */
			uint16_t fix:1;
			/* co-class indicator */
			uint16_t cocl:1;
			/* negation indicator */
			uint16_t neg:1;
			/* we've got 6 bits left here to coincide with dt_d_s
			 * use that and the neg flag for zdiffs
			 * zdiff itself has 15-minute resolution,
			 * range [0, 63] aka [00:00 16:00]
			 * The policy is to store the time always in UTC
			 * but keep the difference in this slot. */
			uint16_t zdiff:6;
#define ZDIFF_RES	(15U * 60U)

			union {
				uint64_t u:48;
				dt_ymdhms_t ymdhms;
				/* for value+unit durations */
				dt_ssexy_t dv:48;
				struct {
#if BYTE_ORDER == BIG_ENDIAN
					int32_t corr:16;
					int32_t soft:32;
#elif BYTE_ORDER == LITTLE_ENDIAN
					int32_t soft:32;
					int32_t corr:16;
#else
# warning unknown byte order
#endif	/* BYTE_ORDER */
				};
			};
		} __attribute__((packed));
		/* sandwich types */
		struct {
			struct dt_ddur_s d;
			struct dt_t_s t;
		};
	};
};


/* decls */
/**
 * Like strptime() for our dates.
 * The format characters are _NOT_ compatible with strptime().
 * If FMT is NULL the standard format for each calendric system is used,
 * see format.texi or dateutils info page.
 *
 * FMT can also be the name of a calendar:
 * - ymd for YMD dates
 * - ymcw for YMCW dates
 * - bizda for bizda/YMDU dates
 *
 * If optional EP is non-NULL it will point to the end of the parsed
 * date string. */
extern struct dt_dt_s
dt_strpdt(const char *str, const char *fmt, char **ep);

/**
 * Like strftime() for our dates */
extern size_t
dt_strfdt(char *restrict buf, size_t bsz, const char *fmt, struct dt_dt_s);

/**
 * Parse durations as in 1w5d, etc. */
extern struct dt_dtdur_s
dt_strpdtdur(const char *str, char **ep);

/**
 * Print a duration. */
extern size_t
dt_strfdtdur(char *restrict buf, size_t bsz, const char *fmt, struct dt_dtdur_s);

/**
 * Negate the duration. */
extern struct dt_dtdur_s dt_neg_dtdur(struct dt_dtdur_s);

/**
 * Is duration DUR negative? */
extern int dt_dtdur_neg_p(struct dt_dtdur_s dur);

/**
 * Like time() but return the current date in the desired format. */
extern struct dt_dt_s dt_datetime(dt_dttyp_t dttyp);

/**
 * Convert D to another calendric system, specified by TGTTYP. */
extern struct dt_dt_s dt_dtconv(dt_dttyp_t tgttyp, struct dt_dt_s);

/**
 * Add duration DUR to date/time D.
 * The result will be in the calendar as specified by TGTTYP, or if
 * DT_UNK is given, the calendar of D will be used. */
extern struct dt_dt_s
dt_dtadd(struct dt_dt_s d, struct dt_dtdur_s dur);

/**
 * Get duration between D1 and D2.
 * The result will be of type TGTTYP,
 * the calendar of D1 will be used, e.g. its month-per-year, days-per-week,
 * etc. conventions count.
 * If instead D2 should count, swap D1 and D2 and negate the duration
 * by setting/clearing the neg bit. */
extern struct dt_dtdur_s
dt_dtdiff(dt_dtdurtyp_t tgttyp, struct dt_dt_s d1, struct dt_dt_s d2);

/**
 * Compare two dates, yielding 0 if they are equal, -1 if D1 is older,
 * 1 if D1 is younger than the D2. */
extern int dt_dtcmp(struct dt_dt_s d1, struct dt_dt_s d2);

/**
 * Check if D is in the interval spanned by D1 and D2,
 * 1 if D1 is younger than the D2. */
extern int
dt_dt_in_range_p(struct dt_dt_s d, struct dt_dt_s d1, struct dt_dt_s d2);

/* more specific but still useful functions */
/**
 * Convert a dt_dt_s to an epoch difference, based on the Unix epoch. */
extern dt_ssexy_t dt_to_unix_epoch(struct dt_dt_s);

/**
 * Convert a dt_dt_s to an epoch difference, based on the GPS epoch. */
extern dt_ssexy_t dt_to_gps_epoch(struct dt_dt_s);

/**
 * Set specific fallback date/time to use when input is underspecified.
 * Internally, when no default is set and input is underspecified  the
 * value of `dt_datetime()' (i.e. now) is used to fill fields up.
 * This is also used for ambiguous format specifiers (like %y or %_y)
 * to position their range on the absolute time scale. */
extern void dt_set_base(struct dt_dt_s);
#define HAVE_DT_SET_BASE	1

/**
 * Return the base date/time as struct dt_dt_s. */
extern struct dt_dt_s dt_get_base(void);
#define HAVE_DT_GET_BASE	1

/**
 * Crop datetimess with days beyond ultimo. */
extern __attribute__((const)) struct dt_dt_s dt_fixup(struct dt_dt_s);

/**
 * Decay military midnigths to the following day 00:00:00 */
extern __attribute__((const)) struct dt_dt_s dt_milfup(struct dt_dt_s);


/* some useful gimmicks, sort of */
static inline __attribute__((const)) bool
dt_unk_p(struct dt_dt_s d)
{
	return !(d.sandwich || d.typ > DT_UNK);
}

static inline __attribute__((const)) bool
dt_durunk_p(struct dt_dtdur_s d)
{
	return !d.durtyp;
}

static inline __attribute__((const)) bool
dt_sandwich_p(struct dt_dt_s d)
{
	return d.sandwich && d.d.typ > DT_DUNK;
}

static inline __attribute__((const)) bool
dt_sandwich_only_d_p(struct dt_dt_s d)
{
	return !d.sandwich && d.d.typ > DT_DUNK && d.d.typ < DT_NDTYP;
}

static inline __attribute__((const)) bool
dt_sandwich_only_t_p(struct dt_dt_s d)
{
	return d.sandwich && d.typ == DT_UNK;
}

static inline __attribute__((const)) bool
dt_separable_p(struct dt_dt_s d)
{
/* return true if D is a d+t sandwich or D is d-only or D is t-only */
	return d.typ < DT_PACK;
}

#define DT_SANDWICH_UNK		(DT_UNK)

static inline void
dt_make_sandwich(struct dt_dt_s *d, dt_dtyp_t dty, dt_ttyp_t tty)
{
	d->d.typ = dty;
	d->t.typ = tty;
	d->sandwich = 1;
	return;
}

static inline void
dt_make_d_only(struct dt_dt_s *d, dt_dtyp_t dty)
{
	d->d.typ = dty;
	d->t.typ = DT_TUNK;
	d->sandwich = 0;
	return;
}

static inline void
dt_make_t_only(struct dt_dt_s *d, dt_ttyp_t tty)
{
	d->d.typ = DT_DUNK;
	d->t.typ = tty;
	d->sandwich = 1;
	return;
}

#endif	/* INCLUDED_dt_core_h_ */
