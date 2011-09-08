/*** dcal.c -- convert calendrical systems
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
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

typedef struct plate_s *plate_t;

typedef enum {
	PLMODE_UNK,
	PLMODE_D2P,
	PLMODE_P2D,
} plmode_t;

struct plate_s {
	int yy;
	int m;
	int d;
	int mw;
	int wd;
};

/* week days of the years 1970 to 2038 */
static const uint8_t jan01_wday[] = {
	/* 1970 */
	4, 5, 6, 1, 2, 3, 4, 6,
	/* 1978 */
	0, 1, 2, 4, 5, 6, 0, 2,
	/* 1986 */
	3, 4, 5, 0, 1, 2, 3, 5,
	/* 1994 */
	6, 0, 1, 3, 4, 5, 6, 1,
	/* 2002 */
	2, 3, 4, 6, 0, 1, 2, 4,
	/* 2010 */
	5, 6, 0, 2, 3, 4, 5, 0,
	/* 2018 */
	1, 2, 3, 5, 6, 0, 1, 3,
	/* 2026 */
	4, 5, 6, 1, 2, 3, 4, 6,
};

static const unsigned short int __mon_yday[2][13] = {
	/* Normal years.  */
	{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
	/* Leap years.  */
	{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};


static inline int
__leapp(int y)
{
	return y % 4 == 0;
}

static inline int
days_in_month(int y, int m)
{
	/* just to keep track of the leap-year property */
	int l = __leapp(y);
	/* days in month */
	return __mon_yday[l][m] - __mon_yday[l][m - 1];
}

static int
plate_mweek(plate_t p)
{
/* anything <=7 is the first in that month */
	/* special case, check if it's the last of the month */
	if (p->d + 7 > days_in_month(p->yy, p->m)) {
		return 5;
	}
	return (p->d - 1) / 7 + 1;
}

static int
yday(int y, int m, int d)
{
	return d + __mon_yday[__leapp(y)][m - 1];
}

static int
plate_wday(plate_t p)
{
/* compute wday from yy, m, and d */
	int yd = yday(p->yy, p->m, p->d) - 1;
	int wd = (yd + jan01_wday[p->yy - 1970]) % 7;
	return wd ?: 7;
}

/* stolen from ffff_date_dse_to_tm() */
#define SECS_PER_MINUTE	(60)
#define SECS_PER_HOUR	(SECS_PER_MINUTE * 60)
#define SECS_PER_DAY	(SECS_PER_HOUR * 24)

static void
ffff_gmtime(plate_t p, time_t t)
{
	register int days, yy;
	const unsigned short int *ip;

	/* just go to day computation */
	days = t / SECS_PER_DAY;
	/* week day computation, that one's easy, 1 jan '70 was Thu */
	p->wd = (days + 4) % 7 ?: 7;

	/* gotta do the date now */
	yy = 1970;
	/* stolen from libc */
#define DIV(a, b)		((a) / (b))
/* we only care about 1970 to 2099 and there are no bullshit leap years */
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
	p->yy = yy;

	ip = __mon_yday[__leapp(yy)];
	/* unrolled */
	yy = 12;
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
	//p->yd = days;
	p->m = yy + 1;
	p->d = days - ip[yy] + 1;
	p->mw = plate_mweek(p);
	return;
}

static void
now_plate(plate_t p)
{
	time_t t[1];

	time(t);
	ffff_gmtime(p, t[0]);
	return;
}

static void __attribute__((unused))
month_decr(plate_t p)
{
	if (--p->m <= 0) {
		p->m += 12;
		p->yy--;
	}
	return;
}

static void __attribute__((unused))
month_incr(plate_t p)
{
	if (++p->m > 12) {
		p->m -= 12;
		p->yy++;
	}
	return;
}

static void
massage_date(plate_t p)
{
	/* massage the plate first */
	if (p->d <= 0) {
		month_decr(p);
		p->d += days_in_month(p->yy, p->m);
	} else if (p->m <= 0) {
		month_decr(p);
	}
	return;
}

static void
massage_plate(plate_t p)
{
	if (p->m == 0 && p->mw == 0) {
		p->m = 12;
		p->mw = 5;
		p->yy--;
	} else if (p->m == 0 /*&& p->mw > 0*/) {
		p->mw = 1;
	} else if (p->mw == 0) {
		month_decr(p);
		p->mw += 5;
	}
	return;
}


static int
plate_to_date(plate_t p)
{
	int wd1;

	/* massage the plate first */
	massage_plate(p);
	/* see what weekday the first of the month was */
	wd1 = yday(p->yy, p->m, jan01_wday[p->yy - 1970]);
	/* now obtain weekday value (0 = sun, 1 = mon, 2 = ...) */
	wd1 %= 7;
	/* first WD1 is 1, second WD1 is 8, third WD1 is 15, etc.
	 * so the first WDx with WDx > WD1 is on (WDx - WD1) + 1 */
	p->d = p->wd + 7 * p->mw - wd1 + (p->wd < wd1 ?: -6);
	/* look to find the last of the month correctly */
	if (p->d > days_in_month(p->yy, p->m)) {
		p->d -= 7;
	}
	return 0;
}

static int
date_to_plate(plate_t p)
{
	/* massage the date first */
	massage_date(p);

	p->mw = plate_mweek(p);
	p->wd = plate_wday(p);
	return 0;
}


static int
parse(plate_t p, const char *s)
{
	char *on;
	unsigned int tmp;

	p->yy = strtoul(s, &on, 10);
	if (*on++ != '-') {
		return PLMODE_UNK;
	}
	p->m = strtoul(on, &on, 10);
	if (*on++ != '-') {
		return PLMODE_UNK;
	}
	/* now there's excitement, could be a date or a plate */
	tmp = strtoul(on, &on, 10);
	if (*on == '\0' && tmp > 0) {
		p->d = tmp;
		return PLMODE_D2P;
	} else if (*on++ == '-' && tmp <= 5) {
		/* we must be in plate mode */
		p->mw = tmp;
		p->wd = strtoul(on, &on, 10) % 7 ?: 7;
		if (*on == '\0') {
			return PLMODE_P2D;
		}
	}
	return PLMODE_UNK;
}

static int
dcal_conv(int mode, struct plate_s *p)
{
	int res;

	switch (mode) {
	case PLMODE_D2P:
		res = date_to_plate(p);
		printf("%04d-%02d-%02d-%02d\n", p->yy, p->m, p->mw, p->wd);
		break;
	case PLMODE_P2D:
		res = plate_to_date(p);
		printf("%04d-%02d-%02d\n", p->yy, p->m, p->d);
		break;
	default:
		res = 1;
	}
	return res;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
#endif	/* __INTEL_COMPILER */
#include "dcal-clo.h"
#include "dcal-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	struct plate_s p[1] = {{0}};
	int mode = 0;
	int res = 0;

	if (cmdline_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	}

	if (argi->inputs_num) {
		for (size_t i = 0; i < argi->inputs_num; i++) {
			if ((mode = parse(p, argi->inputs[i])) > PLMODE_UNK) {
				dcal_conv(mode, p);
			}
		}
	} else {
		/* read from stdin */
		FILE *fp = stdin;
		char *line;
		size_t lno = 0;

		/* no threads reading this stream */
		__fsetlocking(fp, FSETLOCKING_BYCALLER);

		for (line = NULL; !feof_unlocked(fp); lno++) {
			ssize_t n;
			size_t len;

			n = getline(&line, &len, fp);
			if (n < 0) {
				break;
			}
			/* terminate the string accordingly */
			line[n - 1] = '\0';
			/* check if line matches */
			if ((mode = parse(p, line)) > PLMODE_UNK) {
				dcal_conv(mode, p);
			}
		}
		/* get rid of resources */
		free(line);
	}

out:
	cmdline_parser_free(argi);
	return res;
}

/* dcal.c ends here */
