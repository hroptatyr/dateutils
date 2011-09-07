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


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
#endif	/* __INTEL_COMPILER */
#include "dseq-clo.h"
#include "dseq-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	time_t fst, lst;
	long int ite = 1;
	char *sep = "\n";
	int res = 0;

	if (cmdline_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	}

	switch (argi->inputs_num) {
	default:
		cmdline_parser_print_help();
		res = 1;
		goto out;
	case 1:
		fst = sc_ts(argi->inputs[0]);
		lst = time(NULL);
		break;
	case 2:
		fst = sc_ts(argi->inputs[0]);
		lst = sc_ts(argi->inputs[1]);
		break;
	case 3:
		fst = sc_ts(argi->inputs[0]);
		ite = strtol(argi->inputs[1], NULL, 10);
		lst = sc_ts(argi->inputs[2]);
		break;
	}

	while (fst <= lst) {
		pr_ts(fst);
		fputs(sep, stdout);
		fst += ite * 86400;
	}
out:
	cmdline_parser_free(argi);
	return res;
}

/* dseq.c ends here */
