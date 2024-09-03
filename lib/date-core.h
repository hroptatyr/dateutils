/*** date-core.h -- our universe of dates
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

#if !defined INCLUDED_date_core_h_
#define INCLUDED_date_core_h_

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include "boops.h"
#include "token.h"

#if defined __cplusplus
extern "C" {
#endif	/* __cplusplus */

/**
 * Date types we support.
 * Must not be more than 16. */
typedef enum {
	DT_DUNK,
	DT_YMD,
	DT_YMCW,
	DT_BIZDA,
	DT_YWD,
	DT_YD,
	DT_DAISY,
	DT_BIZSI,
	DT_JDN,
	DT_LDN,
	DT_MDN,
	DT_UMMULQURA,
	DT_NDTYP,
} dt_dtyp_t;

/**
 * Duration types we support.
 * Must not be more than 16. */
typedef enum {
	DT_DURUNK,
	/* compacted durations */
	DT_DURYMD,
	DT_DURYMCW,
	DT_DURBIZDA,
	DT_DURYWD,
	DT_DURYD,
	/* value-unit designated durations */
	DT_DURD,
	DT_DURBD,
	DT_DURWK,
	DT_DURMO,
	DT_DURQU,
	DT_DURYR,
	DT_NDURTYP,
} dt_durtyp_t;

#if defined WITH_FAST_ARITH
# define DT_MIN_YEAR	(1917)
# define DT_MAX_YEAR	(2099)
#else
# define DT_MIN_YEAR	(1601)
# define DT_MAX_YEAR	(4095)
#endif	/* WITH_FAST_ARITH */

/** ymds
 * ymds are just bcd coded concatenations of 8601 dates */
typedef union {
	uint32_t u;
	struct {
#if BYTE_ORDER == BIG_ENDIAN
		/* 10 bits left, make ym coincide with ym from ymcw */
		unsigned int:10;
		unsigned int y:12;
		unsigned int m:4;
		unsigned int d:6;
#elif BYTE_ORDER == LITTLE_ENDIAN
		unsigned int d:6;
		unsigned int m:4;
		unsigned int y:12;
		/* 10 bits left, make ym coincide with ym from ymcw */
		unsigned int:10;
#else
# warning unknown byte order
#endif	/* BYTE_ORDER */
	};
} dt_ymd_t;

typedef dt_ymd_t dt_ummulqura_t;

/** ymcws
 * ymcws are year-month-count-weekday bcd coded. */
typedef union {
	uint32_t u;
	struct {
#if BYTE_ORDER == BIG_ENDIAN
		/* 10 bits left, make ym coincide with ym from ymd */
		unsigned int:10;
		unsigned int y:12;
		unsigned int m:4;
		unsigned int c:3;
		unsigned int w:3;
#elif BYTE_ORDER == LITTLE_ENDIAN
		unsigned int w:3;
		unsigned int c:3;
		unsigned int m:4;
		unsigned int y:12;
		/* 10 bits left, make ym coincide with ym from ymd */
		unsigned int:10;
#else
# warning unknown byte order
#endif	/* BYTE_ORDER */
	};
} dt_ymcw_t;

/** ywds
 * ywds are ISO 8601's year-week-day calendars.
 * By coincidence ycw's y and w slots are accessible through the ymcw bit field,
 * whether that's useful or not will occur to us later and then we might change
 * the layout.
 * Also, there's one auxiliary parameter the number of overhanging days
 * before the first day (Mon) in the first week, this number is in the
 * range of -3 to 3.  For a year to start on Sunday it's +1, for a year
 * to start on Tuesday it's -1. */
typedef union {
	uint32_t u;
	struct {
#define YWD_SUNWK_CNT	(0)
#define YWD_MONWK_CNT	(1)
#define YWD_ISOWK_CNT	(2)
#define YWD_ABSWK_CNT	(3)
#if BYTE_ORDER == BIG_ENDIAN
		/* 8 bits left */
		unsigned int:7;
		unsigned int y:12;
		unsigned int c:7;
		unsigned int w:3;
		signed int hang:3;
#elif BYTE_ORDER == LITTLE_ENDIAN
		signed int hang:3;
		unsigned int w:3;
		unsigned int c:7;
		unsigned int y:12;
		/* 8 bits left */
		unsigned int:7;
#else
# warning unknown byte order
#endif	/* BYTE_ORDER */
	};
} dt_ywd_t;

typedef union {
	uint16_t u;
	uint32_t bs:16;
	struct {
		/* counting convention */
		unsigned int cc:2;
		unsigned int:14;
	};
} __attribute__((__packed__)) dt_ywd_param_t;

/** yds
 * yds are pure helpers and don't exist in the wild. */
typedef union {
	uint32_t u;
	struct {
#if BYTE_ORDER == BIG_ENDIAN
		unsigned int y:16U;
		signed int d:16U;
#elif BYTE_ORDER == LITTLE_ENDIAN
		signed int d:16U;
		unsigned int y:16U;
#else
# warning unknown byte order
#endif	/* BYTE_ORDER */
	};
} dt_yd_t;

/** daysi
 * daisys are days since X, <DT_MIN_YEAR>-01-00 here */
typedef uint32_t dt_daisy_t;
#define DT_DAISY_BASE_YEAR	(DT_MIN_YEAR)
/* and a signed version */
typedef int32_t dt_sdaisy_t;

/** jdn (julian day number)
 * julian days are whole solar days since noon 1 Jan 4713 BC.
 * We will mostly use the daisy type for this. */
typedef float dt_jdn_t;

/** ldn (lilian day number)
 * lilian days are whole solar days since the inception of the Gregorian
 * calendar, i.e. 15 Oct 1582.
 * We will mostly use the daisy type for this. */
typedef dt_daisy_t dt_ldn_t;

/** mdn (matlab day number)
 * matlab days are whole proleptic solar days since 01 Jan 0.
 * We will mostly use the daisy type for this. */
typedef dt_daisy_t dt_mdn_t;

/** bizda
 * bizdas is a calendar that counts business days before or after a
 * certain day in the month, mostly ultimo. */
typedef union {
	uint32_t u;
	struct {
#define BIZDA_AFTER	(0U)/*>*/
#define BIZDA_BEFORE	(1U)/*<*/
#define BIZDA_ULTIMO	(0U)
#if BYTE_ORDER == BIG_ENDIAN
		/* 5 bits left */
		unsigned int:5;
		/* business day */
		unsigned int y:12;
		unsigned int m:4;
		unsigned int bd:5;
#elif BYTE_ORDER == LITTLE_ENDIAN
		/* business day */
		unsigned int bd:5;
		unsigned int m:4;
		unsigned int y:12;
		/* 5 bits left */
		unsigned int:5;
#else
# warning unknown byte order
#endif	/* BYTE_ORDER */
	};
} dt_bizda_t;

typedef union {
	uint16_t u;
	uint32_t bs:16;
	struct {
		/* before or after */
		unsigned int ab:1;
		/* reference day, use 00 for ultimo */
		unsigned int ref:5;
		unsigned int:10;
	};
} __attribute__((__packed__)) dt_bizda_param_t;

/**
 * Duration type. */
typedef int32_t dt_dur_t;

/**
 * Collection of all date types. */
struct dt_d_s {
	/* date type */
	dt_dtyp_t typ:4;
	/* unused here, but used by inherited types (e.g. dt_dt_s) */
	uint32_t:3;
	/* error indicator, usually means date has been fixed up */
	uint32_t fix:1;
	/* was duration predicate */
	uint32_t xxx:1;
	/* negated predicate */
	uint32_t neg:1;
	/* fill up to next ui16 boundary */
	uint32_t:6;
	/* for parametrised types */
	uint32_t param:16;
	union {
		uint32_t u;
		dt_ymd_t ymd;
		dt_ymcw_t ymcw;
		dt_ywd_t ywd;
		dt_daisy_t daisy;
		dt_daisy_t bizsi;
		dt_jdn_t jdn;
		dt_ldn_t ldn;
		dt_mdn_t mdn;
		/* all bizdas mixed into this */
		dt_bizda_t bizda;
		/* for helper purposes only */
		dt_yd_t yd;
		/* basically ymd */
		dt_ummulqura_t ummulqura;
	};
};

struct dt_ddur_s {
	/* date duration type */
	dt_durtyp_t durtyp:4;
	/* unused here, but used by dt_dtdur_s */
	uint32_t:3;
	/* error indicator, usually means date has been fixed up */
	uint32_t fix:1;
	/* co-class indicator */
	uint32_t cocl:1;
	/* negated predicate */
	uint32_t neg:1;
	/* fill up to next ui16 boundary */
	uint32_t:6;
	/* for parametrised types */
	uint32_t param:16;
	union {
		uint32_t u;
		dt_ymd_t ymd;
		dt_ymcw_t ymcw;
		dt_ywd_t ywd;
		/* all bizdas mixed into this */
		dt_bizda_t bizda;
		/* duration value, for value+unit durations only,
		 * the .durtyp slot indicates the unit */
		dt_dur_t dv;
		/* for helper purposes only */
		dt_yd_t yd;
	};
};

/* widely understood notion of weekdays */
typedef enum {
	DT_MIRACLEDAY,
	DT_MONDAY,
	DT_TUESDAY,
	DT_WEDNESDAY,
	DT_THURSDAY,
	DT_FRIDAY,
	DT_SATURDAY,
	DT_SUNDAY,
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
#define GREG_MONTHS_P_YEAR	(12U)
#define HIJRI_MONTHS_P_YEAR	(12U)
#define DUWW_BDAYS_P_WEEK	(5U)


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
extern struct dt_d_s
dt_strpd(const char *str, const char *fmt, char **ep);

extern struct dt_d_s
dt_strpd_special(const char *str, dt_dtyp_t, char **ep);

/**
 * Like strftime() for our dates */
extern size_t
dt_strfd(char *restrict buf, size_t bsz, const char *fmt, struct dt_d_s);

/**
 * Parse durations as in 1w5d, etc. */
extern struct dt_ddur_s
dt_strpddur(const char *str, char **ep);

/**
 * Print a duration. */
extern size_t
dt_strfddur(char *restrict buf, size_t bsz, const char *fmt, struct dt_ddur_s);

/**
 * Like time() but return the current date in the desired format. */
extern struct dt_d_s dt_date(dt_dtyp_t outtyp);

/**
 * Convert D to another calendric system, specified by TGTTYP. */
extern struct dt_d_s dt_dconv(dt_dtyp_t tgttyp, struct dt_d_s);

/**
 * Get the year count (gregorian) of a date,
 * calendars without the notion of a year will return 0. */
extern int dt_get_year(struct dt_d_s);

/**
 * Get the month within the year of a date,
 * calendars without the notion of a month will return 0. */
extern int dt_get_mon(struct dt_d_s);

/**
 * Get the weekday of a date. */
extern dt_dow_t dt_get_wday(struct dt_d_s);

/**
 * Get the day of the month of a date. */
extern int dt_get_mday(struct dt_d_s d);

/**
 * Get the business day count of a date in a month. */
extern int dt_get_bday(struct dt_d_s d);

/**
 * Get the business day count of a date in a month Before/After REF. */
extern int dt_get_bday_q(struct dt_d_s d, dt_bizda_param_t bp);

/**
 * Get the quarter number of a date. */
extern int dt_get_quarter(struct dt_d_s d);

/**
 * Get the day of the year of a date.
 * This might only be intuitive for YMD dates.  The formal definition
 * is to find a representation of D that lacks the notion of a month,
 * so for YMD dates this would be the sum of the days in the months
 * preceding M and the current day of the month in M.
 * For YMCW dates this will yield the n-th W-day in Y.
 * For calendars without the notion of a year this will return 0. */
extern unsigned int dt_get_yday(struct dt_d_s d);

/**
 * Return N where N is the week within the year that THIS is in. */
extern int dt_get_wcnt_year(struct dt_d_s this, unsigned int wkcnt_convention);

/**
 * Return N where N is the week within the month that THIS is in. */
extern int dt_get_wcnt_mon(struct dt_d_s this);

/* converters */
extern dt_daisy_t dt_conv_to_daisy(struct dt_d_s);

extern dt_ymd_t __daisy_to_ymd(dt_daisy_t);

/* adders */
/**
 * Add duration DUR to date D. */
extern struct dt_d_s
dt_dadd(struct dt_d_s d, struct dt_ddur_s dur);

/**
 * Add N (gregorian) days to date D. */
extern struct dt_d_s
dt_dadd_d(struct dt_d_s d, int n);

/**
 * Add N business days to date D. */
extern struct dt_d_s
dt_dadd_b(struct dt_d_s d, int n);

/**
 * Add N weeks to date D. */
extern struct dt_d_s
dt_dadd_w(struct dt_d_s d, int n);

/**
 * Add N months to date D.
 * For calendars without the notion of months the result is D. */
extern struct dt_d_s
dt_dadd_m(struct dt_d_s d, int n);

/**
 * Add N years to date D.
 * For calendars without the notion of years the result is D. */
extern struct dt_d_s
dt_dadd_y(struct dt_d_s d, int n);

/**
 * Negate the duration. */
extern struct dt_ddur_s dt_neg_dur(struct dt_ddur_s);

/**
 * Is duration DUR negative? */
extern int dt_dur_neg_p(struct dt_ddur_s dur);

/**
 * Get duration between D1 and D2.
 * The result will be of type TGTTYP,
 * the calendar of D1 will be used, e.g. its month-per-year, days-per-week,
 * etc. conventions count.
 * If instead D2 should count, swap D1 and D2 and negate the duration
 * by setting/clearing the neg bit. */
extern struct dt_ddur_s
dt_ddiff(dt_durtyp_t tgttyp, struct dt_d_s d1, struct dt_d_s d2, int carry);

/**
 * Compare two dates, yielding 0 if they are equal, -1 if D1 is older,
 * 1 if D1 is younger than the D2. */
extern int dt_dcmp(struct dt_d_s d1, struct dt_d_s d2);

/**
 * Check if D is in the interval spanned by D1 and D2,
 * 1 if D1 is younger than the D2. */
extern int dt_d_in_range_p(struct dt_d_s d, struct dt_d_s d1, struct dt_d_s d2);

#if defined LIBDUT
/**
 * Return the base date/time as struct dt_d_s.
 * Defined in dt-core.c */
extern struct dt_d_s dt_get_dbase(void);
#endif	/* LIBDUT */

/**
 * Crop dates with days beyond ultimo. */
extern __attribute__((const)) struct dt_d_s dt_dfixup(struct dt_d_s);


/* other ctors */
static inline struct dt_d_s
dt_make_ymd(unsigned int y, unsigned int m, unsigned int d)
{
	struct dt_d_s res;

	res.typ = DT_YMD;
	res.neg = 0U;
	res.fix = 0U;
	res.param = 0U;
	res.ymd.y = y;
	res.ymd.m = m;
	res.ymd.d = d;
	return res;
}

static inline struct dt_d_s
dt_make_ymcw(unsigned int y, unsigned int m, unsigned int c, unsigned int w)
{
	struct dt_d_s res;

	res.typ = DT_YMCW;
	res.neg = 0U;
	res.fix = 0U;
	res.param = 0U;
	res.ymcw.y = y;
	res.ymcw.m = m;
	res.ymcw.c = c;
	res.ymcw.w = w;
	return res;
}

static inline struct dt_ddur_s
dt_make_ddur(dt_durtyp_t typ, dt_dur_t d)
{
#if defined HAVE_ANON_STRUCTS_INIT
	return (struct dt_ddur_s){typ, .dv = d};
#else
	struct dt_ddur_s res = {typ};
	res.dv = d;
	return res;
#endif
}

static inline dt_bizda_param_t
__get_bizda_param(struct dt_d_s that)
{
	dt_bizda_param_t p;
	p.bs = that.param;
	return p;
}

static inline dt_bizda_param_t
__make_bizda_param(unsigned int ab, unsigned int ref)
{
	dt_bizda_param_t p;
	p.ab = ab;
	p.ref = ref;
	return p;
}

static inline dt_ywd_param_t
__get_ywd_param(struct dt_d_s that)
{
	return (dt_ywd_param_t){.bs = that.param};
}

static inline dt_ywd_param_t
__make_ywd_param(unsigned int cc)
{
	dt_ywd_param_t p;
	p.cc = cc;
	return p;
}

static inline bool
__leapp(unsigned int y)
{
#if defined WITH_FAST_ARITH
	return y % 4 == 0;
#else  /* !WITH_FAST_ARITH */
	return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
#endif	/* WITH_FAST_ARITH */
}

#if defined __cplusplus
}
#endif	/* __cplusplus */

#endif	/* INCLUDED_date_core_h_ */
