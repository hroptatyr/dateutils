/*** date-core.h -- our universe of dates
 *
 * Copyright (C) 2011 Sebastian Freundt
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

#if !defined INCLUDED_date_core_h_
#define INCLUDED_date_core_h_

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "token.h"

#if defined __cplusplus
extern "C" {
#endif	/* __cplusplus */

#if !defined DECLF
# define DECLF	static __attribute__((unused))
# define DEFUN	static
# define INCLUDE_DATE_CORE_IMPL
#elif !defined DEFUN
# define DEFUN
#endif	/* !DECLF */
#if !defined restrict
# define restrict	__restrict
#endif	/* !restrict */

typedef enum {
	DT_UNK,
	DT_YMD,
	DT_YMCW,
	DT_BIZDA,
	DT_DAISY,
	DT_BIZSI,
} dt_dtyp_t;

#define DT_MIN_YEAR	(0)
#define DT_MAX_YEAR	(4095)

/** ymds
 * ymds are just bcd coded concatenations of 8601 dates */
typedef union {
	uint32_t u;
	struct {
		unsigned int d:5;
		unsigned int m:4;
		unsigned int y:12;
		/* 11 bits left */
		unsigned int pad:11;
	};
} dt_ymd_t;

/** ymcws
 * ymcws are year-month-count-weekday bcd coded. */
typedef union {
	uint32_t u;
	struct {
		unsigned int w:3;
		unsigned int c:3;
		unsigned int m:4;
		unsigned int y:12;
		/* 10 bits left */
		unsigned int pad:10;
	};
} dt_ymcw_t;

/** daysi
 * daisys are days since X, 1917-01-01 here */
typedef uint32_t dt_daisy_t;
#define DT_DAISY_BASE_YEAR	(1917)

/** bizda
 * bizdas is a calendar that counts business days before or after a
 * certain day in the month, mostly ultimo. */
typedef union {
	uint32_t u;
	struct {
#define BIZDA_AFTER	(0U)/*>*/
#define BIZDA_BEFORE	(1U)/*<*/
#define BIZDA_ULTIMO	(0U)
		/* business day */
		unsigned int bd:5;
		unsigned int m:4;
		unsigned int y:12;
		/* 5 bits left */
		unsigned int pad:5;
	};
} dt_bizda_t;

typedef union {
	uint32_t u:16;
	struct {
		/* before or after */
		unsigned int ab:1;
		/* reference day, use 00 for ultimo */
		unsigned int ref:5;
	};
} dt_bizda_param_t;

/**
 * Collection of all date types. */
struct dt_d_s {
	/* for parametrised types */
	dt_dtyp_t typ:14;
	uint16_t dur:1;
	uint16_t neg:1;
	uint32_t param:16;
	union {
		uint32_t u;
		dt_ymd_t ymd;
		dt_ymcw_t ymcw;
		dt_daisy_t daisy;
		dt_daisy_t bizsi;
		/* all bizdas mixed into this */
		dt_bizda_t bizda;
	};
};

/* widely understood notion of weekdays */
typedef enum {
	DT_SUNDAY,
	DT_MONDAY,
	DT_TUESDAY,
	DT_WEDNESDAY,
	DT_THURSDAY,
	DT_FRIDAY,
	DT_SATURDAY,
	DT_MIRACLEDAY
} dt_dow_t;

/* match operations */
typedef uint32_t/*:3*/ oper_t;

enum {
	OP_UNK = 0,
	OP_FALSE = OP_UNK,
	/* bit 1 set */
	OP_EQ,
	/* bit 2 set */
	OP_LT,
	OP_LE,
	/* bit 3 set */
	OP_GT,
	OP_GE,
	/* bits 2 and 3 set */
	OP_NE,
	/* bits 1, 2 and 3 set */
	OP_TRUE,
};


/* constants (for known calendars) */
#define GREG_DAYS_P_WEEK	(7U)


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
DECLF struct dt_d_s
dt_strpd(const char *str, const char *fmt, char **ep);

/**
 * Like strftime() for our dates */
DECLF size_t
dt_strfd(char *restrict buf, size_t bsz, const char *fmt, struct dt_d_s);

/**
 * Parse durations as in 1w5d, etc. */
DECLF struct dt_d_s
dt_strpdur(const char *str, char **ep);

/**
 * Print a duration. */
DECLF size_t
dt_strfddur(char *restrict buf, size_t bsz, const char *fmt, struct dt_d_s);

/**
 * Like time() but return the current date in the desired format. */
DECLF struct dt_d_s dt_date(dt_dtyp_t outtyp);

/**
 * Convert D to another calendric system, specified by TGTTYP. */
DECLF struct dt_d_s dt_conv(dt_dtyp_t tgttyp, struct dt_d_s);

/**
 * Get the year count (gregorian) of a date,
 * calendars without the notion of a year will return 0. */
DECLF int dt_get_year(struct dt_d_s);

/**
 * Get the month within the year of a date,
 * calendars without the notion of a month will return 0. */
DECLF int dt_get_mon(struct dt_d_s);

/**
 * Get the weekday of a date. */
DECLF dt_dow_t dt_get_wday(struct dt_d_s);

/**
 * Get the day of the month of a date. */
DECLF int dt_get_mday(struct dt_d_s d);

/**
 * Get the business day count of a date in a month. */
DECLF int dt_get_bday(struct dt_d_s d);

/**
 * Get the business day count of a date in a month Before/After REF. */
DECLF int dt_get_bday_q(struct dt_d_s d, dt_bizda_param_t bp);

/**
 * Get the quarter number of a date. */
DECLF int dt_get_quarter(struct dt_d_s d);

/**
 * Get the day of the year of a date.
 * This might only be intuitive for YMD dates.  The formal definition
 * is to find a representation of D that lacks the notion of a month,
 * so for YMD dates this would be the sum of the days in the months
 * preceding M and the current day of the month in M.
 * For YMCW dates this will yield the n-th W-day in Y.
 * For calendars without the notion of a year this will return 0. */
DECLF unsigned int dt_get_yday(struct dt_d_s d);

/**
 * Add duration DUR to date D.
 * The result will be in the calendar as specified by TGTTYP, or if
 * DT_UNK is given, the calendar of D will be used. */
DECLF struct dt_d_s
dt_dadd(struct dt_d_s d, struct dt_d_s dur);

/**
 * Negate the duration. */
DECLF struct dt_d_s dt_neg_dur(struct dt_d_s);

/**
 * Is duration DUR negative? */
DECLF int dt_dur_neg_p(struct dt_d_s dur);

/**
 * Get duration between D1 and D2.
 * The result will be of type TGTTYP,
 * the calendar of D1 will be used, e.g. its month-per-year, days-per-week,
 * etc. conventions count.
 * If instead D2 should count, swap D1 and D2 and negate the duration
 * by setting/clearing the neg bit. */
DECLF struct dt_d_s
dt_ddiff(dt_dtyp_t tgttyp, struct dt_d_s d1, struct dt_d_s d2);

/**
 * Compare two dates, yielding 0 if they are equal, -1 if D1 is older,
 * 1 if D1 is younger than the D2. */
DECLF int dt_cmp(struct dt_d_s d1, struct dt_d_s d2);

/**
 * Check if D is in the interval spanned by D1 and D2,
 * 1 if D1 is younger than the D2. */
DECLF int dt_in_range_p(struct dt_d_s d, struct dt_d_s d1, struct dt_d_s d2);


#if defined INCLUDE_DATE_CORE_IMPL
# include "date-core.c"
#endif	/* INCLUDE_DATE_CORE_IMPL */

#if defined __cplusplus
}
#endif	/* __cplusplus */

#endif	/* INCLUDED_date_core_h_ */
