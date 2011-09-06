/* like seq(1) but for date sequences */

#include <stdlib.h>
#include <stdio.h>
#if !defined __USE_XOPEN
# define __USE_XOPEN
#endif	/* !XOPEN */
#include <time.h>
#include <string.h>
#include <getopt.h>

static char *fmt = "%Y-%m-%d";

static void
pr_ts(time_t ts)
{
	struct tm tm;
	static char b[32];

	memset(&tm, 0, sizeof(tm));
	(void)gmtime_r(&ts, &tm);
	strftime(b, sizeof(b), fmt, &tm);
	fputs(b, stdout);
	return;
}

static time_t
sc_ts(const char *s)
{
	struct tm tm;

	/* basic sanity check */
	if (s == NULL) {
		return 0;
	}
	/* wipe tm */
	memset(&tm, 0, sizeof(tm));
	(void)strptime(s, fmt, &tm);
	return mktime(&tm);
}

static void
usage(void)
{
	fputs("\
Usage: dseq [OPTION] FIRST\n\
  or:  dseq [OPTION] FIRST LAST\n\
", stdout);
	fputs("\
Print dates from FIRST to LAST (or today if omitted).\n\
\n\
  -f, --format=FORMAT      use printf style floating-point FORMAT\n\
  -s, --separator=STRING   use STRING to separate numbers (default: \\n)\n\
", stdout);
	exit(0);
}

static struct option const long_options[] = {
	{ "format", required_argument, NULL, 'f'},
	{ "separator", required_argument, NULL, 's'},
	{ NULL, 0, NULL, 0}
};

int
main(int argc, char *argv[])
{
	time_t fst, lst;
	int inc = 86400;
	int optc;
	char *sep = "\n";

	if (argc < 2) {
		usage();
	}

	while (optind < argc) {
		optc = getopt_long(argc, argv, "+f:s", long_options, NULL);
		if (optc == -1) {
			break;
		}

		switch (optc) {
		case 'f':
			fmt = optarg;
			break;
		case 's':
			sep = optarg;
			break;
		default:
			usage();
		}
	}

	fst = sc_ts(argv[optind++]);
	if (argc == optind) {
		lst = time(NULL);
	} else {
		lst = sc_ts(argv[optind++]);
	}

	while (fst <= lst) {
		pr_ts(fst);
		fputs(sep, stdout);
		fst += inc;
	}
	return 0;
}

/* dseq.c ends here */
