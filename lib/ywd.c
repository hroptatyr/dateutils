/*** ywd.c -- guts for ywd dates
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
 **/
/* this is an attempt to code aspect-oriented */
#define ASPECT_YWD

/* table of 53-week years (years % 400)

  {4, 9, 15, 20, 26, 32, 37, 43, 48, 54, 60, 65, 71, 76, 82, 88, 93, 99, 105,
  111, 116, 122, 128, 133, 139, 144, 150, 156, 161, 167, 172, 178, 184, 189,
  195, 201, 207, 212, 218, 224, 229, 235, 240, 246, 252, 257, 263, 268, 274,
  280, 285, 291, 296, 303, 308, 314, 320, 325, 331, 336, 342, 348, 353, 359,
  364, 370, 376, 381, 387, 392, 398};
 */


#if !defined YWD_ASPECT_HELPERS_
#define YWD_ASPECT_HELPERS_
static dt_dow_t
__ywd_get_jan01_wday(dt_ywd_t d)
{
/* hang of 0 means Mon, -1 Tue, -2 Wed, -3 Thu, 3 Fri, 2 Sat, 1 Sun */
	int res;

	if (UNLIKELY((res = 1 - d.hang) < 0)) {
		return (dt_dow_t)(GREG_DAYS_P_WEEK + res);
	}
	return (dt_dow_t)res;
}

static int
__ywd_get_jan01_hang(dt_dow_t j01)
{
/* Mon means hang of 0, Tue -1, Wed -2, Thu -3, Fri 3, Sat 2, Sun 1 */
	int res;

	if (UNLIKELY((res = 1 - (int)j01) < -3)) {
		return (int)(GREG_DAYS_P_WEEK + res);
	}
	return res;
}
#endif	/* !YWD_ASPECT_HELPERS_ */


#if defined ASPECT_GETTERS && !defined YWD_ASPECT_GETTERS_
#define YWD_ASPECT_GETTERS_
static dt_ywd_t
__make_ywd(unsigned int y, unsigned int c, unsigned int w, unsigned int cc)
{
/* build a 8601 compliant ywd object from year Y, week C and weekday W
 * where C conforms to week-count convention cc */
	dt_ywd_t res = {0};
	dt_dow_t j01;

	/* this one's special as it needs the hang helper slot */
	j01 = __get_jan01_wday(y);
	res.hang = __ywd_get_jan01_hang(j01);

	res.y = y;
	res.w = w < GREG_DAYS_P_WEEK ? w : 0;

	switch (cc) {
	default:
	case YWD_ISOWK_CNT:
		res.c = c;
		break;
	case YWD_ABSWK_CNT:
		if (res.hang <= 0 || w < j01 || w && !j01/*j01 Sun, w not*/) {
			res.c = c;
		} else {
			res.c = c + 1;
		}
		break;
	case YWD_SUNWK_CNT:
		if (j01 == DT_SUNDAY) {
			res.c = c;
		} else {
			res.c = c + 1;
		}
		break;
	case YWD_MONWK_CNT:
		if (j01 <= DT_MONDAY) {
			res.c = c;
		} else {
			res.c = c + 1;
		}
		break;
	}
	return res;
}

static unsigned int
__ywd_get_yday(dt_ywd_t d)
{
/* since everything is in ISO 8601 format, getting the doy is a matter of
 * counting how many days there are in a week. */
	return GREG_DAYS_P_WEEK * (d.c - 1) + (d.w ? d.w : 7) + d.hang;
}

static dt_dow_t
__ywd_get_wday(dt_ywd_t that)
{
	return (dt_dow_t)that.w;
}

static unsigned int
__ywd_get_wcnt_mon(dt_ywd_t d)
{
/* given a YWD with week-count within the year (ISOWK_CNT convention)
 * return the week-count within the month (ABSWK_CNT convention) */
	unsigned int yd = __ywd_get_yday(d);
	struct __md_s x = __yday_get_md(d.y, yd);
	return (x.d - 1) / 7 + 1;
}

static int
__ywd_get_wcnt_year(dt_ywd_t d, unsigned int tgtcc)
{
	dt_dow_t j01;

	if (tgtcc == YWD_ISOWK_CNT) {
		return d.c;
	}
	/* otherwise we need to shift things */
	j01 = __ywd_get_jan01_wday(d);
	switch (tgtcc) {
	case YWD_ABSWK_CNT:
		if (d.w < j01) {
			return d.c - 1;
		} else if (d.hang < 0) {
			return d.c + 1;
		}
		break;
	case YWD_MONWK_CNT:
		if (j01 >= DT_TUESDAY) {
			return d.c - 1;
		}
		break;
	case YWD_SUNWK_CNT:
		if (j01 >= DT_MONDAY) {
			return d.c - 1;
		}
		break;
	default:
		break;
	}
	return d.c;
}

static struct __md_s
__ywd_get_md(dt_ywd_t d)
{
	unsigned int yday = __ywd_get_yday(d);
	struct __md_s res = __yday_get_md(d.y, yday);

	if (UNLIKELY(res.m == 0)) {
		res.m = 12;
		res.d--;
	} else if (UNLIKELY(res.m == 13)) {
		res.m = 1;
	}
	return res;
}
#endif	/* ASPECT_GETTERS */


#if defined ASPECT_CONV && !defined YWD_ASPECT_CONV_
#define YWD_ASPECT_CONV_
/* we need some of the stuff above, so get it */
#define ASPECT_GETTERS
#include "ywd.c"
#undef ASPECT_GETTERS

static dt_ymd_t
__ywd_to_ymd(dt_ywd_t d)
{
	unsigned int y;

	struct __md_s md = __ywd_get_md(d);

	if (d.c == 1 && d.w < __ywd_get_jan01_wday(d) && d.w) {
		y = d.y - 1;
	} else if (d.c >= 53/* max weeks per year*/) {
		y = d.y + 1;
	} else {
		y = d.y;
	}
	return (dt_ymd_t){.y = y, .m = md.m, .d = md.d};
}

static dt_ymcw_t
__ymd_to_ymcw(dt_ymd_t d)
{
	unsigned int c = __ymd_get_count(d);
	unsigned int w = __ymd_get_wday(d);
#if defined __C1X
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
#endif	/* ASPECT_CONV */


#if defined ASPECT_STRF && !defined YWD_ASPECT_STRF_
#define YWD_ASPECT_STRF_
static void
__prep_strfd_ywd(struct strpd_s *tgt, dt_ywd_t d)
{
/* place ywd data of THIS into D for printing with FMT. */
	if (d.c == 1 && d.w < __ywd_get_jan01_wday(d) && d.w) {
		/* put gregorian year into y and real year into q */
		tgt->y = d.y - 1;
		tgt->q = d.y;
	} else if (d.c >= 53) {
		/* bit of service for the %Y printer */
		tgt->y = d.y + 1;
		tgt->q = d.y;
	} else {
		tgt->y = d.y;
		tgt->q = d.y;
	}
	/* business as usual here */
	tgt->c = d.c;
	tgt->w = d.w;
	tgt->flags.real_y_in_q = 1;
	return;
}
#endif	/* ASPECT_STRF */

/* ywd.c ends here */
