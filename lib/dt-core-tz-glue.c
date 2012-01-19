/*** dt-core-tz-glue.c -- gluing date/times and tzs
 *
 * Copyright (C) 2012 Sebastian Freundt
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
/* dt-core-tz-glue.h implementation */
#if !defined INCLUDED_dt_core_tz_glue_c_
#define INCLUDED_dt_core_tz_glue_c_

#include "dt-core-tz-glue.h"

#define DAISY_UNIX_BASE		(19359)

static inline int
__pos_mod(int num, int mod)
{
	int res = num % mod;

	if (UNLIKELY(res < 0)) {
		return res + mod;
	}
	return res;
}


/**
 * Return a dt object that forgot about DT's zone and uses ZONE instead. */
DEFUN struct dt_dt_s
dtz_forgetz(struct dt_dt_s dt, zif_t zone)
{
	dt_daisy_t d = dt_conv_to_daisy(dt.d);
	struct dt_dt_s res = dt_dt_initialiser();
	int32_t d_unix = (d - DAISY_UNIX_BASE) * 86400 +
		(dt.t.hms.h * 60 + dt.t.hms.m) * 60 + dt.t.hms.s;
	int32_t d_utc = zif_utc_time(zone, d_unix);

	/* convert the date part back */
	{
#if defined __C1X
		struct dt_d_s tmp = {
			.typ = DT_DAISY,
			.daisy = d_utc / 86400 + DAISY_UNIX_BASE,
		};
#else  /* !__C1X */
		struct dt_d_s tmp;
		tmp.typ = DT_DAISY;
		tmp.daisy = d_utc / 86400 + DAISY_UNIX_BASE;
#endif	/* __C1X */

		res.d = dt_conv(dt.d.typ, tmp);
	}

	/* convert the time part back */
	{
		int32_t sexy = __pos_mod(d_utc, 86400);
#if defined __C1X
		res.t.hms = (dt_hms_t){
			.s = sexy % 60,
			.m = (sexy % 3600) / 60,
			.h = sexy / 3600,
		};
#else  /* !__C1X */
		res.t.hms.s = sexy % 60;
		res.t.hms.m = (sexy % 3600) / 60;
		res.t.hms.h = sexy / 3600;
#endif	/* __C1X */
	}
	return res;
}

/**
 * Return a dt object from a UTC'd DT that uses ZONE. */
DEFUN struct dt_dt_s
dtz_enrichz(struct dt_dt_s dt, zif_t zone)
{
	dt_daisy_t d = dt_conv_to_daisy(dt.d);
	struct dt_dt_s res = dt_dt_initialiser();
	int32_t d_unix = (d - DAISY_UNIX_BASE) * 86400 +
		(dt.t.hms.h * 60 + dt.t.hms.m) * 60 + dt.t.hms.s;
	int32_t d_loc = zif_local_time(zone, d_unix);

	/* convert the date part back */
	{
#if defined __C1X
		struct dt_d_s tmp = {
			.typ = DT_DAISY,
			.daisy = d_loc / 86400 + DAISY_UNIX_BASE,
		};
#else  /* !__C1X */
		struct dt_d_s tmp;
		tmp.typ = DT_DAISY;
		tmp.daisy = d_loc / 86400 + DAISY_UNIX_BASE;
#endif	/* __C1X */

		res.d = dt_conv(dt.d.typ, tmp);
	}

	/* convert the time part back */
	{
		int32_t sexy = __pos_mod(d_loc, 86400);
#if defined __C1X
		res.t.hms = (dt_hms_t){
			.s = sexy % 60,
			.m = (sexy % 3600) / 60,
			.h = sexy / 3600,
		};
#else  /* !__C1X */
		res.t.hms.s = sexy % 60;
		res.t.hms.m = (sexy % 3600) / 60;
		res.t.hms.h = sexy / 3600;
#endif	/* __C1X */
	}
	return res;
}

#endif	/* INCLUDED_dt_core_tz_glue_c_ */
/* dt-core-tz-glue.c ends here */