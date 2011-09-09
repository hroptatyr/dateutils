/* library tester */
#include <unistd.h>
#include <time.h>

static void
orig_strptime_perf(size_t nruns)
{
	const char tstr[] = "2000-01-01";
	for (size_t i = 0; i < nruns; i++) {
		struct tm tm;
		strptime(tstr, "%Y-%m-%d", &tm);
	}
	return;
}

int
main(int argc, char *argv[])
{
	orig_strptime_perf(10000000);
	return 0;
}

/* testlib.c ends here */
