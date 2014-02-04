/* metaprogramming */
#if !defined INCLUDED_gmtime_h_
#define INCLUDED_gmtime_h_

/* UTC has a constant day length */
#define UTC_SECS_PER_DAY	(86400)

static void
ffff_gmtime(struct tm *tm, const time_t t)
{
	static uint16_t __mon_yday[] = {
		/* this is \sum ml,
		 * first element is a bit set of leap days to add */
		0xfff8, 0,
		31, 59, 90, 120, 151, 181,
		212, 243, 273, 304, 334, 365
	};
	register int days;
	register unsigned int yy;
	const uint16_t *ip;

	/* just go to day computation */
	days = (int)(t / UTC_SECS_PER_DAY);
	/* week day computation, that one's easy, 1 jan '70 was Thu */
	tm->tm_wday = (days + 4) % GREG_DAYS_P_WEEK;

	/* gotta do the date now */
	yy = 1970;
	/* stolen from libc */
#define DIV(a, b)		((a) / (b))
/* we only care about 1901 to 2099 and there are no bullshit leap years */
#define LEAPS_TILL(y)		(DIV(y, 4))
	while (days < 0 || days >= (!__leapp(yy) ? 365 : 366)) {
		/* Guess a corrected year, assuming 365 days per year. */
		register unsigned int yg = yy + days / 365 - (days % 365 < 0);

		/* Adjust DAYS and Y to match the guessed year.  */
		days -= (yg - yy) * 365 +
			LEAPS_TILL(yg - 1) - LEAPS_TILL(yy - 1);
		yy = yg;
	}
	/* set the year */
	tm->tm_year = (int)yy;

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
		yy = 1;
	}
	/* set the rest of the tm structure */
	tm->tm_mday = days - ip[yy] + 1;
	tm->tm_yday = days;
	tm->tm_mon = (int)yy;
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
#if defined FFFF_GMTIME_SUBDAY
	tm->tm_hour = (t % UTC_SECS_PER_DAY) / 60U / 60U;
	tm->tm_min = (tm->tm_sec % 3600U) / 60U;
	tm->tm_sec = (tm->tm_sec % 60U);
#endif	/* FFFF_GMTIME_SUBDAY */
	return;
}

#endif	/* INCLUDED_gmtime_h_ */
