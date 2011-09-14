/*** date-core.h -- our universe of dates
 *
 * Copyright (C) 2011 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of datetools.
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
/* implementation part of date-core.h */
#include <string.h>
#include "date-core.h"

#if defined DEBUG_FLAG
# include <stdio.h>
#endif	/* DEBUG_FLAG */
#if !defined LIKELY
# define LIKELY(_x)	__builtin_expect((_x), 1)
#endif
#if !defined UNLIKELY
# define UNLIKELY(_x)	__builtin_expect((_x), 0)
#endif
#if !defined countof
# define countof(x)	(sizeof(x) / sizeof(*(x)))
#endif	/* !countof */

/* weekdays of the first day of the year,
 * 3 bits per year, times 10 years makes 1 uint32_t */
typedef struct {
#define __JAN01_Y_PER_B		(10)
	unsigned int y0:3;
	unsigned int y1:3;
	unsigned int y2:3;
	unsigned int y3:3;
	unsigned int y4:3;
	unsigned int y5:3;
	unsigned int y6:3;
	unsigned int y7:3;
	unsigned int y8:3;
	unsigned int y9:3;
	/* 2 bits left */
	unsigned int rest:2;
} __jan01_wday_block_t;

#define M	(unsigned int)(DT_MONDAY)
#define T	(unsigned int)(DT_TUESDAY)
#define W	(unsigned int)(DT_WEDNESDAY)
#define R	(unsigned int)(DT_THURSDAY)
#define F	(unsigned int)(DT_FRIDAY)
#define A	(unsigned int)(DT_SATURDAY)
#define S	(unsigned int)(DT_SUNDAY)


/* helpers */
#define SECS_PER_MINUTE	(60)
#define SECS_PER_HOUR	(SECS_PER_MINUTE * 60)
#define SECS_PER_DAY	(SECS_PER_HOUR * 24)

static const __jan01_wday_block_t __jan01_wday[] = {
#define __JAN01_WDAY_BEG	(1970)
	{
		/* 1970 - 1979 */
		R, F, A, M, T, W, R, A, S, M, 0,
	}, {
		/* 1980 - 1989 */
		T, R, F, A, S, T, W, R, F, S, 0,
	}, {
		/* 1990 - 1999 */
		M, T, W, F, A, S, M, W, R, F, 0,
	}, {
		/* 2000 - 2009 */
		A, M, T, W, R, A, S, M, T, R, 0,
	}, {
		/* 2010 - 2019 */
		F, A, S, T, W, R, F, S, M, T, 0,
	}, {
		/* 2020 - 2029 */
		W, F, A, S, M, W, R, F, A, M, 0,
	}
#define __JAN01_WDAY_END	(2029)
};

static uint16_t __mon_yday[] = {
/* this is \sum ml, first element is a bit set of leap days to add */
	0xfff8, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

static const char *__long_wday[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
	"Miracleday",
};

static const char *__abbr_wday[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Mir",
};

static const char __abab_wday[] = "SMTWRFAX";

static const char *__long_mon[] = {
	"Miraculary",
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December",
};

static const char *__abbr_mon[] = {
	"Mir",
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
};

/* futures expiry codes, how convenient */
static const char __abab_mon[] = "_FGHJKMNQUVXZ";

/* stolen from Klaus Klein/David Laight's strptime() */
static const char*
strtoui_lim(uint32_t *tgt, const char *str, uint32_t llim, uint32_t ulim)
{
	uint32_t result = 0;
	char ch;
	/* The limit also determines the number of valid digits. */
	int rulim = ulim > 10 ? ulim : 10;

	ch = *str;
	if (ch < '0' || ch > '9') {
		return NULL;
	}
	do {
		result *= 10;
		result += ch - '0';
		rulim /= 10;
		ch = *++str;
	} while ((result * 10 <= ulim) && rulim && ch >= '0' && ch <= '9');

	if (result < llim || result > ulim) {
		return NULL;
	}
	*tgt = result;
	return str;
}

static size_t
ui32tostr(char *restrict buf, size_t bsz, uint32_t d, int pad)
{
/* all strings should be little */
#define C(x, d)	((x) / (d) % 10 + '0')
	size_t res;

	if (UNLIKELY(d > 10000)) {
		return 0;
	}
	switch ((res = pad) < bsz ? res : bsz) {
	case 4:
		buf[pad - 4] = C(d, 1000);
	case 3:
		buf[pad - 3] = C(d, 100);
	case 2:
		buf[pad - 2] = C(d, 10);
	case 1:
		buf[pad - 1] = C(d, 1);
		break;
	default:
	case 0:
		res = 0;
		break;
	}
	return res;
}

static const char*
strtoarri(uint32_t *tgt, const char *buf, const char *const *arr, size_t narr)
{
	for (size_t i = 0; i < narr; i++) {
		const char *chk = arr[i];
		size_t len = strlen(chk);

		if (strncasecmp(chk, buf, len) == 0) {
			*tgt = i;
			return buf + len;
		}
	}
	/* no matches */
	return NULL;
}

static size_t
arritostr(
	char *restrict buf, size_t bsz, size_t i,
	const char *const *arr, size_t narr)
{
	size_t ncp;
	size_t len;

	if (i > narr) {
		return 0;
	}
	len = strlen(arr[i]);
	ncp = bsz > len ? len : bsz;
	memcpy(buf, arr[i], ncp);
	return ncp;
}

static inline int
__leapp(int y)
{
	return y % 4 == 0;
}

static void
ffff_gmtime(struct tm *tm, const time_t t)
{
	register int days, yy;
	const uint16_t *ip;

	/* just go to day computation */
	days = t / SECS_PER_DAY;
	/* week day computation, that one's easy, 1 jan '70 was Thu */
	tm->tm_wday = (days + 4) % 7;

	/* gotta do the date now */
	yy = 1970;
	/* stolen from libc */
#define DIV(a, b)		((a) / (b))
/* we only care about 1901 to 2099 and there are no bullshit leap years */
#define LEAPS_TILL(y)		(DIV(y, 4))
	while (days < 0 || days >= (!__leapp(yy) ? 365 : 366)) {
		/* Guess a corrected year, assuming 365 days per year. */
		register int yg = yy + days / 365 - (days % 365 < 0);

		/* Adjust DAYS and Y to match the guessed year.  */
		days -= (yg - yy) * 365 +
			LEAPS_TILL(yg - 1) - LEAPS_TILL(yy - 1);
		yy = yg;
	}
	/* set the year */
	tm->tm_year = yy;

	ip = __mon_yday;
	/* unrolled */
	yy = 13;
	if (days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy] &&
	    days < ip[--yy]) {
		yy = 0;
	}
	/* set the rest of the tm structure */
	tm->tm_mday = days - ip[yy] + 1;
	tm->tm_yday = days;
	tm->tm_mon = yy;
	/* fix up leap years */
	if (UNLIKELY(__leapp(tm->tm_year))) {
		if ((ip[0] >> (yy)) & 1) {
			if (UNLIKELY(tm->tm_yday == 59)) {
				tm->tm_mon = 2;
				tm->tm_mday = 29;
			} else if (UNLIKELY(tm->tm_yday == ip[yy])) {
				tm->tm_mday = tm->tm_yday - ip[--tm->tm_mon];
			} else {
				tm->tm_mday--;
			}
		}
	}
	return;
}


/* converters and getters */
static inline __jan01_wday_block_t
__get_jan01_block(int year)
{
	return __jan01_wday[(year - __JAN01_WDAY_BEG) / __JAN01_Y_PER_B];
}

static inline dt_daisy_t
__jan00_daisy(int year)
{
/* daisy's base year is both 1 mod 4 and starts on a monday, so ... */
#define TO_BASE(x)	((x) - DT_DAISY_BASE_YEAR)
#define TO_YEAR(x)	((x) + DT_DAISY_BASE_YEAR)
	int by = TO_BASE(year);
	return by * 365 + by / 4;
}

static inline dt_dow_t
__get_wday(int year)
{
	unsigned int res;
	__jan01_wday_block_t j01b;

	if (UNLIKELY(year < __JAN01_WDAY_BEG || year > __JAN01_WDAY_END)) {
		return DT_MIRACLEDAY;
	}
	j01b = __get_jan01_block(year);

	switch (year % 10) {
	default:
		res = DT_MIRACLEDAY;
		break;
	case 0:
		res = j01b.y0;
		break;
	case 1:
		res = j01b.y1;
		break;
	case 2:
		res = j01b.y2;
		break;
	case 3:
		res = j01b.y3;
		break;
	case 4:
		res = j01b.y4;
		break;
	case 5:
		res = j01b.y5;
		break;
	case 6:
		res = j01b.y6;
		break;
	case 7:
		res = j01b.y7;
		break;
	case 8:
		res = j01b.y8;
		break;
	case 9:
		res = j01b.y9;
		break;
	}
	return (dt_dow_t)res;
}

static inline int
__get_mdays(int y, int m)
{
	int res = __mon_yday[m + 1] - __mon_yday[m];

	if (UNLIKELY(__leapp(y) && m == 2)) {
		res++;
	}
	return res;
}

static int
__ymd_get_yday(dt_ymd_t this)
{
	int res = this.d + __mon_yday[this.m];

	if (UNLIKELY(this.y == 0)) {
		return 0;
	} else if (UNLIKELY(__leapp(this.y))) {
		res += (__mon_yday[0] >> this.m) & 1;
	}
	return res;
}

static dt_dow_t
__ymd_get_wday(dt_ymd_t this)
{
	int yd;
	dt_dow_t j01_wd;
	if ((yd = __ymd_get_yday(this)) > 0 &&
	    (j01_wd = __get_wday(this.y)) != DT_MIRACLEDAY) {
		return (dt_dow_t)((yd - 1 + (unsigned int)j01_wd) % 7);
	}
	return DT_MIRACLEDAY;
}

static int
__ymd_get_count(dt_ymd_t this)
{
	if (UNLIKELY(this.d + 7 > __get_mdays(this.y, this.m))) {
		return 5;
	}
	return (this.d - 1) / 7 + 1;
}

static dt_dow_t
__ymcw_get_wday(dt_ymcw_t this)
{
	return (dt_dow_t)this.w;
}

static int
__ymcw_get_day(dt_ymcw_t this)
{
	int wd01;
	int wd_jan01;
	int res;

	/* weekday the year started with */
	wd_jan01 = __get_wday(this.y);
	/* see what weekday the first of the month was*/
	wd01 = __ymd_get_yday((dt_ymd_t){.y = this.y, .m = this.m, .d = 01});
	wd01 = (wd_jan01 - 1 + wd01) % 7;

	/* first WD1 is 1, second WD1 is 8, third WD1 is 15, etc.
	 * so the first WDx with WDx > WD1 is on (WDx - WD1) + 1 */
	res = (this.w + 7 - wd01) % 7 + 1 + 7 * (this.c - 1);
	/* not all months have a 5th X, so check for this */
	if (res > __get_mdays(this.y, this.m)) {
		 /* 5th = 4th in that case */
		res -= 7;
	}
	return res;
}

static dt_ymcw_t
__ymd_to_ymcw(dt_ymd_t d)
{
	int c = __ymd_get_count(d);
	int w = __ymd_get_wday(d);
	return (dt_ymcw_t){.y = d.y, .m = d.m, .c = c, .w = w};
}

static dt_daisy_t
__ymd_to_daisy(dt_ymd_t d)
{
/* compute days since 1917-01-01 (Mon),
 * if year slot is absent in D compute the day in the year of D instead. */
	dt_daisy_t res;
	int dy = TO_BASE(d.y);

	if (UNLIKELY(dy < 0)) {
		return 0;
	}
	res = __jan00_daisy(d.y);
	res += __mon_yday[d.m] + d.d;
	if (UNLIKELY(__leapp(d.y))) {
		res += (__mon_yday[0] >> (d.m)) & 1;
	}
	return res;
}

static dt_daisy_t
__ymcw_to_daisy(dt_ymcw_t d)
{
/* compute days since 1917-01-01 (Mon),
 * if year slot is absent in D compute the day in the year of D instead. */
	dt_daisy_t res;
	int dy = TO_BASE(d.y);

	if (UNLIKELY(dy < 0)) {
		return 0;
	}
	res = __jan00_daisy(d.y);
	res += __mon_yday[d.m];
	/* add up days too */
	res += __ymcw_get_day(d);
	if (UNLIKELY(__leapp(d.y))) {
		res += (__mon_yday[0] >> (d.m)) & 1;
	}
	return res;
}

static dt_dow_t
__daisy_get_wday(dt_daisy_t d)
{
/* daisy wdays are simple because the base year is chosen so that day 0
 * in the daisy calendar is a sunday */
	return (dt_dow_t)(d % 7);
}

static int
__daisy_get_year(dt_daisy_t d)
{
/* given days since 1917-01-01 (Mon), compute a year */
	int by;

	if (UNLIKELY(d == 0)) {
		return 0;
	}
	for (by = d / 365; __jan00_daisy(TO_YEAR(by)) >= d; by--);
	return TO_YEAR(by);
}

static dt_ymd_t
__daisy_to_ymd(dt_daisy_t this)
{
	dt_daisy_t j00;
	int doy;
	int y;
	int m;
	int d;

	if (UNLIKELY(this == 0)) {
		return (dt_ymd_t){.u = 0};
	}
	y = __daisy_get_year(this);
	j00 = __jan00_daisy(y);
	doy = this - j00;
	for (m = 1; m < 12 && doy > __mon_yday[m + 1]; m++);
	d = doy - __mon_yday[m];

	/* fix up leap years */
	if (UNLIKELY(__leapp(y))) {
		if ((__mon_yday[0] >> (m)) & 1) {
			if (UNLIKELY(doy == 60)) {
				m = 2;
				d = 29;
			} else if (UNLIKELY(doy == __mon_yday[m] + 1)) {
				m--;
				d = doy - __mon_yday[m] - 1;
			} else {
				d--;
			}
		}
	}
	return (dt_ymd_t){.y = y, .m = m, .d = d};
}

static dt_ymcw_t
__daisy_to_ymcw(dt_daisy_t this)
{
	dt_ymd_t tmp;
	int c;
	int w;

	if (UNLIKELY(this == 0)) {
		return (dt_ymcw_t){.u = 0};
	}
	tmp = __daisy_to_ymd(this);
	c = __ymd_get_count(tmp);
	w = __daisy_get_wday(this);
	return (dt_ymcw_t){.y = tmp.y, .m = tmp.m, .c = c, .w = w};
}

static dt_ymd_t
__ymcw_to_ymd(dt_ymcw_t d)
{
	int md = __ymcw_get_day(d);
	return (dt_ymd_t){.y = d.y, .m = d.m, .d = md};
}


/* converting accessors */
DEFUN dt_dow_t
dt_get_wday(struct dt_d_s this)
{
	switch (this.typ) {
	case DT_YMD:
		return __ymd_get_wday(this.ymd);
	case DT_YMCW:
		return __ymcw_get_wday(this.ymcw);
	case DT_DAISY:
		return __daisy_get_wday(this.daisy);
	default:
	case DT_UNK:
		return DT_MIRACLEDAY;
	}
}

DEFUN int
dt_get_mday(struct dt_d_s this)
{
	if (LIKELY(this.typ == DT_YMD)) {
		return this.ymd.d;
	}
	switch (this.typ) {
	case DT_YMCW:
		return __ymcw_get_day(this.ymcw);
	default:
	case DT_UNK:
		return 0;
	}
}

static int
dt_get_count(struct dt_d_s this)
{
	if (LIKELY(this.typ == DT_YMCW)) {
		return this.ymcw.c;
	}
	switch (this.typ) {
	case DT_YMD:
		return __ymd_get_count(this.ymd);
	default:
	case DT_UNK:
		return 0;
	}
}


/* converters */
static dt_daisy_t
dt_conv_to_daisy(struct dt_d_s this)
{
	switch (this.typ) {
	case DT_YMD:
		return __ymd_to_daisy(this.ymd);
	case DT_YMCW:
		return __ymcw_to_daisy(this.ymcw);
	case DT_DAISY:
		return this.daisy;
	case DT_BIZDA:
		break;
	case DT_UNK:
	default:
		break;
	}
	return 0;
}

static dt_ymd_t
dt_conv_to_ymd(struct dt_d_s this)
{
	switch (this.typ) {
	case DT_YMD:
		return this.ymd;
	case DT_YMCW:
		return __ymcw_to_ymd(this.ymcw);
	case DT_DAISY:
		return __daisy_to_ymd(this.daisy);
	case DT_BIZDA:
		break;
	case DT_UNK:
	default:
		break;
	}
	return (dt_ymd_t){.u = 0};
}

static dt_ymcw_t
dt_conv_to_ymcw(struct dt_d_s this)
{
	switch (this.typ) {
	case DT_YMD:
		return __ymd_to_ymcw(this.ymd);
	case DT_YMCW:
		return this.ymcw;
	case DT_DAISY:
		return __daisy_to_ymcw(this.daisy);
	case DT_BIZDA:
		break;
	case DT_UNK:
	default:
		break;
	}
	return (dt_ymcw_t){.u = 0};
}


/* guessing parsers */
static struct dt_d_s
__strpd_std(const char *str)
{
	struct dt_d_s res = {DT_UNK, 0};
	const char *sp = str;
	unsigned int y;
	unsigned int m;
	unsigned int d;
	unsigned int c;
	unsigned int w;

	if (sp == NULL) {
		goto out;
	}
	/* read the year */
	sp = strtoui_lim(&y, sp, 0, 9999);
	if (sp == NULL || *sp++ != '-') {
		goto out;
	}
	/* read the month */
	sp = strtoui_lim(&m, sp, 0, 12);
	if (sp == NULL || *sp++ != '-') {
		goto out;
	}
	/* read the day or the count */
	sp = strtoui_lim(&d, sp, 0, 31);
	if (sp == NULL) {
		/* didn't work, fuck off */
		goto out;
	}
	/* check the date type */
	switch (*sp++) {
	case '\0':
		/* it was a YMD date */
		res.typ = DT_YMD;
		goto assess;
	case '-':
		/* it is a YMCW date */
		res.typ = DT_YMCW;
		if ((c = d) > 5) {
			/* nope, it was bollocks */
			goto out;
		}
		break;
	case '<':
	case '>':
		/* it's a YMDU date */
		;
		break;
	default:
		/* it's fuckered */
		goto out;
	}
	sp = strtoui_lim(&w, sp, 0, 7);
	if (sp == NULL) {
		/* didn't work, fuck off */
		res.typ = DT_UNK;
		goto out;
	}
assess:
	switch (res.typ) {
	default:
	case DT_UNK:
		break;
	case DT_YMD:
		res.ymd.y = y;
		res.ymd.m = m;
		res.ymd.d = d;
		break;
	case DT_YMCW:
		res.ymcw.y = y;
		res.ymcw.m = m;
		res.ymcw.c = c;
		res.ymcw.w = w;
		break;
	}
out:
	return res;
}


/* implementations */
DEFUN struct dt_d_s
dt_strpd(const char *str, const char *fmt)
{
	struct dt_d_s res = {DT_UNK, 0};
	unsigned int y;
	unsigned int m;
	unsigned int d;
	unsigned int c;
	unsigned int w;

	if (UNLIKELY(fmt == NULL)) {
		return __strpd_std(str);
	}

	for (const char *fp = fmt, *sp = str; *fp && sp; fp++) {
		if (*fp == '%') {
			int shaught = 0;
			switch (*++fp) {
			default:
				goto literal;
			case 'F':
				shaught = 1;
			case 'Y':
				sp = strtoui_lim(&y, sp, 0, 9999);
				if (UNLIKELY(shaught == 0 || *sp++ != '-')) {
					break;
				}
			case 'm':
				sp = strtoui_lim(&m, sp, 0, 12);
				if (UNLIKELY(shaught == 0 || *sp++ != '-')) {
					break;
				}
			case 'd':
				/* gregorian mode */
				res.typ = DT_YMD;
				sp = strtoui_lim(&d, sp, 0, 31);
				break;
			case 'w':
				/* ymcw mode */
				res.typ = DT_YMCW;
				sp = strtoui_lim(&w, sp, 0, 7);
				break;
			case 'c':
				/* ymcw mode */
				res.typ = DT_YMCW;
				sp = strtoui_lim(&c, sp, 0, 5);
				break;
			case 'a':
				/* ymcw mode! */
				sp = strtoarri(
					&w, sp,
					__abbr_wday, countof(__abbr_wday));
				break;
			case 'A':
				/* ymcw mode! */
				sp = strtoarri(
					&w, sp,
					__long_wday, countof(__long_wday));
				break;
			case 'b':
			case 'h':
				sp = strtoarri(
					&m, sp,
					__abbr_mon, countof(__abbr_mon));
				break;
			case 'B':
				sp = strtoarri(
					&m, sp,
					__long_mon, countof(__long_mon));
				break;
			case 'y':
				sp = strtoui_lim(&y, sp, 0, 99);
				if ((y += 2000) > 2068) {
					y -= 100;
				}
				break;
			}
		} else {
		literal:
			if (*fp != *sp++) {
				res.typ = DT_UNK;
				break;
			}
		}
	}
	switch (res.typ) {
	default:
	case DT_UNK:
		break;
	case DT_YMD:
		res.ymd.y = y;
		res.ymd.m = m;
		res.ymd.d = d;
		break;
	case DT_YMCW:
		res.ymcw.y = y;
		res.ymcw.m = m;
		res.ymcw.c = c;
		res.ymcw.w = w;
		break;
	}
	return res;
}

DEFUN size_t
dt_strfd(char *restrict buf, size_t bsz, const char *fmt, struct dt_d_s this)
{
	size_t res = 0;
	unsigned int y;
	unsigned int m;
	unsigned int d;
	unsigned int c;

	if (UNLIKELY(buf == NULL || bsz == 0)) {
		goto out;
	}

	switch (this.typ) {
	case DT_YMD:
		y = this.ymd.y;
		m = this.ymd.m;
		d = this.ymd.d;
		if (fmt == NULL) {
			fmt = "%F\n";
		}
		break;
	case DT_YMCW:
		y = this.ymcw.y;
		m = this.ymcw.m;
		if (fmt == NULL) {
			fmt = "%Y-%m-%c-%w\n";
		}
		d = 0;
		break;
	case DT_DAISY: {
		dt_ymd_t tmp = __daisy_to_ymd(this.daisy);
		y = tmp.y;
		m = tmp.m;
		d = tmp.d;
		if (fmt == NULL) {
			/* subject to change */
			fmt = "%F\n";
		}
		break;
	}
	case DT_BIZDA:
		goto out;
	default:
	case DT_UNK:
		goto out;
	}

	for (const char *fp = fmt; *fp && res < bsz; fp++) {
		if (*fp == '%') {
			int shaught = 0;
			switch (*++fp) {
			default:
				goto literal;
			case 'F':
				shaught = 1;
			case 'Y':
				res += ui32tostr(buf + res, bsz - res, y, 4);
				if (UNLIKELY(shaught == 0)) {
					break;
				}
				buf[res++] = '-';
			case 'm':
				res += ui32tostr(buf + res, bsz - res, m, 2);
				if (UNLIKELY(shaught == 0)) {
					break;
				}
				buf[res++] = '-';
			case 'd':
				/* ymd mode check? */
				d = d ?: dt_get_mday(this);
				res += ui32tostr(buf + res, bsz - res, d, 2);
				break;
			case 'w':
				/* ymcw mode check */
				d = dt_get_wday(this);
				res += ui32tostr(buf + res, bsz - res, d, 2);
				break;
			case 'c':
				/* ymcw mode check? */
				c = dt_get_count(this);
				res += ui32tostr(buf + res, bsz - res, c, 2);
				break;
			case 'a':
				/* get the weekday in ymd mode!! */
				res += arritostr(
					buf + res, bsz - res,
					dt_get_wday(this),
					__abbr_wday, countof(__abbr_wday));
				break;
			case 'A':
				/* get the weekday in ymd mode!! */
				res += arritostr(
					buf + res, bsz - res,
					dt_get_wday(this),
					__long_wday, countof(__long_wday));
				break;
			case 'b':
			case 'h':
				res += arritostr(
					buf + res, bsz - res, m,
					__abbr_mon, countof(__abbr_mon));
				break;
			case 'B':
				res += arritostr(
					buf + res, bsz - res, m,
					__long_mon, countof(__long_mon));
				break;
			case 'y':
				res += ui32tostr(buf + res, bsz - res, y, 2);
				break;
			}
		} else {
		literal:
			buf[res++] = *fp;
		}
	}
	if (res > 0 && buf[res - 1] != '\n') {
		/* auto-newline */
		buf[res++] = '\n';
	}
out:
	if (res < bsz) {
		buf[res] = '\0';
	}
	return res;
}


/* date getters, platform dependent */
DEFUN struct dt_d_s
dt_date(dt_dtyp_t outtyp)
{
	struct dt_d_s res;
	time_t t = time(NULL);

	switch ((res.typ = outtyp)) {
	case DT_YMD:
	case DT_YMCW: {
		struct tm tm;
		ffff_gmtime(&tm, t);
		switch (res.typ) {
		case DT_YMD:
			res.ymd.y = tm.tm_year;
			res.ymd.m = tm.tm_mon;
			res.ymd.d = tm.tm_mday;
			break;
		case DT_YMCW: {
			dt_ymd_t tmp = {
				.y = tm.tm_year,
				.m = tm.tm_mon,
				.d = tm.tm_mday,
			};
			res.ymcw.y = tm.tm_year;
			res.ymcw.m = tm.tm_mon;
			res.ymcw.c = __ymd_get_count(tmp);
			res.ymcw.w = tm.tm_wday;
			break;
		}
		}
		break;
	}
	case DT_DAISY:
		/* time_t's base is 1970-01-01, which is daisy 19359 */
		res.daisy = t / 86400 + 19359;
		break;
	default:
	case DT_UNK:
		res.u = 0;
	}
	return res;
}

DEFUN struct dt_d_s
dt_conv(dt_dtyp_t tgttyp, struct dt_d_s d)
{
	struct dt_d_s res;

	switch ((res.typ = tgttyp)) {
	case DT_YMD:
		res.ymd = dt_conv_to_ymd(d);
		break;
	case DT_YMCW:
		res.ymcw = dt_conv_to_ymcw(d);
		break;
	case DT_DAISY:
		res.daisy = dt_conv_to_daisy(d);
		break;
	case DT_BIZDA:
		break;
	case DT_UNK:
	default:
		res.typ = DT_UNK;
		break;
	}
	return res;
}

DEFUN struct dt_d_s
dt_next_day(struct dt_d_s d, int increm)
{
	switch (d.typ) {
	case DT_DAISY:
		d.daisy += increm;
		break;
	case DT_UNK:
	default:
		d.typ = DT_UNK;
		d.u = 0;
		break;
	}
	return d;
}

/* date-core.c ends here */
