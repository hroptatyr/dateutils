/*** dt-core.h -- our universe of datetimes
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

#if !defined INCLUDED_dt_core_h_
#define INCLUDED_dt_core_h_

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#if defined __cplusplus
extern "C" {
#endif	/* __cplusplus */

#if !defined DECLF
# define DECLF	static __attribute__((unused))
# define DEFUN	static
# define INCLUDE_DATETIME_CORE_IMPL
# define INCLUDE_TZRAW_IMPL
#elif !defined DEFUN
# define DEFUN
#endif	/* !DECLF */
#if !defined restrict
# define restrict	__restrict
#endif	/* !restrict */

#include "date-core.h"
#include "time-core.h"

typedef enum {
	DT_SANDWICH = 1,
	DT_SEXY,
} dt_dttyp_t;

/** sandwiches
 * sandwiches are just packs of dates and times */
typedef union {
	uint64_t u:53;
	struct {
		unsigned int d:5;
		unsigned int m:4;
		/* offset by the year 1900 */
#define DT_YEAR_OFFS	(1900)
		unsigned int y:8;
		/* round up to 32 bits, remaining bits are seconds east */
		unsigned int offs:15;

		/* time part */
		unsigned int S:8;
		unsigned int M:8;
		unsigned int H:5;
		/* 11 bits left */
	};
} dt_ymdhms_t;

/** sexy
 * sexy is really, secsi, seconds since X, 1970-01-01T00:00:00 here */
typedef uint64_t dt_sexy_t;
#define DT_SEXY_BASE_YEAR	(1917)

/**
 * Collection of all date types. */
struct __dt_s {
	/* for parametrised types */
	dt_dttyp_t typ:9;
	uint16_t dur:1;
	uint16_t neg:1;
	union {
		uint64_t u:53;
		dt_ymdhms_t ymdhms;
		dt_sexy_t sexy:53;
	};
} __attribute__((packed));

struct dt_dt_s {
	union {
		struct __dt_s x64;
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
 * Like time() but return the current date in the desired format. */
DECLF struct dt_dt_s dt_datetime(dt_dtyp_t outtyp);

/**
 * Convert D to another calendric system, specified by TGTTYP. */
DECLF struct dt_dt_s dt_dtconv(dt_dtyp_t tgttyp, struct dt_dt_s);


#if defined INCLUDE_DATETIME_CORE_IMPL
# include "date-core.c"
# include "time-core.c"
# include "dt-core.c"
#endif	/* INCLUDE_DATETIME_CORE_IMPL */

#if defined __cplusplus
}
#endif	/* __cplusplus */

#endif	/* INCLUDED_dt_core_h_ */
