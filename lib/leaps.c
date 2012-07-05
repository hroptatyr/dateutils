/*** leaps.c -- materialised leap seconds
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
/* implementation part of leaps.h */
#if !defined INCLUDED_leaps_c_
#define INCLUDED_leaps_c_

#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdint.h>
#include <limits.h>
#include "leaps.h"

#if !defined LIKELY
# define LIKELY(_x)	__builtin_expect((_x), 1)
#endif	/* !LIKELY */
#if !defined UNLIKELY
# define UNLIKELY(_x)	__builtin_expect((_x), 0)
#endif	/* !UNLIKELY */

typedef ssize_t sidx_t;


/* this can be called roughly 100m/sec */
static sidx_t
find_uidx(zleap_t lv, size_t nlv, uint32_t c, sidx_t i, sidx_t min, sidx_t max)
{
/* given a daisy C find the index of the transition before CT00:00:00 */
	do {
		uint32_t tl, tu;

		tl = lv[i].u;
		tu = lv[i + 1].u;

		if (c > tl && c <= tu) {
			/* found him */
			return i;
		} else if (c > tu) {
			min = i + 1;
			i = (i + max) / 2;
		} else if (c <= tl) {
			max = i - 1;
			i = (i + min) / 2;
		}
	} while (max - min > 0 && i < (sidx_t)nlv);
	return i;
}

static sidx_t
find_sidx(zleap_t lv, size_t nlv, int32_t c, sidx_t i, sidx_t min, sidx_t max)
{
/* given a daisy C find the index of the transition before CT00:00:00 */
	do {
		int32_t tl, tu;

		tl = lv[i].v;
		tu = lv[i + 1].v;

		if (c > tl && c <= tu) {
			/* found him */
			return i;
		} else if (c > tu) {
			min = i + 1;
			i = (i + max) / 2;
		} else if (c <= tl) {
			max = i - 1;
			i = (i + min) / 2;
		}
	} while (max - min > 0 && i < (sidx_t)nlv);
	return i;
}


/* public apis */
DEFUN int
leaps_between(zleap_t lv, size_t nlv, uint32_t d1, uint32_t d2)
{
	sidx_t min = 0;
	sidx_t max = nlv - 1;
	sidx_t this = max / 2;
	sidx_t thi2;

	if (UNLIKELY(d1 == d2)) {
		/* huh? */
		return 0;
	}

	this = find_uidx(lv, nlv, d1, this, min, max);

	if (d1 < d2) {
		min = this;
	} else {
		max = this;
	}
	thi2 = find_uidx(lv, nlv, d2, (min + max) / 2, min, max);
	if (LIKELY(this == thi2)) {
		return 0;
	}
	return lv[thi2].corr - lv[this].corr;
}

DEFUN int
leaps_on(zleap_t lv, size_t nlv, uint32_t d)
{
	sidx_t min = 0;
	sidx_t max = nlv - 1;
	sidx_t this = max / 2;

	this = find_uidx(lv, nlv, d, this, min, max);
	if (LIKELY(lv[this].u == d)) {
		return 0;
	} else if (this) {
		return lv[this].corr - lv[this - 1].corr;
	} else if (this + 1 < (sidx_t)nlv) {
		return lv[this + 1].corr - lv[this].corr;
	}
	/* huh?  this case means there's a fuckered array */
	return 0;
}

DEFUN int
leaps_till(zleap_t lv, size_t nlv, uint32_t d)
{
	sidx_t min = 0;
	sidx_t max = nlv - 1;
	sidx_t this = max / 2;

	this = find_uidx(lv, nlv, d, this, min, max);
	return lv[this].corr - lv[0].corr;
}

DEFUN int
leaps_since(zleap_t lv, size_t nlv, uint32_t d)
{
	sidx_t min = 0;
	sidx_t max = nlv - 1;
	sidx_t this = max / 2;

	this = find_uidx(lv, nlv, d, this, min, max);
	return lv[nlv - 1].corr - lv[this].corr;
}

/* signed variants */
DEFUN int
leaps_sbetween(zleap_t lv, size_t nlv, int32_t d1, int32_t d2)
{
	sidx_t min = 0;
	sidx_t max = nlv - 1;
	sidx_t this = max / 2;
	sidx_t thi2;

	if (UNLIKELY(d1 == d2)) {
		/* huh? */
		return 0;
	}

	this = find_sidx(lv, nlv, d1, this, min, max);

	if (d1 < d2) {
		min = this;
	} else {
		max = this;
	}
	thi2 = find_sidx(lv, nlv, d2, (min + max) / 2, min, max);
	if (LIKELY(this == thi2)) {
		return 0;
	}
	return lv[thi2].corr - lv[this].corr;
}

DEFUN int
leaps_son(zleap_t lv, size_t nlv, int32_t d)
{
	sidx_t min = 0;
	sidx_t max = nlv - 1;
	sidx_t this = max / 2;

	this = find_sidx(lv, nlv, d, this, min, max);
	if (LIKELY(lv[this].u == d)) {
		return 0;
	} else if (this) {
		return lv[this].corr - lv[this - 1].corr;
	} else if (this + 1 < (sidx_t)nlv) {
		return lv[this + 1].corr - lv[this].corr;
	}
	/* huh?  this case means there's a fuckered array */
	return 0;
}

DEFUN int
leaps_still(zleap_t lv, size_t nlv, int32_t d)
{
	sidx_t min = 0;
	sidx_t max = nlv - 1;
	sidx_t this = max / 2;

	this = find_sidx(lv, nlv, d, this, min, max);
	return lv[this].corr - lv[0].corr;
}

DEFUN int
leaps_ssince(zleap_t lv, size_t nlv, int32_t d)
{
	sidx_t min = 0;
	sidx_t max = nlv - 1;
	sidx_t this = max / 2;

	this = find_sidx(lv, nlv, d, this, min, max);
	return lv[nlv - 1].corr - lv[this].corr;
}

#endif	/* INCLUDED_leaps_c_ */
/* leaps.c ends here */
