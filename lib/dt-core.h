/*** dt-core.h -- our universe of datetimes
 *
 * Copyright (C) 2011-2012 Sebastian Freundt
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

#if defined __cplusplus
extern "C" {
#endif	/* __cplusplus */

#if !defined DECLF
# define DECLF	static __attribute__((unused))
# define DEFUN	static
# define INCLUDE_DATETIME_CORE_IMPL
# define INCLUDE_TZRAW_IMPL
# define INCLUDE_LEAPS_IMPL
#elif !defined DEFUN
# define DEFUN
#endif	/* !DECLF */
#if !defined DECLV
# define DECLV		DECLF
#endif	/* !DECLV */
#if !defined DEFVAR
# define DEFVAR		DEFUN
#endif	/* !DEFVAR */
#if !defined restrict
# define restrict	__restrict
#endif	/* !restrict */

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

/** packs
 * packs are just packs of dates and times */
typedef union {
	uint64_t u:53;
	struct {
#if defined WORDS_BIGENDIAN
#define DT_YEAR_OFFS	(1900)
		/* offset by the year 1900 */
		unsigned int y:12;
		unsigned int m:4;
		unsigned int d:5;
		/* round up to 32 bits, remaining bits are seconds east */
		unsigned int offs:11;

		/* time part */
		unsigned int H:5;
		unsigned int M:8;
		unsigned int S:8;
#else  /* !WORDS_BIGENDIAN */
		unsigned int d:5;
		unsigned int m:4;
		/* offset by the year 1900 */
#define DT_YEAR_OFFS	(1900)
		unsigned int y:12;
		/* round up to 32 bits, remaining bits are seconds east */
		unsigned int offs:11;

		/* time part */
		unsigned int S:8;
		unsigned int M:8;
		unsigned int H:5;
#endif	/* WORDS_BIGENDIAN */
	};
} dt_ymdhms_t;

/** sexy
 * sexy is really, secsi, seconds since X, 1970-01-01T00:00:00 here */
typedef uint64_t dt_sexy_t;
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
			/* unused, pad to next ui8 */
			uint16_t:3;
			/* duration indicator */
			uint16_t dur:1;
			/* negation indicator */
			uint16_t neg:1;
			/* whether to be aware of leap-seconds */
			uint16_t tai:1;
			union {
				uint64_t u:53;
				dt_ymdhms_t ymdhms;
				dt_sexy_t sexy:53;
				dt_ssexy_t sexydur:53;
				dt_ssexy_t sxepoch:53;
				struct {
#if defined WORDS_BIGENDIAN
					int32_t corr:21;
					int32_t soft:32;
#else  /* !WORDS_BIGENDIAN */
					int32_t soft:32;
					int32_t corr:21;
#endif	/* WORDS_BIGENDIAN */
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

/* spec tokeniser, spec flags plus modifiers and stuff */
#if 0
struct dt_spec_s {
	struct {
		/* ordinal flag, 01, 02, 03 -> 1st 2nd 3rd */
		unsigned int ord:1;
		/* roman numeral flag */
		unsigned int rom:1;
		/* controls abbreviation */
		enum {
			DT_SPMOD_NORM,
			DT_SPMOD_ABBR,
			DT_SPMOD_LONG,
			DT_SPMOD_ILL,
		} abbr:2;
		/* for directions a(fter 0)/b(efore 1) */
		unsigned int ab:1;
		/* pad to the next byte */
		unsigned int pad:3;
	};
	dt_spfl_t spfl:8;
};
#endif


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
DECLF struct dt_dt_s
dt_strpdt(const char *str, const char *fmt, char **ep);

/**
 * Like strftime() for our dates */
DECLF size_t
dt_strfdt(char *restrict buf, size_t bsz, const char *fmt, struct dt_dt_s);

/**
 * Parse durations as in 1w5d, etc. */
DECLF struct dt_dt_s
dt_strpdtdur(const char *str, char **ep);

/**
 * Print a duration. */
DECLF size_t
dt_strfdtdur(char *restrict buf, size_t bsz, const char *fmt, struct dt_dt_s);

/**
 * Negate the duration. */
DECLF struct dt_dt_s dt_neg_dtdur(struct dt_dt_s);

/**
 * Is duration DUR negative? */
DECLF int dt_dtdur_neg_p(struct dt_dt_s dur);

/**
 * Like time() but return the current date in the desired format. */
DECLF struct dt_dt_s dt_datetime(dt_dttyp_t dttyp);

/**
 * Convert D to another calendric system, specified by TGTTYP. */
DECLF struct dt_dt_s dt_dtconv(dt_dttyp_t tgttyp, struct dt_dt_s);

/**
 * Add duration DUR to date/time D.
 * The result will be in the calendar as specified by TGTTYP, or if
 * DT_UNK is given, the calendar of D will be used. */
DECLF struct dt_dt_s
dt_dtadd(struct dt_dt_s d, struct dt_dt_s dur);

/**
 * Get duration between D1 and D2.
 * The result will be of type TGTTYP,
 * the calendar of D1 will be used, e.g. its month-per-year, days-per-week,
 * etc. conventions count.
 * If instead D2 should count, swap D1 and D2 and negate the duration
 * by setting/clearing the neg bit. */
DECLF struct dt_dt_s
dt_dtdiff(dt_dttyp_t tgttyp, struct dt_dt_s d1, struct dt_dt_s d2);

/**
 * Compare two dates, yielding 0 if they are equal, -1 if D1 is older,
 * 1 if D1 is younger than the D2. */
DECLF int dt_dtcmp(struct dt_dt_s d1, struct dt_dt_s d2);

/**
 * Check if D is in the interval spanned by D1 and D2,
 * 1 if D1 is younger than the D2. */
DECLF int
dt_dt_in_range_p(struct dt_dt_s d, struct dt_dt_s d1, struct dt_dt_s d2);

/* more specific but still useful functions */
/**
 * Transform named format strings in FMT to their flag notation.
 * E.g. ymd -> %FT%T */
DECLF void __trans_dtfmt(const char **fmt);


/* some useful gimmicks, sort of */
static inline struct dt_dt_s
__attribute__((pure, const))
dt_dt_initialiser(void)
{
#if defined __C1X
	struct dt_dt_s res = {
		.typ = DT_UNK,
		.sandwich = 0U,
		.dur = 0U,
		.neg = 0U,
		.tai = 0U,
		.u = 0U,
		.t = {
			.typ = DT_TUNK,
			.dur = 0U,
			.neg = 0U,
			.u = 0U,
		},
	};
#else  /* !__C1X */
	struct dt_dt_s res;
#endif	/* __C1X */

#if !defined __C1X
	res.typ = DT_UNK;
	res.sandwich = 0U;
	res.dur = 0U;
	res.neg = 0U;
	res.tai = 0U;
	res.u = 0U;

	res.t.typ = DT_TUNK;
	res.t.dur = 0U;
	res.t.neg = 0U;
	res.t.u = 0U;
#endif	/* !__C1X */
	return res;
}

static inline bool
dt_unk_p(struct dt_dt_s d)
{
	return !(d.sandwich || d.typ > DT_UNK);
}

static inline bool
dt_sandwich_p(struct dt_dt_s d)
{
	return d.sandwich && d.d.typ > DT_DUNK;
}

static inline bool
dt_sandwich_only_d_p(struct dt_dt_s d)
{
	return !d.sandwich && d.d.typ > DT_DUNK && d.d.typ < DT_NDTYP;
}

static inline bool
dt_sandwich_only_t_p(struct dt_dt_s d)
{
	return d.sandwich && d.typ == DT_UNK;
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


#if defined INCLUDE_DATETIME_CORE_IMPL
# include "date-core.c"
# include "time-core.c"
# include "dt-core.c"
#endif	/* INCLUDE_DATETIME_CORE_IMPL */

#if defined __cplusplus
}
#endif	/* __cplusplus */

#endif	/* INCLUDED_dt_core_h_ */
