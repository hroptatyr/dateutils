/*** ywd.c -- guts for ywd dates
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
 **/
/* this is an attempt to code aspect-oriented */
#define ASPECT_YWD

#include "nifty.h"

#if !defined DEFUN
# define DEFUN
#endif	/* !DEFUN */

/* table of 53-week years (years % 400)

  {4, 9, 15, 20, 26, 32, 37, 43, 48, 54, 60, 65, 71, 76, 82, 88, 93, 99, 105,
  111, 116, 122, 128, 133, 139, 144, 150, 156, 161, 167, 172, 178, 184, 189,
  195, 201, 207, 212, 218, 224, 229, 235, 240, 246, 252, 257, 263, 268, 274,
  280, 285, 291, 296, 303, 308, 314, 320, 325, 331, 336, 342, 348, 353, 359,
  364, 370, 376, 381, 387, 392, 398};

  we could use gperf to get a perfect hash, thank god the list is fixed, but
  at the moment we let the compiler work it out
 */

#if defined GET_ISOWK_FULL_SWITCH
#elif defined GET_ISOWK_28Y_SWITCH
#else
# define GET_ISOWK_FULL_SWITCH
#endif


#if !defined YWD_ASPECT_HELPERS_
#define YWD_ASPECT_HELPERS_
static dt_dow_t
__ywd_get_jan01_wday(dt_ywd_t d)
{
/* hang of 0 means Mon, -1 Tue, -2 Wed, -3 Thu, 3 Fri, 2 Sat, 1 Sun */
	int res;

	/* the second portion of the assert is trivially true */
	assert(d.hang >= -3 /*&& d.hang <= 3*/);
	if (UNLIKELY((res = 1 - d.hang) <= 0)) {
		res += GREG_DAYS_P_WEEK;
		assert(res > DT_MIRACLEDAY);
	}
	return (dt_dow_t)res;
}

static int
__ywd_get_jan01_hang(dt_dow_t j01)
{
/* Mon means hang of 0, Tue -1, Wed -2, Thu -3, Fri 3, Sat 2, Sun 1 */
	int res;

	if (UNLIKELY((res = 1 - (int)j01) < -3)) {
		assert(res >= -6);
		return (int)GREG_DAYS_P_WEEK + res;
	}
	return res;
}

static __attribute__((unused)) dt_dow_t
__ywd_get_dec31_wday(dt_ywd_t d)
{
/* a year starting on W ends on W if not a leap year */
	dt_dow_t res = __ywd_get_jan01_wday(d);

	if (UNLIKELY(__leapp(d.y))) {
		if (++res > GREG_DAYS_P_WEEK) {
			/* wrap around DT_SUNDAY -> DT_MONDAY */
			res = DT_MONDAY;
		}
	}
	return res;
}

#if defined GET_ISOWK_FULL_SWITCH
DEFUN __attribute__((const)) inline unsigned int
__get_isowk(unsigned int y)
{
/* return the number of iso weeks in Y */
	switch (y % 400) {
	default:
		break;
	case 4:
	case 9:
	case 15:
	case 20:
	case 26:
	case 32:
	case 37:
	case 43:
	case 48:
	case 54:
	case 60:
	case 65:
	case 71:
	case 76:
	case 82:
	case 88:
	case 93:
	case 99:
	case 105:
	case 111:
	case 116:
	case 122:
	case 128:
	case 133:
	case 139:
	case 144:
	case 150:
	case 156:
	case 161:
	case 167:
	case 172:
	case 178:
	case 184:
	case 189:
	case 195:
	case 201:
	case 207:
	case 212:
	case 218:
	case 224:
	case 229:
	case 235:
	case 240:
	case 246:
	case 252:
	case 257:
	case 263:
	case 268:
	case 274:
	case 280:
	case 285:
	case 291:
	case 296:
	case 303:
	case 308:
	case 314:
	case 320:
	case 325:
	case 331:
	case 336:
	case 342:
	case 348:
	case 353:
	case 359:
	case 364:
	case 370:
	case 376:
	case 381:
	case 387:
	case 392:
	case 398:
		return 53;
	}
	return 52;
}

static unsigned int
__get_z31wk(unsigned int y)
{
/* return the week number of 31 dec in year Y, where weeks hanging over into
 * the new year are treated as 53
 * In the 400 year cycle, there's 243 years with 53 weeks and
 * 157 years with 52 weeks. */
	switch (y % 400U) {
	default:
		break;
	case 0:
	case 5:
	case 6:
	case 10:
	case 11:
	case 16:
	case 17:
	case 21:
	case 22:
	case 23:
	case 27:
	case 28:
	case 33:
	case 34:
	case 38:
	case 39:
	case 44:
	case 45:
	case 49:
	case 50:
	case 51:
	case 55:
	case 56:
	case 61:
	case 62:
	case 66:
	case 67:
	case 72:
	case 73:
	case 77:
	case 78:
	case 79:
	case 83:
	case 84:
	case 89:
	case 90:
	case 94:
	case 95:
	case 100:
	case 101:
	case 102:
	case 106:
	case 107:
	case 112:
	case 113:
	case 117:
	case 118:
	case 119:
	case 123:
	case 124:
	case 129:
	case 130:
	case 134:
	case 135:
	case 140:
	case 141:
	case 145:
	case 146:
	case 147:
	case 151:
	case 152:
	case 157:
	case 158:
	case 162:
	case 163:
	case 168:
	case 169:
	case 173:
	case 174:
	case 175:
	case 179:
	case 180:
	case 185:
	case 186:
	case 190:
	case 191:
	case 196:
	case 197:
	case 202:
	case 203:
	case 208:
	case 209:
	case 213:
	case 214:
	case 215:
	case 219:
	case 220:
	case 225:
	case 226:
	case 230:
	case 231:
	case 236:
	case 237:
	case 241:
	case 242:
	case 243:
	case 247:
	case 248:
	case 253:
	case 254:
	case 258:
	case 259:
	case 264:
	case 265:
	case 269:
	case 270:
	case 271:
	case 275:
	case 276:
	case 281:
	case 282:
	case 286:
	case 287:
	case 292:
	case 293:
	case 297:
	case 298:
	case 299:
	case 304:
	case 305:
	case 309:
	case 310:
	case 311:
	case 315:
	case 316:
	case 321:
	case 322:
	case 326:
	case 327:
	case 332:
	case 333:
	case 337:
	case 338:
	case 339:
	case 343:
	case 344:
	case 349:
	case 350:
	case 354:
	case 355:
	case 360:
	case 361:
	case 365:
	case 366:
	case 367:
	case 371:
	case 372:
	case 377:
	case 378:
	case 382:
	case 383:
	case 388:
	case 389:
	case 393:
	case 394:
	case 395:
	case 399:
		return 52;
	}
	/* more weeks with 53, so default to that */
	return 53;
}

#elif defined GET_ISOWK_28Y_SWITCH
DEFUN __attribute__((const)) inline unsigned int
__get_isowk(unsigned int y)
{
	switch (y % 28U) {
	default:
		break;
	case 16:
		/* 1920, 1948, ... */
	case 21:
		/* 1925, 1953, ... */
	case 27:
		/* 1931, 1959, ... */
	case 4:
		/* 1936, 1964, ... */
	case 10:
		/* 1942, 1970, ... */
		return 53;
	}
	return 52;
}

static unsigned int
__get_z31wk(unsigned int y)
{
/* return the week number of 31 dec in year Y, where weeks hanging over into
 * the new year are treated as 53
 * In the 400 year cycle, there's 243 years with 53 weeks and
 * 157 years with 52 weeks. */
#if 0
	switch (y % 28U) {
	default:
		break;
		/* pattern in the 28y cycle is: 5 1 4 1 5 1 4 1 1 4 1 */
	case 0:
	case 5:
	case 6:
	case 10:
	case 11:
	case 16:
	case 17:
	case 21:
	case 22:
	case 23:
	case 27:
		return 52;
	}
	/* more weeks with 53, so default to that */
	return 53;
#endif
#define BIT(x)	(1U << (x))
	/* redo with bitmasks */
	static const unsigned int w =
		BIT(1U) ^ BIT(2U) ^ BIT(3U) ^ BIT(4U) ^
		BIT(7U) ^ BIT(8U) ^ BIT(9U) ^ BIT(12U) ^
		BIT(13U) ^ BIT(14U) ^ BIT(15U) ^ BIT(18U) ^
		BIT(19U) ^ BIT(20U) ^ BIT(24U) ^ BIT(25U) ^ BIT(26U);
	return 52 + (w >> (y % 28U)) & 0b1U;
}

#endif	/* GET_ISOWK_* */

DEFUN __attribute__((pure)) dt_ywd_t
__ywd_fixup(dt_ywd_t d)
{
/* given dates like 2012-W53-01 this returns 2013-W01-01 */
	int nw;

	if (LIKELY(d.c <= 52)) {
		/* brill all years have 52 weeks */
		;
	} else if (UNLIKELY(d.c > (nw = __get_isowk(d.y)))) {
		d.c = nw;
	}
	return d;
}

#define canon_yc(y, c, hang)				\
	if (LIKELY(c >= 1 && c <= 52)) {		\
		/* all years support this */		\
		;					\
	} else if (UNLIKELY(c < 1)) {			\
		if (UNLIKELY(__leapp(--y))) {		\
			hang++;				\
		}					\
		if (++hang > 3) {			\
			hang -= GREG_DAYS_P_WEEK;	\
		}					\
		c += __get_isowk(y);			\
	} else if (UNLIKELY(c > __get_isowk(y))) {	\
		c -= __get_isowk(y);			\
		if (UNLIKELY(__leapp(y++))) {		\
			hang--;				\
		}					\
		if (--hang < -3) {			\
			hang += GREG_DAYS_P_WEEK;	\
		}					\
	}


#endif	/* !YWD_ASPECT_HELPERS_ */


#if defined ASPECT_GETTERS && !defined YWD_ASPECT_GETTERS_
#define YWD_ASPECT_GETTERS_
static dt_ywd_t
__make_ywd_c(unsigned int y, unsigned int c, dt_dow_t w, unsigned int cc)
{
/* build a 8601 compliant ywd object from year Y, week C and weekday W
 * where C conforms to week-count convention cc */
	dt_ywd_t res = {0};
	dt_dow_t j01;
	int hang;

	/* this one's special as it needs the hang helper slot */
	j01 = __get_jan01_wday(y);
	hang = __ywd_get_jan01_hang(j01);

	switch (cc) {
	default:
	case YWD_ISOWK_CNT:
		break;
	case YWD_ABSWK_CNT: {
		dt_dow_t u = w ?: DT_THURSDAY;

		if (hang == 1 && u < DT_SUNDAY) {
			/* n-th W in the year is n-th week,
			 * year starts on sunday */
			;
		} else if (hang > 0 && u < DT_SUNDAY && u < j01) {
			/* n-th W in the year is n-th week,
			 * in this case the year doesn't start on sunday */
			;
		} else if (hang <= 0 && (u >= j01 || u == DT_SUNDAY)) {
			/* n-th W in the year is n-th week */
			;
		} else if (hang > 0) {
			/* those weekdays that hang over into the last year */
			c--;
		} else if (hang <= 0) {
			/* weekdays missing in the first week of Y */
			c++;
		}
	}
		canon_yc(y, c, hang);
		break;
	case YWD_SUNWK_CNT:
		if (j01 == DT_SUNDAY) {
			;
		} else {
			c++;
		}
		break;
	case YWD_MONWK_CNT:
		if (j01 <= DT_MONDAY) {
			;
		} else {
			c++;
		}
		break;
	}

	/* assign and fuck off */
	res.y = y;
	res.c = c;
	res.w = w;
	res.hang = hang;
	return res;
}

static dt_ywd_t
__make_ywd_yd_dow(unsigned int y, int yd, dt_dow_t dow)
{
/* build a 8601 compliant ywd object from year Y, day-of-year YD
 * and weekday DOW */
	dt_ywd_t res = {0};
	dt_dow_t j01;
	unsigned int c;
	int hang;

	/* deduce the weekday of the first, given the weekday
	 * of the yd-th is DOW */
	j01 = __get_jan01_yday_dow(yd, dow);
	hang = __ywd_get_jan01_hang(j01);

	/* compute weekday, decompose yd into 7p + q */
	c = (yd + GREG_DAYS_P_WEEK - 1 - hang) / (signed int)GREG_DAYS_P_WEEK;

	/* fixup c (and y) */
	canon_yc(y, c, hang);

	/* assign and fuck off */
	res.y = y;
	res.c = c;
	res.w = dow;
	res.hang = hang;
	return res;
}

static dt_ywd_t
__make_ywd_ybd(unsigned int y, int yd)
{
/* build a 8601 compliant ywd object from year Y and year-business-day YD */
	dt_ywd_t res = {0};
	dt_dow_t j01;
	unsigned int c;
	int w;
	int hang;

	/* this one's special as it needs the hang helper slot */
	j01 = __get_jan01_wday(y);
	hang = __ywd_get_jan01_hang(j01);

	/* compute weekday, decompose yd into 7p + q */
	c = (yd + DUWW_BDAYS_P_WEEK - 1) / (signed int)DUWW_BDAYS_P_WEEK;
	w = (yd + DUWW_BDAYS_P_WEEK - 1) % (signed int)DUWW_BDAYS_P_WEEK;
	if ((w += j01) > (signed int)DUWW_BDAYS_P_WEEK) {
		w -= DUWW_BDAYS_P_WEEK;
		c++;
	} else if (w < (signed int)DT_MONDAY) {
		w += DUWW_BDAYS_P_WEEK;
		c--;
	}

	/* fixup c (and y) */
	canon_yc(y, c, hang);

	/* assign and fuck off */
	res.y = y;
	res.c = c;
	res.w = w;
	res.hang = hang;
	return res;
}

static __attribute__((const)) int
__ywd_get_yday(dt_ywd_t d)
{
/* since everything is in ISO 8601 format, getting the doy is a matter of
 * counting how many days there are in a week.
 * This may return negative values and values larger than the number of
 * days in that year. */
	return GREG_DAYS_P_WEEK * (d.c - 1) +  (dt_dow_t)(d.w ?: DT_THURSDAY) + d.hang;
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
	int yd = __ywd_get_yday(d);
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
		if (j01 >= DT_TUESDAY && j01 < DT_SUNDAY) {
			return d.c - 1;
		}
		break;
	case YWD_SUNWK_CNT:
		if (j01 < DT_SUNDAY) {
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
	int yday = __ywd_get_yday(d);
	struct __md_s res = __yday_get_md(d.y, yday);

	if (UNLIKELY(res.m == 0)) {
		res.m = 12;
		res.d--;
	} else if (UNLIKELY(res.m == 13)) {
		res.m = 1;
	}
	return res;
}

static unsigned int
__ywd_get_mon(dt_ywd_t d)
{
	int yd = __ywd_get_yday(d);
	return __yday_get_md(d.y, yd).m;
}

static unsigned int
__ywd_get_year(dt_ywd_t d)
{
/* return the true gregorian year */
	unsigned int y = d.y;

	if (d.c == 1) {
		dt_dow_t f01 = __ywd_get_jan01_wday(d);
		y -= d.hang <= 0 && (dt_dow_t)(d.w ?: DT_THURSDAY) < f01;
	} else if (d.c >= __get_z31wk(y)) {
		dt_dow_t z31 = __ywd_get_dec31_wday(d);
		y += z31 < DT_SUNDAY && (dt_dow_t)(d.w ?: DT_THURSDAY) > z31;
	}
	return y;
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
	unsigned int y = __ywd_get_year(d);
	struct __md_s md = __ywd_get_md(d);

#if defined HAVE_ANON_STRUCTS_INIT
	return (dt_ymd_t){.y = y, .m = md.m, .d = md.d};
#else  /* !HAVE_ANON_STRUCTS_INIT */
	{
		dt_ymd_t res;

		res.y = y;
		res.m = md.m;
		res.d = md.d;
		return res;
	}
#endif	/* HAVE_ANON_STRUCTS_INIT */
}

static dt_ymcw_t
__ywd_to_ymcw(dt_ywd_t d)
{
	unsigned int y = __ywd_get_year(d);
	struct __md_s md = __ywd_get_md(d);
	unsigned int c;

	/* we obtain C from weekifying the month */
	c = (md.d - 1U) / GREG_DAYS_P_WEEK + 1U;

#if defined HAVE_ANON_STRUCTS_INIT
	return (dt_ymcw_t){.y = y, .m = md.m, .c = c, .w = d.w};
#else  /* !HAVE_ANON_STRUCTS_INIT */
	{
		dt_ymcw_t res;

		res.y = y;
		res.m = md.m;
		res.c = c;
		res.w = d.w;
		return res;
	}
#endif	/* HAVE_ANON_STRUCTS_INIT */
}

static dt_daisy_t
__ywd_to_daisy(dt_ywd_t d)
{
	dt_daisy_t res;

	res = __jan00_daisy(d.y);
	res += __ywd_get_yday(d);
	return res;
}

static dt_yd_t
__ywd_to_yd(dt_ywd_t d)
{
	unsigned int y = __ywd_get_year(d);
	int x = __ywd_get_yday(d);

	if (UNLIKELY(x <= 0)) {
		/* gotta go for last years thing */
		x += __get_ydays(d.y - 1);
	} else if (x > (int)__get_ydays(d.y)) {
		x -= __get_ydays(d.y);
	}

#if defined HAVE_ANON_STRUCTS_INIT
	return (dt_yd_t){.y = y, .d = x};
#else  /* !HAVE_ANON_STRUCTS_INIT */
	dt_yd_t res;
	res.y = y;
	res.d = x;
	return res;
#endif	/* HAVE_ANON_STRUCTS_INIT */
}
#endif	/* ASPECT_CONV */


#if defined ASPECT_ADD && !defined YWD_ASPECT_ADD_
#define YWD_ASPECT_ADD_

static dt_ywd_t
__ywd_fixup_w(unsigned int y, signed int w, dt_dow_t d, int hang)
{
	dt_ywd_t res = {0};

	/* fixup q */
	if (LIKELY(w >= 1 && w <= 52)) {
		/* all years support this */
		;
	} else if (w < 1) {
		do {
			if (UNLIKELY(__leapp(--y))) {
				hang++;
			}
			if (++hang > 3) {
				hang -= GREG_DAYS_P_WEEK;
			}
			w += __get_isowk(y);
		} while (w < 1);

	} else {
		int nw;

		while (w > (nw = __get_isowk(y))) {
			w -= nw;
			if (UNLIKELY(__leapp(y++))) {
				hang--;
			}
			if (--hang < -3) {
				hang += GREG_DAYS_P_WEEK;
			}
		}
	}

	/* final assignment */
	res.y = y;
	res.c = w;
	res.w = d;
	res.hang = hang;
	return res;
}

static dt_ywd_t
__ywd_add_w(dt_ywd_t d, int n)
{
/* add N weeks to D */
	signed int tgtc = d.c + n;

	return __ywd_fixup_w(d.y, tgtc, (dt_dow_t)d.w, d.hang);
}

static dt_ywd_t
__ywd_add_d(dt_ywd_t d, int n)
{
/* add N days to D
 * we reduce this to __ywd_add_w() */
	signed int aw = n / 7;
	signed int ad = n % 7;

	if ((ad += d.w) > (signed int)GREG_DAYS_P_WEEK) {
		ad -= GREG_DAYS_P_WEEK;
		aw++;
	} else if (ad <= 0) {
		ad += GREG_DAYS_P_WEEK;
		aw--;
	}

	d.w = (dt_dow_t)ad;
	return __ywd_add_w(d, aw);
}

static dt_ywd_t
__ywd_add_b(dt_ywd_t d, int UNUSED(n))
{
/* add N business days to D */
	return d;
}

static dt_ywd_t
__ywd_add_y(dt_ywd_t d, int n)
{
/* add N years to D */
	dt_dow_t j01;

	d.y = d.y + n;
	/* recompute hang */
	j01 = __get_jan01_wday(d.y);
	d.hang = __ywd_get_jan01_hang(j01);
	return d;
}
#endif	/* ASPECT_ADD */


#if defined ASPECT_DIFF && !defined YWD_ASPECT_DIFF_
#define YWD_ASPECT_DIFF_

static struct dt_ddur_s
__ywd_diff(dt_ywd_t d1, dt_ywd_t d2)
{
/* compute d2 - d1 entirely in terms of ymd but express the result as yd */
	struct dt_ddur_s res = dt_make_ddur(DT_DURYWD, 0);
	signed int tgtd;
	signed int tgtw;
	signed int tgty;

	if (d1.u > d2.u) {
		/* swap d1 and d2 */
		dt_ywd_t tmp = d1;
		res.neg = 1;
		d1 = d2;
		d2 = tmp;
	}

	/* first compute the difference in years */
	tgty = (d2.y - d1.y);
	/* ... and weeks */
	tgtw = (d2.c - d1.c);
	/* ... oh, and days, too */
	tgtd = (d2.w ?: 7) - (d1.w ?: 7);

	/* add carry */
	if (tgtd < 0) {
		tgtw--;
		tgtd += GREG_DAYS_P_WEEK;
	}
	if (tgtw < 0) {
		tgty--;
		tgtw += __get_isowk(d1.y + tgty);
	}

	/* fill in the results */
	res.ywd.y = tgty;
	res.ywd.c = tgtw;
	res.ywd.w = tgtd;
	return res;
}
#endif	/* ASPECT_DIFF */


#if defined ASPECT_STRF && !defined YWD_ASPECT_STRF_
#define YWD_ASPECT_STRF_
DEFUN void
__prep_strfd_ywd(struct strpd_s *tgt, dt_ywd_t d)
{
/* place ywd data of THIS into D for printing with FMT. */
	if (d.c == 1 && d.hang < 0 && d.w < __ywd_get_jan01_wday(d)) {
		/* put gregorian year into y and real year into q */
		tgt->y = d.y - 1;
		tgt->q = d.y;
	} else if (d.c >= 53 && d.w >= __ywd_get_jan01_wday(d) +
		   1U/*coz 365 = 1 mod 7*/ + __leapp(d.y)) {
		/* put gregorian year into y and real year into q */
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
