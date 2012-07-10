/*** leaps.h -- materialised leap seconds
 *
 * Copyright (C) 2012 Sebastian Freundt
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
 ***/
#if !defined INCLUDED_leaps_h_
#define INCLUDED_leaps_h_

#include <unistd.h>
#include <stdint.h>

#if defined __cplusplus
extern "C" {
#endif	/* __cplusplus */

#if !defined DECLF
# define DECLF	static __attribute__((unused))
# define DEFUN	static
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

typedef const struct zleap_s *zleap_t;
typedef const int32_t *zltr_t;
typedef size_t zidx_t;

/* row-based */
struct zleap_s {
	union {
		uint32_t u;
		int32_t v;
	};
	int32_t corr;
};

/* row based funs */
/**
 * Return the number of leap seconds *inserted* between D1 and D2.
 * If the result is negative then leap seconds have been removed. */
DECLF int leaps_between(zleap_t lv, size_t nlv, uint32_t d1, uint32_t d2);

/**
 * Return the number of leap seconds inserted (>0) or removed (<0) on D. */
DECLF int leaps_on(zleap_t lv, size_t nlv, uint32_t d);

/**
 * Return the number of leap seconds ever inserted till D. */
DECLF int leaps_till(zleap_t lv, size_t nlv, uint32_t d);

/**
 * Return the number of leap seconds inserted after D. */
DECLF int leaps_since(zleap_t lv, size_t nlv, uint32_t d);

/**
 * Return the number of leap seconds *inserted* between D1 and D2.
 * If the result is negative then leap seconds have been removed. */
DECLF int leaps_sbetween(zleap_t lv, size_t nlv, int32_t d1, int32_t d2);

/**
 * Return the number of leap seconds inserted (>0) or removed (<0) on D. */
DECLF int leaps_son(zleap_t lv, size_t nlv, int32_t d);

/**
 * Return the number of leap seconds ever inserted till D. */
DECLF int leaps_still(zleap_t lv, size_t nlv, int32_t d);

/**
 * Return the number of leap seconds inserted after D. */
DECLF int leaps_ssince(zleap_t lv, size_t nlv, int32_t d);

/* col-based funs */
/**
 * Return last leap transition before KEY in a uint32_t field FLD. */
DECLF zidx_t leaps_before_ui32(const uint32_t fld[], size_t nfld, uint32_t key);

/**
 * Return last leap transition before KEY in a int32_t field FLD. */
DECLF zidx_t leaps_before_si32(const int32_t fld[], size_t nfld, int32_t key);

/**
 * Return last leap transition before KEY in a uint64_t field FLD. */
DECLF zidx_t leaps_before_ui64(const uint64_t fld[], size_t nfld, uint64_t key);

/**
 * Return last leap transition before KEY in a int64_t field FLD. */
DECLF zidx_t leaps_before_si64(const int64_t fld[], size_t nfld, int64_t key);


#if defined INCLUDE_LEAPS_IMPL
# include "leaps.c"
#endif	/* INCLUDE_LEAPS_IMPL */

#if defined __cplusplus
}
#endif	/* __cplusplus */

#endif	/* INCLUDED_leaps_h_ */
