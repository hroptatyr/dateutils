/*** dsort.c -- sort FILEs or stdin chronologically
 *
 * Copyright (C) 2011-2022 Sebastian Freundt
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
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>

#include "dt-core.h"
#include "dt-io.h"
#include "dt-locale.h"
#include "prchunk.h"

const char *prog = "dsort";

struct prln_ctx_s {
	struct grep_atom_soa_s *ndl;
	zif_t fromz;
	int outfd;
};

struct sort_ctx_s {
	unsigned int revp:1U;
	unsigned int unqp:1U;
};


static void
safe_write(int fd, const char *buf, size_t bsz)
{
	size_t tot = 0U;
	for (ssize_t nwr;
	     tot < bsz && (nwr = write(fd, buf + tot, bsz - tot)) >= 0;
	     tot += nwr);
	return;
}

static void
proc_line(struct prln_ctx_s ctx, char *line, size_t llen)
{
	struct dt_dt_s d;

	do {
		char buf[64U];
		char *sp, *tp;
		char *bp = buf;
		const char *const ep = buf + sizeof(buf);

		/* find first occurrence then */
		d = dt_io_find_strpdt2(
			line, llen, ctx.ndl, &sp, &tp, ctx.fromz);
		/* print line, first thing */
		safe_write(ctx.outfd, line, llen);

		/* extend by separator */
		*bp++ = '\001';
		/* check if line matches */
		if (!dt_unk_p(d)) {
			/* match! */
			if (!dt_sandwich_only_t_p(d)) {
				bp += dt_strfdt(bp, ep - bp, "%F", d);
			}
			*bp++ = '\001';
			if (!dt_sandwich_only_d_p(d)) {
				bp += dt_strfdt(bp, ep - bp, "%T", d);
			}
		} else {
			/* just two empty fields then, innit? */
			*bp++ = '\001';
		}
		/* finalise the line and print */
		*bp++ = '\n';
		safe_write(ctx.outfd, buf, bp - buf);
	} while (0);
	return;
}

static int
proc_file(struct prln_ctx_s prln, const char *fn)
{
	size_t lno = 0;
	void *pctx;
	int fd;

	if (fn == NULL) {
		/* stdin then innit */
		fd = STDIN_FILENO;
	} else if ((fd = open(fn, O_RDONLY)) < 0) {
		serror("Error: cannot open file `%s'", fn);
		return -1;
	}

	/* using the prchunk reader now */
	if ((pctx = init_prchunk(fd)) == NULL) {
		serror("Error: cannot read from `%s'", fn ?: "<stdin>");
		return -1;
	}

	while (prchunk_fill(pctx) >= 0) {
		for (char *line; prchunk_haslinep(pctx); lno++) {
			size_t llen = prchunk_getline(pctx, &line);

			proc_line(prln, line, llen);
		}
	}
	/* get rid of resources */
	free_prchunk(pctx);
	close(fd);
	return 0;
}


/* helper children, sort(1) and cut(1) */
static pid_t
spawn_sort(int *restrict infd, const int outfd, struct sort_ctx_s sopt)
{
	static char *cmdline[16U] = {"sort", "-t", "-k2"};
	pid_t sortp;
	/* to snarf off traffic from the child */
	int intfd[2];

	if (pipe(intfd) < 0) {
		serror("pipe setup to/from sort failed");
		return -1;
	}

	switch ((sortp = vfork())) {
	case -1:
		/* i am an error */
		serror("vfork for sort failed");
		return -1;

	default:
		/* i am the parent */
		close(intfd[0]);
		*infd = intfd[1];
		/* close outfd here already */
		close(outfd);
		return sortp;

	case 0:;
		char **cp = cmdline + 3U;

		/* i am the child */
		if (sopt.revp) {
			*cp++ = "-r";
		}
		if (sopt.unqp) {
			*cp++ = "-u";
		}
		*cp++ = NULL;

		/* stdout -> outfd */
		dup2(outfd, STDOUT_FILENO);
		/* *infd -> stdin */
		dup2(intfd[0], STDIN_FILENO);
		close(intfd[1]);

		execvp("sort", cmdline);
		serror("execvp(sort) failed");
		_exit(EXIT_FAILURE);
	}
}

static pid_t
spawn_cut(int *restrict infd)
{
	static char *const cmdline[] = {"cut", "-d", "-f1", NULL};
	pid_t cutp;
	/* to snarf off traffic from the child */
	int intfd[2];

	if (pipe(intfd) < 0) {
		serror("pipe setup to/from cut failed");
		return -1;
	}

	switch ((cutp = vfork())) {
	case -1:
		/* i am an error */
		serror("vfork for cut failed");
		return -1;

	default:;
		/* i am the parent */
		close(intfd[0]);
		*infd = intfd[1];
		return cutp;

	case 0:;
		/* i am the child */
		dup2(intfd[0], STDIN_FILENO);
		close(intfd[1]);

		execvp("cut", cmdline);
		serror("execvp(cut) failed");
		_exit(EXIT_FAILURE);
	}
}


#include "dsort.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	char **fmt;
	size_t nfmt;
	zif_t fromz = NULL;
	int rc = 0;
	struct sort_ctx_s sopt = {0U};

	if (yuck_parse(argi, argc, argv)) {
		rc = 1;
		goto out;
	}
	/* init and unescape sequences, maybe */
	fmt = argi->input_format_args;
	nfmt = argi->input_format_nargs;
	if (argi->backslash_escapes_flag) {
		for (size_t i = 0; i < nfmt; i++) {
			dt_io_unescape(fmt[i]);
		}
	}

	if (argi->from_locale_arg) {
		setilocale(argi->from_locale_arg);
	}
	/* try and read the from and to time zones */
	if (argi->from_zone_arg &&
	    (fromz = dt_io_zone(argi->from_zone_arg)) == NULL) {
		error("\
Error: cannot find zone specified in --from-zone: `%s'", argi->from_zone_arg);
		rc = 1;
		goto clear;
	}
	if (argi->base_arg) {
		struct dt_dt_s base = dt_strpdt(argi->base_arg, NULL, NULL);
		dt_set_base(base);
	}

	/* prepare a mini-argi for the sort invocation */
	if (argi->reverse_flag) {
		sopt.revp = 1U;
	}
	if (argi->unique_flag) {
		sopt.unqp = 1U;
	}

	{
		/* process all files */
		struct grep_atom_s __nstk[16], *needle = __nstk;
		size_t nneedle = countof(__nstk);
		struct grep_atom_soa_s ndlsoa;
		struct prln_ctx_s prln = {
			.ndl = &ndlsoa,
			.fromz = fromz,
		};
		pid_t cutp, sortp;

		/* lest we overflow the stack */
		if (nfmt >= nneedle) {
			/* round to the nearest 8-multiple */
			nneedle = (nfmt | 7) + 1;
			needle = calloc(nneedle, sizeof(*needle));
		}
		/* and now build the needles */
		ndlsoa = build_needle(needle, nneedle, fmt, nfmt);

		/* spawn children */
		with (int ifd, ofd) {
			if ((cutp = spawn_cut(&ifd)) < 0) {
				goto ndl_free;
			}
			if ((sortp = spawn_sort(&ofd, ifd, sopt)) < 0) {
				goto ndl_free;
			}
			prln.outfd = ofd;
		}

		for (size_t i = 0U; i < argi->nargs || i == 0U; i++) {
			if (proc_file(prln, argi->args[i]) < 0) {
				rc = 1;
			}
		}

		/* indicate we're no longer writing to the sort helper */
		close(prln.outfd);

		/* wait for sort first */
		with (int st) {
			while (waitpid(sortp, &st, 0) != sortp);
			if (WIFEXITED(st) && WEXITSTATUS(st)) {
				rc = rc ?: WEXITSTATUS(st);
			}
		}
		/* wait for cut then */
		with (int st) {
			while (waitpid(cutp, &st, 0) != cutp);
			if (WIFEXITED(st) && WEXITSTATUS(st)) {
				rc = rc ?: WEXITSTATUS(st);
			}
		}

	ndl_free:
		if (needle != __nstk) {
			free(needle);
		}
	}

clear:
	dt_io_clear_zones();
	if (argi->from_locale_arg) {
		setilocale(NULL);
	}

out:
	yuck_free(argi);
	return rc;
}

/* dsort.c ends here */
