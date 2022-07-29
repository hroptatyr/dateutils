/*** dtest.c -- like test(1) but for dates
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "dt-core.h"
#include "dt-io.h"
#include "dt-locale.h"

const char *prog = "dtest";


#include "dtest.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	char **ifmt;
	size_t nifmt;
	struct dt_dt_s d1, d2;
	zif_t fromz = NULL;
	int res = 0;

	if (yuck_parse(argi, argc, argv)) {
		res = 2;
		goto out;
	}


	if (argi->nargs != 1U + !argi->isvalid_flag) {
		yuck_auto_help(argi);
		res = 2;
		goto out;
	}
	if (argi->from_locale_arg) {
		setilocale(argi->from_locale_arg);
	}
	if (argi->from_zone_arg) {
		fromz = dt_io_zone(argi->from_zone_arg);
	}
	if (argi->base_arg) {
		struct dt_dt_s base = dt_strpdt(argi->base_arg, NULL, NULL);
		dt_set_base(base);
	}

	ifmt = argi->input_format_args;
	nifmt = argi->input_format_nargs;

	if (argi->isvalid_flag) {
		/* check that one date */
		char *ep = NULL;

		res = nifmt > 0U ||
			dt_unk_p(dt_strpdt(*argi->args, NULL, &ep)) ||
			ep == NULL || *ep;
		for (size_t i = 0; i < nifmt; i++, ep = NULL) {
			if (!dt_unk_p(dt_strpdt(*argi->args, ifmt[i], &ep)) &&
			    ep && !*ep) {
				res = 0;
				break;
			}
		}
		goto out;
	}
	if (dt_unk_p(d1 = dt_io_strpdt(argi->args[0U], ifmt, nifmt, fromz))) {
		if (!argi->quiet_flag) {
			dt_io_warn_strpdt(argi->args[0U]);
		}
		res = 2;
		goto out;
	}
	if (dt_unk_p(d2 = dt_io_strpdt(argi->args[1U], ifmt, nifmt, fromz))) {
		if (!argi->quiet_flag) {
			dt_io_warn_strpdt(argi->args[1U]);
		}
		res = 2;
		goto out;
	}

	/* just do the comparison */
	if ((res = dt_dtcmp(d1, d2)) == -2) {
		/* non-comparable */
		res = 3;
	} else if (argi->cmp_flag) {
		switch (res) {
		case 0:
			res = 0;
			break;
		case -1:
			res = 2;
			break;
		case 1:
			res = 1;
			break;
		default:
			res = 3;
			break;
		}
	} else if (argi->eq_flag) {
		res = res == 0 ? 0 : 1;
	} else if (argi->ne_flag) {
		res = res != 0 ? 0 : 1;
	} else if (argi->lt_flag || argi->ot_flag) {
		res = res == -1 ? 0 : 1;
	} else if (argi->le_flag) {
		res = res == -1 || res == 0 ? 0 : 1;
	} else if (argi->gt_flag || argi->nt_flag) {
		res = res == 1 ? 0 : 1;
	} else if (argi->ge_flag) {
		res = res == 1 || res == 0 ? 0 : 1;
	}
out:
	dt_io_clear_zones();
	if (argi->from_locale_arg) {
		setilocale(NULL);
	}

	yuck_free(argi);
	return res;
}

/* dtest.c ends here */
