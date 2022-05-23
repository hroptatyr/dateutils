/*** leaps.c -- materialised leap seconds
 *
 * Copyright (C) 2012-2022 Sebastian Freundt
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
/* implementation part of leaps.h */
#if !defined INCLUDED_leaps_c_
#define INCLUDED_leaps_c_

#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdint.h>
#include <limits.h>
#include "nifty.h"
#include "leaps.h"

typedef ssize_t sidx_t;

#if !defined DEFUN
# define DEFUN
#endif	/* !DEFUN */


/* this can be called roughly 100m/sec */
#define DEF_FIND_BEFORE(N, X)					\
static zidx_t							\
find_before_##N(						\
	const X v[], size_t nv, X key, zidx_t i, zidx_t min, zidx_t max) \
{								\
/* Given key K find the index of the transition before */	\
	do {							\
		X lo, up;					\
								\
		lo = v[i];					\
		up = v[i + 1];					\
								\
		if (key > lo && key <= up) {			\
			/* found him */				\
			break;					\
		} else if (key > up) {				\
			min = i + 1;				\
			i = (i + max) / 2;			\
		} else if (key <= lo) {				\
			max = i - 1;				\
			i = (i + min) / 2;			\
		}						\
	} while (max > min && i < nv);				\
	return i;						\
}								\
static const int UNUSED(defined_find_before_##name##_p)

DEF_FIND_BEFORE(ui32, uint32_t);
DEF_FIND_BEFORE(si32, int32_t);
DEF_FIND_BEFORE(ui64, uint64_t);
DEF_FIND_BEFORE(si64, int64_t);


/* public apis */
DEFUN zidx_t
leaps_before_ui32(const uint32_t fld[], size_t nfld, uint32_t key)
{
	zidx_t min = 0;
	zidx_t max = nfld - 1;
	zidx_t this = max / 2;

	return find_before_ui32(fld, nfld, key, this, min, max);
}

DEFUN zidx_t
leaps_before_si32(const int32_t fld[], size_t nfld, int32_t key)
{
	zidx_t min = 0;
	zidx_t max = nfld - 1;
	zidx_t this = max / 2;

	return find_before_si32(fld, nfld, key, this, min, max);
}

DEFUN zidx_t
leaps_before_ui64(const uint64_t fld[], size_t nfld, uint64_t key)
{
	zidx_t min = 0;
	zidx_t max = nfld - 1;
	zidx_t this = max / 2;

	return find_before_ui64(fld, nfld, key, this, min, max);
}

DEFUN zidx_t
leaps_before_si64(const int64_t fld[], size_t nfld, int64_t key)
{
	zidx_t min = 0;
	zidx_t max = nfld - 1;
	zidx_t this = max / 2;

	return find_before_si64(fld, nfld, key, this, min, max);
}

#endif	/* INCLUDED_leaps_c_ */
/* leaps.c ends here */
