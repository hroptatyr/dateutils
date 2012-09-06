/*** bizda.c -- guts for bizda dates
 *
 * Copyright (C) 2010-2012 Sebastian Freundt
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
#define ASPECT_BIZDA
/* permanent aspect, to be read as have we ever seen aspect_bizda */
#if !defined ASPECT_BIZDA_
#define ASPECT_BIZDA_
#endif	/* !ASPECT_BIZDA_ */


#if !defined BIZDA_ASPECT_HELPERS_
#define BIZDA_ASPECT_HELPERS_

#if defined ASPECT_YMD
static int
__get_d_equiv(dt_dow_t dow, int b)
{
	int res = 0;

	switch (dow) {
	case DT_MONDAY:
	case DT_TUESDAY:
	case DT_WEDNESDAY:
	case DT_THURSDAY:
	case DT_FRIDAY:
		res += GREG_DAYS_P_WEEK * (b / (signed int)DUWW_BDAYS_P_WEEK);
		b %= (signed int)DUWW_BDAYS_P_WEEK;
		break;
	case DT_SATURDAY:
		res++;
	case DT_SUNDAY:
		res++;
		b--;
		res += GREG_DAYS_P_WEEK * (b / (signed int)DUWW_BDAYS_P_WEEK);
		if ((b %= (signed int)DUWW_BDAYS_P_WEEK) < 0) {
			/* act as if we're on the monday after */
			res++;
		}
		dow = DT_MONDAY;
		break;
	case DT_MIRACLEDAY:
	default:
		break;
	}

	/* fixup b */
	if (b < 0) {
		res -= GREG_DAYS_P_WEEK;
		b += DUWW_BDAYS_P_WEEK;
	}
	/* b >= 0 && b < 5 */
	switch (dow) {
	case DT_SUNDAY:
	case DT_MONDAY:
	case DT_TUESDAY:
	case DT_WEDNESDAY:
	case DT_THURSDAY:
	case DT_FRIDAY:
		if ((int)dow + b <= (int)DT_FRIDAY) {
			res += b;
		} else {
			res += b + 2;
		}
		break;
	case DT_SATURDAY:
		res += b + 1;
		break;
	case DT_MIRACLEDAY:
	default:
		res = 0;
	}
	return res;
}
#endif	/* ASPECT_YMD */
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

		assert(b - 1 + wd01 - 1 >= 0);
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
	magic = (b - 1 + (wd01 ? wd01 : 6) - 1);
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
	static struct __bdays_by_wday_s tbl[7] = {
		{
			/* DT_SUNDAY */
			2, 0, 3,  0, 3, 2,  1, 3, 1,  2, 2, 1,  1, 2, 0
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
#if defined WORDS_BIGENDIAN
			page.u <<= 2;
#else  /* !WORDS_BIGENDIAN */
			page.u >>= 2;
#endif	/* WORDS_BIGENDIAN */
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
		page.s = tbl[(j01wd + DT_MONDAY) % GREG_DAYS_P_WEEK];
#if defined WORDS_BIGENDIAN
		page.u <<= 6;
#else  /* !WORDS_BIGENDIAN */
		page.u >>= 6;
#endif	/* WORDS_BIGENDIAN */
		for (unsigned int i = 4; i < m; i++) {
			accum += page.lu;
#if defined WORDS_BIGENDIAN
			page.u <<= 2;
#else  /* !WORDS_BIGENDIAN */
			page.u >>= 2;
#endif	/* WORDS_BIGENDIAN */
		}
	}
	return 20 * (m - 1) + accum + that.bd;
}
#endif	/* BIZDA_ASPECT_GETTERS_ */


#if defined ASPECT_CONV && !defined BIZDA_ASPECT_CONV_
#define BIZDA_ASPECT_CONV_

#endif	/* ASPECT_CONV */


#if defined ASPECT_STRF && !defined BIZDA_ASPECT_STRF_
#define BIZDA_ASPECT_STRF_

static void
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
