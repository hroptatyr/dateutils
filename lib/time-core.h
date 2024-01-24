/*** time-core.h -- our universe of times
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

#if !defined INCLUDED_time_core_h_
#define INCLUDED_time_core_h_

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "boops.h"
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
#if BYTE_ORDER == BIG_ENDIAN
		uint32_t u24:24;
		uint32_t:32;
#elif BYTE_ORDER == LITTLE_ENDIAN
		uint32_t:32;
		uint32_t u24:24;
#else
# warning unknown byte order
#endif	/* BYTE_ORDER */
	} __attribute__((packed));
	struct {
#if BYTE_ORDER == BIG_ENDIAN
		uint64_t h:8;
		uint64_t m:8;
		uint64_t s:8;
		uint64_t ns:32;
#elif BYTE_ORDER == LITTLE_ENDIAN
		uint64_t ns:32;
		uint64_t s:8;
		uint64_t m:8;
		uint64_t h:8;
#else
# warning unknown byte order
#endif	/* BYTE_ORDER */
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
		struct {
			signed int sdur:26;
			unsigned int nsdur:30;
		} __attribute__((packed));
		dt_hms_t hms;
	} __attribute__((packed));
};


/* helpers */
#if !defined NANOS_PER_SEC
# define NANOS_PER_SEC		(1000 * 1000 * 1000)
#endif	/* !SECS_PER_MIN */
#if !defined SECS_PER_MIN
# define SECS_PER_MIN		(60)
#endif	/* !SECS_PER_MIN */
#if !defined MINS_PER_HOUR
# define MINS_PER_HOUR		(60)
#endif	/* !MINS_PER_HOUR */
#if !defined HOURS_PER_DAY
# define HOURS_PER_DAY		(24)
#endif	/* !HOURS_PER_DAY */
#if !defined SECS_PER_HOUR
# define SECS_PER_HOUR		(SECS_PER_MIN * MINS_PER_HOUR)
#endif	/* !SECS_PER_HOUR */
#if !defined SECS_PER_DAY
# define SECS_PER_DAY		(SECS_PER_HOUR * HOURS_PER_DAY)
#endif	/* !SECS_PER_DAY */


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
 * Add DURS seconds and CORR corrections to T. */
extern struct dt_t_s dt_tadd_s(struct dt_t_s t, int durs, int corr);

/**
 * Compute the duration between T1 and T2 (as in T2 - T1). */
extern int dt_tdiff_s(struct dt_t_s t1, struct dt_t_s t2);

/**
 * Compute the duration between T1 and T2 (as in T2 - T1). */
extern int64_t dt_tdiff_ns(struct dt_t_s t1, struct dt_t_s t2);

/**
 * Compare two time values, yielding 0 if they are equal, -1 if T1 is older,
 * 1 if T1 is younger than the T2. */
extern int dt_tcmp(struct dt_t_s t1, struct dt_t_s t2);

/**
 * Like time() but always return the current UTC time. */
extern struct dt_t_s dt_time(void);

#if defined LIBDUT
/**
 * Return the base date/time as struct dt_t_s.
 * Defined in dt-core.c */
extern struct dt_t_s dt_get_tbase(void);
#endif	/* LIBDUT */


/* some useful gimmicks, sort of */
static inline unsigned int
__secs_since_midnight(struct dt_t_s t)
{
	return (t.hms.h * MINS_PER_HOUR + t.hms.m) * SECS_PER_MIN + t.hms.s;
}

#if defined __cplusplus
}
#endif	/* __cplusplus */

#endif	/* INCLUDED_time_core_h_ */
