/*** dt-test.c -- just to work around getopt issues in dt-test.sh */

#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <unistd.h>
#include <stdio.h>

#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#endif	/* __INTEL_COMPILER */
#include "dt-test.xh"
#include "dt-test.x"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
# pragma warning (default:181)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	const char *srcdir;
	char *shpart;

	if (cmdline_parser(argc, argv, argi)) {
		goto out;
	} else if (argi->inputs_num != 1) {
		print_help_common();
		goto out;
	}

	if (argi->builddir_given) {
		setenv("builddir", argi->builddir_arg, 1);
	}
	if (argi->srcdir_given) {
		setenv("srcdir", argi->srcdir_arg, 1);
		srcdir = argi->srcdir_arg;
	} else {
		srcdir = getenv("srcdir");
	}
	if (argi->hash_given) {
		setenv("hash", argi->hash_arg, 1);
	}
	if (argi->husk_given) {
		setenv("husk", argi->husk_arg, 1);
	}

	/* just to be clear about this */
#if defined WORDS_BIGENDIAN
	setenv("endian", "big", 1);
#else  /* !WORDS_BIGENDIAN */
	setenv("endian", "little", 1);
#endif	/* WORDS_BIGENDIAN */

	/* bang the actual testfile */
	setenv("testfile", argi->inputs[0], 1);

	/* promote srcdir */
	if (srcdir) {
		static char buf[4096];
		ssize_t res;

		if ((res = readlink(srcdir, buf, sizeof(buf))) >= 0) {
			buf[res] = '\0';
			srcdir = buf;
		}
	}

	/* build the command */
	if ((shpart = argi->shell_bits_arg) == NULL ||
	    (shpart[0] != '/' && shpart[0] != '.')) {
		static const char fn[] = "dt-test.sh";
		static char buf[4096];
		size_t idx = 0UL;

		if (srcdir && (idx = strlen(srcdir))) {
			memcpy(buf, srcdir, idx);
			if (buf[idx - 1] != '/') {
				buf[idx++] = '/';
			}
		}
		if (!argi->shell_bits_given) {
			memcpy(buf + idx, fn, sizeof(fn));
		} else {
			size_t z;
			memcpy(buf + idx, shpart, z = strlen(shpart));
			buf[idx + z] = '\0';
		}
		shpart = buf;
	}

	/* exec the test script */
	{
		char *const new_argv[] = {"dt-test", shpart, NULL};

		if (execv("/bin/sh", new_argv)) {
			perror("shell part not found");
		}
	}

out:
	cmdline_parser_free(argi);
	/* never succeed */
	return 1;
}

/* dt-test.c ends here */
