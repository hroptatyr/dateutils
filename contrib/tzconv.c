/*** tzconv.c -- convert matlab dates between timezones
 *
 * Copyright (C) 2013-2019 Sebastian Freundt
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
 ***/
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
/* matlab stuff */
#if defined HAVE_OCTAVE_MEX_H
# include <octave/mex.h>
#else  /* !HAVE_OCTAVE_MEX_H */
# include <mex.h>
#endif	/* HAVE_OCTAVE_MEX_H */
/* our stuff */
#include "tzraw.h"

/* see tzconv.m for details */

static zif_t
find_zone(const mxArray *zstr)
{
	char *zone = mxArrayToString(zstr);
	zif_t res;

	res = zif_open(zone);
	if (zone != NULL) {
		mxFree(zone);
	}
	return res;
}


void
mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	zif_t fromz;
	zif_t toz;

	if (nrhs != 3 || nlhs > 1) {
		mexErrMsgTxt("invalid usage, see `help tzconv'\n");
		return;
	}

	/* find zones */
	if ((fromz = find_zone(prhs[1])) == NULL) {
		mexErrMsgTxt("cannot open from zone\n");
	}
	if ((toz = find_zone(prhs[2])) == NULL) {
		zif_close(fromz);
		mexErrMsgTxt("cannot open target zone\n");
	}

#define TO_UNIX(x)	((x) - 719529.0) * 86400.0
#define TO_MATL(x)	((x) / 86400.0) + 719529.0
	{
		mwSize m = mxGetM(prhs[0]);
		mwSize n = mxGetN(prhs[0]);
		const double *src = mxGetPr(prhs[0]);
		double *tgt;

		plhs[0] = mxCreateDoubleMatrix(m, n, mxREAL);
		tgt = mxGetPr(plhs[0]);

		for (mwSize i = 0; i < m * n; i++) {
			double x = TO_UNIX(src[i]);

			if (x < 2147483647.0 && x > -2147483648.0) {
				double frac = modf(x, &x);
				int32_t utc = zif_utc_time(fromz, (int32_t)x);
				int32_t lcl = zif_local_time(toz, utc);

				tgt[i] = TO_MATL((double)lcl) + frac / 86400.0;
			} else {
				tgt[i] = NAN;
			}
		}
	}

	zif_close(fromz);
	zif_close(toz);
	return;
}

/* gand_get_series.c ends here */
