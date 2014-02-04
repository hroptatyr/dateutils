/*** time-core.h -- our universe of times
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

#if !defined INCLUDED_time_core_h_
#define INCLUDED_time_core_h_

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "token.h"

#if defined __cplusplus
extern "C" {
#endif	/* __cplusplus */

typedef enum {
	DT_TUNK,
#define DT_TUNK		(dt_ttyp_t)(DT_TUNK)
	DT_HMS,
#define DT_HMS		(dt_ttyp_t)(DT_HMS)
	DT_NTTYP,
} dt_ttyp_t;

/** hms
 * hms times are just bog-standard hour, minute, second times */
typedef union {
	uint64_t u:56;
	struct {
#if defined WORDS_BIGENDIAN
		uint32_t u24:24;
		uint32_t:32;
#else  /* !WORDS_BIGENDIAN */
		uint32_t:32;
		uint32_t u24:24;
#endif	/* WORDS_BIGENDIAN */
	} __attribute__((packed));
	struct {
#if defined WORDS_BIGENDIAN
		uint64_t h:8;
		uint64_t m:8;
		uint64_t s:8;
		uint64_t ns:32;
#else  /* !WORDS_BIGENDIAN */
		uint64_t ns:32;
		uint64_t s:8;
		uint64_t m:8;
		uint64_t h:8;
#endif	/* WORDS_BIGENDIAN */
	} __attribute__((packed));
} __attribute__((packed)) dt_hms_t;

/**
 * Collection of all time types. */
struct dt_t_s {
	struct {
		dt_ttyp_t typ:2;
		uint64_t dur:1;
		uint64_t neg:1;
		/* used for tadd operations and whatnot, range [-7,7] */
		int64_t carry:4;
	} __attribute__((packed));
	union {
		uint64_t u:56;
		signed int sdur;
		dt_hms_t hms;
	} __attribute__((packed));
};


/* helpers */
#if !defined SECS_PER_MIN
# define SECS_PER_MIN		(60U)
#endif	/* !SECS_PER_MIN */
#if !defined MINS_PER_HOUR
# define MINS_PER_HOUR		(60U)
#endif	/* !MINS_PER_HOUR */
#if !defined HOURS_PER_DAY
# define HOURS_PER_DAY		(24U)
#endif	/* !HOURS_PER_DAY */
#if !defined SECS_PER_HOUR
# define SECS_PER_HOUR		(SECS_PER_MIN * MINS_PER_HOUR)
#endif	/* !SECS_PER_HOUR */
#if !defined SECS_PER_DAY
# define SECS_PER_DAY		(SECS_PER_HOUR * HOURS_PER_DAY)
#endif	/* !SECS_PER_DAY */


/* formatting defaults */
extern const char hms_dflt[];

extern void __trans_tfmt(const char **fmt);


/* decls */
/**
 * Like strptime() for our times.
 * The format characters are _NOT_ compatible with strptime().
 * If FMT is NULL the standard format for each calendric system is used,
 * see format.texi or dateutils info page.
 *
 * If optional EP is non-NULL it will point to the end of the parsed
 * date string. */
extern struct dt_t_s
dt_strpt(const char *str, const char *fmt, char **ep);

/**
 * Like strftime() for our times. */
extern size_t
dt_strft(char *restrict buf, size_t bsz, const char *fmt, struct dt_t_s);

/**
 * Add DUR to T and return its result.
 * Optional argument CORR is the number of leap-seconds to insert at
 * the end of the day (or remove if negative). */
extern struct dt_t_s dt_tadd(struct dt_t_s t, struct dt_t_s dur, int corr);

/**
 * Compute the duration between T1 and T2 (as in T2 - T1) and return the
 * result in the .sdur slot. */
extern struct dt_t_s dt_tdiff(struct dt_t_s t1, struct dt_t_s t2);

/**
 * Compare two time values, yielding 0 if they are equal, -1 if T1 is older,
 * 1 if T1 is younger than the T2. */
extern int dt_tcmp(struct dt_t_s t1, struct dt_t_s t2);

/**
 * Like time() but always return the current UTC time. */
extern struct dt_t_s dt_time(void);


/* some useful gimmicks, sort of */
static inline __attribute__((pure, const)) struct dt_t_s
dt_t_initialiser(void)
{
#if defined HAVE_SLOPPY_STRUCTS_INIT
	static const struct dt_t_s res = {};
#else
	static const struct dt_t_s res;
#endif	/* HAVE_SLOPPY_STRUCTS_INIT */
	return res;
}

static inline unsigned int
__secs_since_midnight(struct dt_t_s t)
{
	return (t.hms.h * MINS_PER_HOUR + t.hms.m) * SECS_PER_MIN + t.hms.s;
}

#if defined __cplusplus
}
#endif	/* __cplusplus */

#endif	/* INCLUDED_time_core_h_ */
