/* library tester */
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "date-core.h"

static const char test_ymd[] = "2001-02-03";
static const char test_ybd[] = "2001-Feb-03";

static void __attribute__((unused))
orig_strptime_perf(size_t nruns)
{
	for (size_t i = 0; i < nruns; i++) {
		struct tm tm;
		strptime(test_ymd, "%Y-%m-%d", &tm);
	}
	return;
}

static void __attribute__((unused))
test_strpd(size_t nruns)
{
	struct dt_d_s s;

	for (size_t i = 0; i < nruns; i++) {
		if ((s = dt_strpd(test_ymd, "%F")).u == 0) {
			break;
		}
	}
	if ((s = dt_strpd(test_ymd, "%F")).typ) {
		char buf[256];
		dt_strfd(buf, sizeof(buf), "%F %a %A %b %B\n", s);
		fputs(buf, stdout);
	}
	if ((s = dt_strpd(test_ybd, "%Y-%b-%d")).typ) {
		char buf[256];
		dt_strfd(buf, sizeof(buf), "%F %a %A %b %B\n", s);
		fputs(buf, stdout);
	}
	return;
}

static void __attribute__((unused))
test_date(size_t nruns)
{
	struct dt_d_s s;
	char buf[256];

	for (size_t i = 0; i < nruns; i++) {
		if ((s = dt_date(DT_YMCW)).u == 0) {
			break;
		}
	}
	dt_strfd(buf, sizeof(buf), "%Y-%m-%c-%w\n", s);
	fputs(buf, stdout);
	return;
}


int
main(int argc, char *argv[])
{
	const size_t nruns = 10000000;
#if 0
	orig_strptime_perf(nruns);
#elif 0
	test_strpd(nruns);
#elif 1
	test_date(nruns);
#endif
	return 0;
}

/* testlib.c ends here */
