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

struct zleap_tr_s_s {
	int32_t t;
	int32_t corr;
};

struct zleap_tr_d_s {
	dt_daisy_t d;
	int32_t corr;
};

/* currently the list looks like
 * # Leap	YEAR	MONTH	DAY	HH:MM:SS	CORR	R/S
 * Leap	1972	Jun	30	23:59:60	+	S
 * Leap	1972	Dec	31	23:59:60	+	S
 * Leap	1973	Dec	31	23:59:60	+	S
 * Leap	1974	Dec	31	23:59:60	+	S
 * Leap	1975	Dec	31	23:59:60	+	S
 * Leap	1976	Dec	31	23:59:60	+	S
 * Leap	1977	Dec	31	23:59:60	+	S
 * Leap	1978	Dec	31	23:59:60	+	S
 * Leap	1979	Dec	31	23:59:60	+	S
 * Leap	1981	Jun	30	23:59:60	+	S
 * Leap	1982	Jun	30	23:59:60	+	S
 * Leap	1983	Jun	30	23:59:60	+	S
 * Leap	1985	Jun	30	23:59:60	+	S
 * Leap	1987	Dec	31	23:59:60	+	S
 * Leap	1989	Dec	31	23:59:60	+	S
 * Leap	1990	Dec	31	23:59:60	+	S
 * Leap	1992	Jun	30	23:59:60	+	S
 * Leap	1993	Jun	30	23:59:60	+	S
 * Leap	1994	Jun	30	23:59:60	+	S
 * Leap	1995	Dec	31	23:59:60	+	S
 * Leap	1997	Jun	30	23:59:60	+	S
 * Leap	1998	Dec	31	23:59:60	+	S
 * Leap	2005	Dec	31	23:59:60	+	S
 * Leap	2008	Dec	31	23:59:60	+	S
 * Leap	2012	Jun	30	23:59:60	+	S
 *
 * so in zleap_tr notation this is: */
#if 0
static const struct zleap_tr_s_s leaps[] = {
	{0x04b25800, 1},
	{0x05a4ec01, 2},
	{0x07861f82, 3},
	{0x09675303, 4},
	{0x0b488684, 5},
	{0x0d2b0b85, 6},
	{0x0f0c3f06, 7},
	{0x10ed7287, 8},
	{0x12cea608, 9},
	{0x159fca89, 10},
	{0x1780fe0a, 11},
	{0x1962318b, 12},
	{0x1d25ea0c, 13},
	{0x21dae50d, 14},
	{0x259e9d8e, 15},
	{0x277fd10f, 16},
	{0x2a50f590, 17},
	{0x2c322911, 18},
	{0x2e135c92, 19},
	{0x30e72413, 20},
	{0x33b84894, 21},
	{0x368c1015, 22},
	{0x43b71b96, 23},
	{0x495c0797, 24},
	{0x4fef9318, 25},
};
#endif	/* 0 */

/* like leaps but t is in fact daisy */
static const struct zleap_tr_d_s leaps_d[] = {
	{0, 0},
	{20270, 1},
	{20454, 2},
	{20819, 3},
	{21184, 4},
	{21549, 5},
	{21915, 6},
	{22280, 7},
	{22645, 8},
	{23010, 9},
	{23557, 10},
	{23922, 11},
	{24287, 12},
	{25018, 13},
	{25932, 14},
	{26663, 15},
	{27028, 16},
	{27575, 17},
	{27940, 18},
	{28305, 19},
	{28854, 20},
	{29401, 21},
	{29950, 22},
	{32507, 23},
	{33603, 24},
	{34880, 25},
	{UINT32_MAX, 25},
};

#if 1
/* this can be called roughly 100m/sec */
static ssize_t
__find_leaps_idx(dt_daisy_t c, ssize_t this, ssize_t min, ssize_t max)
{
/* given a daisy C find the index of the transition before CT00:00:00 */
	do {
		dt_daisy_t tl, tu;

		tl = leaps_d[this].d;
		tu = leaps_d[this + 1].d;

		if (c > tl && c <= tu) {
			/* found him */
			return this;
		} else if (max - 1 <= min) {
			/* nearly found him */
			return this + 1;
		} else if (c > tu) {
			min = this + 1;
			this = (this + max) / 2;
		} else if (c <= tl) {
			max = this - 1;
			this = (this + min) / 2;
		}
	} while (true);
	/* not reached */
}
#endif	/* 1 */

DEFUN int32_t
leaps_between(dt_daisy_t d1, dt_daisy_t d2)
{
	ssize_t min = 0;
	ssize_t max = countof(leaps_d) - 1;
	ssize_t this = max / 2;
	ssize_t thi2;

	if (UNLIKELY(d1 == d2)) {
		/* huh? */
		return 0;
	}

	this = __find_leaps_idx(d1, this, min, max);

	if (d1 < d2) {
		min = this;
	} else {
		max = this;
	}
	thi2 = __find_leaps_idx(d2, (min + max) / 2, min, max);
	if (LIKELY(this == thi2)) {
		return 0;
	}
	return leaps_d[thi2].corr - leaps_d[this].corr;
}

#endif	/* INCLUDED_leaps_c_ */
/* leaps.c ends here */
