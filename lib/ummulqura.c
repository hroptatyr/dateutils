/*** ummulqura.c -- guts for hijri dates
 *
 * Copyright (C) 2024 Sebastian Freundt
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
/* set aspect temporarily */
#define ASPECT_UMMULQURA
/* permanent aspect, to be read as have we ever seen aspect_ymd */
#if !defined ASPECT_UMMULQURA_
#define ASPECT_UMMULQURA_
#endif	/* !ASPECT_UMMULQURA_ */

#include "nifty.h"

#if !defined DEFUN
# define DEFUN
#endif	/* !DEFUN */


#if !defined UMMULQURA_ASPECT_HELPERS_
#define UMMULQURA_ASPECT_HELPERS_
#include "../data/ummulqura.tab"
#define UMMULQURA_MIN_YEAR	(UMMULQURA_BASE)
#define UMMULQURA_MAX_YEAR	(countof(_bom) + UMMULQURA_BASE - 1)

DEFUN unsigned int
__get_mdays_hijri(unsigned int y, signed int m)
{
	unsigned int of = (y - UMMULQURA_BASE) * HIJRI_MONTHS_P_YEAR + m;
	if (LIKELY(of && of < sizeof(_bom) / sizeof(**_bom))) {
		const uint32_t *_xbom = (void*)_bom;
		return _xbom[of] - _xbom[of - 1U];
	}
	return 30U;
}

DEFUN __attribute__((pure)) dt_ummulqura_t
__ummulqura_fixup(dt_ummulqura_t d)
{
	if (UNLIKELY(d.y < UMMULQURA_BASE)) {
		;
	} else if (UNLIKELY(d.y >= countof(_bom) + UMMULQURA_BASE)) {
		;
	} else if (UNLIKELY(d.m == 0 || d.m > HIJRI_MONTHS_P_YEAR)) {
		;
	} else if (LIKELY(d.y < countof(_bom) + UMMULQURA_BASE - 1 ||
			  d.m < HIJRI_MONTHS_P_YEAR)) {
		uint32_t this = _bom[d.y - UMMULQURA_BASE][d.m - 1];
		uint32_t next = _bom[d.y - UMMULQURA_BASE][d.m];

		if (UNLIKELY(d.d > next - this)) {
			d.d = next - this;
		}
	}
	return d;
}
#endif	/* !UMMULQURA_ASPECT_HELPERS_ */


#if defined ASPECT_GETTERS && !defined UMMULQURA_ASPECT_GETTERS_
#define UMMULQURA_ASPECT_GETTERS_
/* please implement me */
#endif	/* UMMULQURA_ASPECT_GETTERS_ */


#if defined ASPECT_CONV && !defined UMMULQURA_ASPECT_CONV_
#define UMMULQURA_ASPECT_CONV_
/* we need some getter stuff, so get it */
#define ASPECT_GETTERS
#include "ummulqura.c"
#undef ASPECT_GETTERS

static dt_ummulqura_t
__ldn_to_ummulqura(dt_ldn_t d)
{
	dt_ummulqura_t res = {0};
	size_t y, m;

	for (y = 0U; y < countof(_bom) && _bom[y][0U] <= d; y++);
	res.y = --y + UMMULQURA_BASE;
	for (m = 0U; m < 12 && _bom[y][m] <= d; m++);
	res.m = m--;
	res.d = ++d - _bom[y][m];
	return res;
}

static dt_ldn_t
__ummulqura_to_ldn(dt_ummulqura_t d)
{
	dt_ldn_t res = 0;

	if (UNLIKELY(d.y < UMMULQURA_BASE)) {
		goto out;
	} else if (UNLIKELY(d.y >= countof(_bom) + UMMULQURA_BASE)) {
		goto out;
	} else if (UNLIKELY(d.m > HIJRI_MONTHS_P_YEAR)) {
		goto out;
	}
	res = _bom[d.y - UMMULQURA_BASE][d.m - 1U] + d.d - 1;
out:
	return res;
}
#endif	/* ASPECT_CONV */


#if defined ASPECT_ADD && !defined UMMULQURA_ASPECT_ADD_
#define UMMULQURA_ASPECT_ADD_
static __attribute__((pure)) dt_ummulqura_t
__ummulqura_fixup_d(unsigned int y, signed int m, signed int d)
{
	dt_ummulqura_t res = {0};

	m += !m;
	if (LIKELY(d >= 1 && d <= 29)) {
		/* all months in our design range have at least 29 days */
		;
	} else if (d < 1) {
		unsigned int mdays;

		do {
			if (UNLIKELY(--m < 1)) {
				--y;
				m = HIJRI_MONTHS_P_YEAR;
			}
			mdays = __get_mdays_hijri(y, m);
			d += mdays;
		} while (d < 1);

	} else {
		int mdays;

		while (d > (mdays = __get_mdays_hijri(y, m))) {
			d -= mdays;
			if (UNLIKELY(++m > (signed int)HIJRI_MONTHS_P_YEAR)) {
				++y;
				m = 1;
			}
		}
	}

	res.y = y;
	res.m = m;
	res.d = d;
	return res;
}

static dt_ummulqura_t
__ummulqura_add_d(dt_ummulqura_t d, int n)
{
/* add N days to D */
	signed int tgtd;
	d = __ummulqura_fixup(d);
	tgtd = d.d + n + (n < 0 && !d.d);

	/* fixup the day */
	return __ummulqura_fixup_d(d.y, d.m, tgtd);
}

static dt_ummulqura_t
__ummulqura_add_w(dt_ummulqura_t d, int n)
{
/* add N weeks to D */
	return __ummulqura_add_d(d, GREG_DAYS_P_WEEK * n);
}

static dt_ummulqura_t
__ummulqura_add_m(dt_ummulqura_t d, int n)
{
/* add N months to D */
	signed int tgtm = d.m + n;

	while (tgtm > (signed int)GREG_MONTHS_P_YEAR) {
		tgtm -= GREG_MONTHS_P_YEAR;
		++d.y;
	}
	while (tgtm < 1) {
		tgtm += GREG_MONTHS_P_YEAR;
		--d.y;
	}
	/* final assignment */
	d.m = tgtm + (n < 0 && !d.m);
	return d;
}

static dt_ummulqura_t
__ummulqura_add_y(dt_ummulqura_t d, int n)
{
/* add N years to D */
	d.y += n;
	return d;
}
#endif	/* ASPECT_ADD */


#if defined ASPECT_DIFF && !defined UMMULQURA_ASPECT_DIFF_
#define UMMULQURA_ASPECT_DIFF_
#endif	/* ASPECT_DIFF */


#if defined ASPECT_STRF && !defined UMMULQURA_ASPECT_STRF_
#define UMMULQURA_ASPECT_STRF_

#endif	/* ASPECT_STRF */

#undef ASPECT_UMMULQURA

/* ummulqura.c ends here */
