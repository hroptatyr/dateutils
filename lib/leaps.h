/*** leaps.h -- materialised leap seconds
 *
 * Copyright (C) 2012-2024 Sebastian Freundt
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

/* col-based funs */
/**
 * Return last leap transition before KEY in a uint32_t field FLD. */
extern zidx_t leaps_before_ui32(const uint32_t fld[], size_t nfld, uint32_t key);

/**
 * Return last leap transition before KEY in a int32_t field FLD. */
extern zidx_t leaps_before_si32(const int32_t fld[], size_t nfld, int32_t key);

/**
 * Return last leap transition before KEY in a uint64_t field FLD. */
extern zidx_t leaps_before_ui64(const uint64_t fld[], size_t nfld, uint64_t key);

/**
 * Return last leap transition before KEY in a int64_t field FLD. */
extern zidx_t leaps_before_si64(const int64_t fld[], size_t nfld, int64_t key);

#if defined __cplusplus
}
#endif	/* __cplusplus */

#endif	/* INCLUDED_leaps_h_ */
