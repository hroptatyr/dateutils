/*** date-core.h -- our universe of dates
 *
 * Copyright (C) 2011 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of datetools.
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

#if defined __cplusplus
extern "C" {
#endif	/* __cplusplus */

#if !defined DECLF
# define DECLF	static
# define DEFUN	static
# define INCLUDE_DATE_CORE_IMPL
#elif !defined DEFUN
# define DEFUN
#endif	/* !DECLF */

typedef enum {
	DT_UNK,
	DT_YMD,
	DT_YMCD,
} dt_dtyp_t;

/** ymds
 * ymds are just bcd coded concatenations of 8601 dates */
typedef union {
	uint32_t u;
	struct {
		int d:5;
		int m:4;
		int y:16;
		/* 7 bits left */
	};
} dt_ymd_t;

/** ymcds
 * ymcds are year-month-count-weekday bcd coded. */
typedef union {
	uint32_t u;
	struct {
		int d:3;
		int c:3;
		int m:4;
		int y:16;
		/* 6 bits left */
	};
} dt_ymcd_t;

struct dt_d_s {
	dt_dtyp_t typ;
	union {
		uint32_t u;
		dt_ymd_t ymd;
		dt_ymcd_t ymcd;
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


/* decls */
/**
 * Like strptime() for our dates.
 * The format characters are _NOT_ compatible with strptime().
 * This is what we support:
 * %F - alias for %Y-%m-%d
 * %Y - year in ymd and ymcd mode
 * %m - month in ymd and ymcd mode
 * %d - day in ymd mode
 * %c - week of the month in ymcd mode
 * %w - numeric weekday in ymcd mode
 *
 * If FMT is NULL the standard format for each calendric system is used,
 * that is:
 * - %Y-%m-%d for YMD dates
 * - %Y-%m-%c-%d for YMCD dates
 * - %Y-%m-%d%u for YMDU dates */
DECLF struct dt_d_s dt_strpd(const char *str, const char *fmt);
/**
 * Like strftime() for our dates */
DECLF size_t
dt_strfd(char *restrict buf, size_t bsz, const char *fmt, struct dt_d_s d);


#if defined INCLUDE_DATE_CORE_IMPL
# include "date-core.c"
#endif	/* INCLUDE_DATE_CORE_IMPL */

#if defined __cplusplus
}
#endif	/* __cplusplus */

#endif	/* INCLUDED_date_core_h_ */
