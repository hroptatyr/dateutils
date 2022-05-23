/*** bizda.c -- guts for bizda dates
 *
 * Copyright (C) 2010-2022 Sebastian Freundt
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
#include "boops.h"

/* set aspect temporarily */
#define ASPECT_BIZDA
/* permanent aspect, to be read as have we ever seen aspect_bizda */
#if !defined ASPECT_BIZDA_
#define ASPECT_BIZDA_
#endif	/* !ASPECT_BIZDA_ */

#if !defined DEFUN
# define DEFUN
#endif	/* !DEFUN */


#if !defined BIZDA_ASPECT_HELPERS_
#define BIZDA_ASPECT_HELPERS_

#if defined ASPECT_YMD
static int
__get_d_equiv(dt_dow_t dow, int b)
{
/* return the number of gregorian days B business days away from now,
 * where the first day is on a DOW. */
	int res = 0;
	int u5, u7;

	if (dow >= DT_SATURDAY) {
		if (b >= 0) {
			/* move to monday */
			res += 8 - dow;
			dow = DT_MONDAY;
			b -= b != 0;
		} else {
			/* move to friday */
			res -= dow - 5;
			dow = DT_FRIDAY;
			b += b != 0;
		}
	}
	/* 384 == 4 mod 5 and 384 == 6 mod 7 */
	u5 = (dow + 384 + b) % 5;
	b = b / 5 * 7 + b % 5;
	u7 = (dow + 384 + b) % 7;

	/* u5 is the day we want to be on, Mon=0
	 * u7 is the day we land on, Mon=0 */
	res += b;
	res += u5 - u7;
	res += b >= 0 && u5 < u7 ? 7 : 0;
	return res;
}
#endif	/* ASPECT_YMD */

static int
__get_b_equiv(dt_dow_t dow, int d)
{
/* return the number of business days D gregorian days away from now,
 * where the first day is on a DOW. */
	int res = 0;

	switch (dow) {
	case DT_MONDAY:
	case DT_TUESDAY:
	case DT_WEDNESDAY:
	case DT_THURSDAY:
	case DT_FRIDAY:
		res += DUWW_BDAYS_P_WEEK * (d / (signed int)GREG_DAYS_P_WEEK);
		d %= (signed int)GREG_DAYS_P_WEEK;
		break;
	case DT_SATURDAY:
		res--;
	case DT_SUNDAY:
		res--;
		d--;
		res += DUWW_BDAYS_P_WEEK * (d / (signed int)GREG_DAYS_P_WEEK);
		if ((d %= (signed int)GREG_DAYS_P_WEEK) < 0) {
			/* act as if we're on the friday before */
			res++;
		}
		dow = DT_MONDAY;
		break;
	case DT_MIRACLEDAY:
	default:
		break;
	}

	/* invariant dow + d \in [-6,13] */
	switch ((int)dow + d) {
	case -6:
	case -5:
	case -4:
	case -3:
	case -2:
		res += d + 2;
		break;
	case -1:
		res += d + 2;
		break;
	case 0:
		res += d + 1;
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		res += d;
		break;
	case 6:
		res += d - 1;
		break;
	case 7:
		res += d - 2;
		break;
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
		res += d - 2;
		break;
	case 13:
		res += d - 2 - 1;
	default:
		break;
	}
	return res;
}

DEFUN __attribute__((pure)) dt_bizda_t
__bizda_fixup(dt_bizda_t d)
{
/* given dates like 2013-08-23b this returns 2013-08-22b */
	int bdays;

	if (LIKELY(d.bd <= 20)) {
		/* every month has 20 business days */
		;
	} else if (UNLIKELY(d.m == 0 || d.m > GREG_MONTHS_P_YEAR)) {
		;
	} else if (d.bd > (bdays = __get_bdays(d.y, d.m))) {
		d.bd = bdays;
	}
	return d;
}

static int
__get_nwedays(int dur, dt_dow_t wd)
{
/* get the number of weekend days in a sequence of DUR days ending on WD
 * The minimum number of weekend days is simply 2 for every 7 days
 * to get the exact number observe the following:
 *
 * The number of remaining weekend days depends on the remaining number
 * of days and the DOW of the end point.
 *
 * here's the matrix, mod7 down, WD right:
 *    S M T W R F A
 * 0  0 0 0 0 0 0 0
 * 1  1 0 0 0 0 0 1
 * 2  2 1 0 0 0 0 1
 * 3  2 2 1 0 0 0 1
 * 4  2 2 2 1 0 0 1
 * 5  2 2 2 2 1 0 1
 * 6  2 2 2 2 2 1 1
 *
 * That means
 * (mod7 == 0) -> 0
 * (WD == SAT) -> 1
 * (mod7 > WD + 1) -> 2
 * (mod7 > WD) -> 1
 *
 * and that's all the magic behind the following code
 *
 * Note: If the number of weekend days in a sequence of DUR days starting on WD
 * (or, equivalently, -DUR days ending on WD before the start day) is sought
 * after, just use -DUR as input.
 *
 * Here's the table for the converse:
 *    S M T W R F A
 * 0  0 0 0 0 0 0 0
 * 1  1 0 0 0 0 1 2
 * 2  1 0 0 0 1 2 2
 * 3  1 0 0 1 2 2 2
 * 4  1 0 1 2 2 2 2
 * 5  1 1 2 2 2 2 2
 * 6  1 2 2 2 2 2 2
 *
 * mod7 == 0 -> 0
 * wd == SUN -> 1
 * (mod7 + wd >= 7) -> 2
 * (mod7 + wd >= 6) -> 1  */
	int nss = (dur / (signed)GREG_DAYS_P_WEEK) * 2;
	int mod = (dur % (signed)GREG_DAYS_P_WEEK);
	int xwd = (wd % GREG_DAYS_P_WEEK) + 1;

	/* saturday fix-up */
	nss += wd == DT_SATURDAY;

	nss += mod + 0 > xwd;
	nss += mod + 1 > xwd;
	nss -= mod + 7 < xwd;
	nss -= mod + 6 < xwd;
	return nss;
}

static int
__get_nbdays(int dur, dt_dow_t wd)
{
/* get the number of business days in a sequence of DUR days ending on WD
 * which is simply the number of days minus the number of weekend-days */
	if (dur) {
		return dur - __get_nwedays(dur, wd);
	}
	return 0;
}

DEFUN unsigned int
__get_bdays(unsigned int y, unsigned int m)
{
/* the 28th exists in every month, and it's exactly 20 bdays
 * away from the first, oh and it's -1 mod 7
 * then to get the number of bdays remaining in the month
 * from the number of days remaining in the month R
 * we use a multiplication table, downwards the weekday of the
 * 28th, rightwards the days in excess of the 28th
 * Miracleday is only there to make the left hand side of the
 * multiplication 3 bits wide:
 *
 * r->  0  1  2  3
 * Sun  0  1  2  3
 * Mon  0  1  2  3
 * Tue  0  1  2  3
 * Wed  0  1  2  2
 * Thu  0  1  1  1
 * Fri  0  0  0  1
 * Sat  0  0  1  2
 * Mir  0  0  0  0
 */
	unsigned int md = __get_mdays(y, m);

	/* rd should not overflow */
	assert((signed int)md - 28 >= 0);
	
	unsigned int rd = (unsigned int)(md - 28U);
	dt_dow_t m01wd;
	dt_dow_t m28wd;

	/* wday of the 1st and 28th */
	m01wd = __get_m01_wday(y, m);
	m28wd = (dt_dow_t)(m01wd - 1 ?: DT_SUNDAY);
	if (LIKELY(rd > 0)) {
		switch (m28wd) {
		case DT_SUNDAY:
		case DT_MONDAY:
		case DT_TUESDAY:
			return 20 + rd;
		case DT_WEDNESDAY:
			return 20 + rd - (rd == 3);
		case DT_THURSDAY:
			return 21;
		case DT_FRIDAY:
			return 20 + (rd == 3);
		case DT_SATURDAY:
			return 20 + rd - 1;
		case DT_MIRACLEDAY:
		default:
			abort();
		}
	}
	return 20U;
}
#endif	/* BIZDA_ASPECT_HELPERS_ */


#if defined ASPECT_GETTERS && !defined BIZDA_ASPECT_GETTERS_
#define BIZDA_ASPECT_GETTERS_

static unsigned int
__bizda_get_mday(dt_bizda_t that)
{
	dt_dow_t wd01;
	unsigned int res;

	/* find first of the month first */
	wd01 = __get_m01_wday(that.y, that.m);

	switch (wd01) {
	case DT_MONDAY:
	case DT_TUESDAY:
	case DT_WEDNESDAY:
	case DT_THURSDAY:
	case DT_FRIDAY:
		res = 1;
		break;
	case DT_SATURDAY:
		wd01 = DT_MONDAY;
		res = 3;
		break;
	case DT_SUNDAY:
		wd01 = DT_MONDAY;
		res = 2;
		break;
	case DT_MIRACLEDAY:
	default:
		res = 0;
		break;
	}
	/* now just add up bdays */
	{
		unsigned int wk;
		unsigned int nd;
		unsigned int b = that.bd;
		unsigned int magic = (b - 1 + wd01 - 1);

		assert(b + wd01 >= 2);
		wk = magic / DUWW_BDAYS_P_WEEK;
		nd = magic % DUWW_BDAYS_P_WEEK;
		res += wk * GREG_DAYS_P_WEEK + nd - wd01 + 1;
	}
	/* fixup mdays */
	if (res > __get_mdays(that.y, that.m)) {
		res = 0;
	}
	return res;
}

static dt_dow_t
__bizda_get_wday(dt_bizda_t that)
{
	dt_dow_t wd01;
	unsigned int b;
	unsigned int magic;

	/* find first of the month first */
	wd01 = __get_m01_wday(that.y, that.m);
	b = that.bd;
	magic = (b - 1 + (wd01 < DT_SUNDAY ? wd01 : 6) - 1);
	/* now just add up bdays */
	return (dt_dow_t)((magic % DUWW_BDAYS_P_WEEK) + DT_MONDAY);
}

static unsigned int
__bizda_get_count(dt_bizda_t that)
{
/* get N where N is the N-th occurrence of wday in the month of that year */
	unsigned int bd = __get_bdays(that.y, that.m);

	if (UNLIKELY(that.bd + DUWW_BDAYS_P_WEEK > bd)) {
		return DUWW_BDAYS_P_WEEK;
	}
	return (that.bd - 1U) / DUWW_BDAYS_P_WEEK + 1U;
}

static unsigned int
__bizda_get_yday(dt_bizda_t that, dt_bizda_param_t param)
{
/* return the N-th business day in Y,
 * the meaning of ultimo will be stretched to Y-ultimo, either last year's
 * or this year's, other reference points are not yet supported
 *
 * we use the following table (days beyond 20 bdays per month (across)):
 * Mon  3  0  2   1  3  1   2  3  0   3  2  1 
 * Mon  3  1  1   rest like Tue
 * Tue  3  0  1   2  3  0   3  2  1   3  1  2 
 * Tue  3  1  1   rest like Wed
 * Wed  3  0  1   2  2  1   3  1  2   3  0  3 
 * Wed  3  0  2   rest like Thu
 * Thu  2  0  2   2  1  2   3  1  2   2  1  3 
 * Thu  2  0  3   rest like Fri
 * Fri  1  0  3   2  1  2   2  2  2   1  2  3 
 * Fri  1  1  3   rest like Sat
 * Sat  1  0  3   1  2  2   1  3  2   1  2  2 
 * Sat  1  1  3   rest like Sun
 * Sun  2  0  3   0  3  2   1  3  1   2  2  1 
 * Sun  2  1  2   rest like Mon
 * */
	struct __bdays_by_wday_s {
		unsigned int jan:2;
		unsigned int feb:2;
		unsigned int mar:2;
		unsigned int apr:2;
		unsigned int may:2;
		unsigned int jun:2;
		unsigned int jul:2;
		unsigned int aug:2;
		unsigned int sep:2;
		unsigned int oct:2;
		unsigned int nov:2;
		unsigned int dec:2;

		unsigned int feb_leap:2;
		unsigned int mar_leap:2;

		/* 4 bits left */
		unsigned int flags:4;
	};
	static struct __bdays_by_wday_s tbl[8U] = {
		{
			/* DT_MIRACLEDAY */
			0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0
		}, {
			/* DT_MONDAY */
			3, 0, 2,  1, 3, 1,  2, 3, 0,  3, 2, 1,  1, 1, 0
		}, {
			/* DT_TUESDAY */
			3, 0, 1,  2, 3, 0,  3, 2, 1,  3, 1, 2,  1, 1, 0
		}, {
			/* DT_WEDNESDAY */
			3, 0, 1,  2, 2, 1,  3, 1, 2,  3, 0, 3,  0, 2, 0
		}, {
			/* DT_THURSDAY */
			2, 0, 2,  2, 1, 2,  3, 1, 2,  2, 1, 3,  0, 3, 0
		}, {
			/* DT_FRIDAY */
			1, 0, 3,  2, 1, 2,  2, 2, 2,  1, 2, 3,  1, 3, 0
		}, {
			/* DT_SATURDAY */
			1, 0, 3,  1, 2, 2,  1, 3, 2,  1, 2, 2,  1, 3, 0
		}, {
			/* DT_SUNDAY */
			2, 0, 3,  0, 3, 2,  1, 3, 1,  2, 2, 1,  1, 2, 0
		},
	};
	dt_dow_t j01wd;
	unsigned int y = that.y;
	unsigned int m = that.m;
	unsigned int accum = 0;

	if (UNLIKELY(param.ref != BIZDA_ULTIMO)) {
		return 0;
	}
	j01wd = __get_jan01_wday(that.y);

	if (LIKELY(!__leapp(y))) {
		union {
			uint32_t u;
			uint32_t lu:2;
			struct __bdays_by_wday_s s;
		} page = {
			.s = tbl[j01wd],
		};
		for (unsigned int i = 0; i < m - 1; i++) {
			accum += page.lu;
#if BYTE_ORDER == BIG_ENDIAN
			page.u <<= 2;
#elif BYTE_ORDER == LITTLE_ENDIAN
			page.u >>= 2;
#else
# warning unknown byte order
#endif	/* BYTE_ORDER */
		}
	} else if (m > 1) {
		union {
			uint32_t u;
			uint32_t lu:2;
			struct __bdays_by_wday_s s;
		} page = {
			.s = tbl[j01wd],
		};
		accum += page.lu;
		if (m > 2) {
			accum += page.s.feb_leap;
		}
		if (m > 3) {
			accum += page.s.mar_leap;
		}
		/* load a different page now, shift to the right month */
		page.s = tbl[(j01wd < DT_SUNDAY ? j01wd : 0U) + DT_MONDAY];
#if BYTE_ORDER == BIG_ENDIAN
		page.u <<= 6;
#elif BYTE_ORDER == LITTLE_ENDIAN
		page.u >>= 6;
#else
# warning unknown byte order
#endif	/* BYTE_ORDER */
		for (unsigned int i = 4; i < m; i++) {
			accum += page.lu;
#if BYTE_ORDER == BIG_ENDIAN
			page.u <<= 2;
#elif BYTE_ORDER == LITTLE_ENDIAN
			page.u >>= 2;
#else
# warning unknown byte order
#endif	/* BYTE_ORDER */
		}
	}
	return 20 * (m - 1) + accum + that.bd;
}
#endif	/* BIZDA_ASPECT_GETTERS_ */


#if defined ASPECT_CONV && !defined BIZDA_ASPECT_CONV_
#define BIZDA_ASPECT_CONV_
static dt_ymd_t
__bizda_to_ymd(dt_bizda_t d)
{
	unsigned int tgtd = __bizda_get_mday(d);
#if defined HAVE_ANON_STRUCTS_INIT
	return (dt_ymd_t){.y = d.y, .m = d.m, .d = tgtd};
#else  /* !HAVE_ANON_STRUCTS_INIT */
	dt_ymd_t res;

	res.y = d.y;
	res.m = d.m;
	res.d = tgtd;
	return res;
#endif	/* HAVE_ANON_STRUCTS_INIT */
}

static dt_ywd_t
__bizda_to_ywd(dt_bizda_t d, dt_bizda_param_t p)
{
	unsigned int yd = __bizda_get_yday(d, p);

	return __make_ywd_ybd(d.y, yd);
}

static dt_ymcw_t
__bizda_to_ymcw(dt_bizda_t d, dt_bizda_param_t UNUSED(p))
{
	unsigned int c = __bizda_get_count(d);
	dt_dow_t w = __bizda_get_wday(d);
#if defined HAVE_ANON_STRUCTS_INIT
	return (dt_ymcw_t){.y = d.y, .m = d.m, .c = c, .w = w};
#else
	dt_ymcw_t res;
	res.y = d.y;
	res.m = d.m;
	res.c = c;
	res.w = w;
	return res;
#endif
}

static dt_daisy_t
__bizda_to_daisy(dt_bizda_t d, dt_bizda_param_t p)
{
	dt_daisy_t res;
	unsigned int ybd;
	unsigned int wd;

	res = __jan00_daisy(d.y);
	wd = __daisy_get_wday(res);
	ybd = __bizda_get_yday(d, p);
	res += __get_d_equiv((dt_dow_t)wd, ybd);
	return res;
}
#endif	/* ASPECT_CONV */


#if defined ASPECT_ADD && !defined BIZDA_ASPECT_ADD_
#define BIZDA_ASPECT_ADD_

static dt_bizda_t
__bizda_fixup_b(unsigned int y, signed int m, signed int b)
{
	dt_bizda_t res = {0};

	if (LIKELY(b >= 1 && b <= 20)) {
		/* all months in our design range have at least 20 bdays */
		;
	} else if (b < 1) {
		int bdays;

		do {
			if (UNLIKELY(--m < 1)) {
				--y;
				m = GREG_MONTHS_P_YEAR;
			}
			bdays = __get_bdays(y, m);
			b += bdays;
		} while (b < 1);

	} else {
		int bdays;

		while (b > (bdays = __get_bdays(y, m))) {
			b -= bdays;
			if (UNLIKELY(++m > (signed int)GREG_MONTHS_P_YEAR)) {
				++y;
				m = 1;
			}
		}
	}

	res.y = y;
	res.m = m;
	res.bd = b;
	return res;
}

static dt_bizda_t
__bizda_add_b(dt_bizda_t d, int n)
{
/* add N business days to D */
	int tgtb = d.bd + n;

	return __bizda_fixup_b(d.y, d.m, tgtb);
}

static dt_bizda_t
__bizda_add_d(dt_bizda_t d, int n)
{
/* add N real days to D */
	dt_dow_t wd = __bizda_get_wday(d);
	int tgtb = d.bd + __get_b_equiv(wd, n);

	return __bizda_fixup_b(d.y, d.m, tgtb);
}

static dt_bizda_t
__bizda_add_w(dt_bizda_t d, int n)
{
/* add N weeks to D */
	return __bizda_add_d(d, GREG_DAYS_P_WEEK * n);
}

static dt_bizda_t
__bizda_add_m(dt_bizda_t d, int n)
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
	d.m = tgtm;
	return d;
}

static dt_bizda_t
__bizda_add_y(dt_bizda_t d, int n)
{
/* add N years to D */
	d.y += n;
	return d;
}

#endif	/* ASPECT_ADD */


#if defined ASPECT_STRF && !defined BIZDA_ASPECT_STRF_
#define BIZDA_ASPECT_STRF_

DEFUN void
__prep_strfd_bizda(struct strpd_s *tgt, dt_bizda_t d, dt_bizda_param_t bp)
{
	tgt->y = d.y;
	tgt->m = d.m;
	tgt->b = d.bd;
	if (LIKELY(bp.ab == BIZDA_AFTER)) {
		tgt->flags.ab = BIZDA_AFTER;
	} else {
		tgt->flags.ab = BIZDA_BEFORE;
	}
	tgt->flags.bizda = 1;
	return;
}
#endif	/* ASPECT_STRF */

#undef ASPECT_BIZDA

/* bizda.c ends here */
