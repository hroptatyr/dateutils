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

#include "nifty.h"
#include "dt-core-tz-glue.h"

#define DAISY_UNIX_BASE		(19359)

static inline dt_ssexy_t
____to_unix_epoch(struct dt_dt_s dt)
{
/* daisy is competing with the prevalent unix epoch, this is the offset */
#define DAISY_UNIX_BASE		(19359)
	if (dt.typ == DT_SEXY) {
		/* no way to find out, is there */
		return dt.sexy;
	} else if (dt_sandwich_p(dt) || dt_sandwich_only_d_p(dt)) {
		struct dt_d_s d = dt_dconv(DT_DAISY, dt.d);
		dt_daisy_t dd = d.daisy;
		dt_ssexy_t res = (dd - DAISY_UNIX_BASE) * SECS_PER_DAY;
		if (dt_sandwich_p(dt)) {
			res += (dt.t.hms.h * 60 + dt.t.hms.m) * 60 + dt.t.hms.s;
		}
		return res;
	}
	return 0;
}

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
dtz_forgetz(struct dt_dt_s d, zif_t zone)
{
	struct dt_dt_s res;
	dt_ssexy_t d_unix;

	if (dt_sandwich_only_d_p(d) || dt_sandwich_only_t_p(d)) {
		return d;
	}

	/* convert date/time part to unix stamp */
	d_unix = ____to_unix_epoch(d);
	d_unix = zif_utc_time(zone, d_unix);

	/* convert the date part back */
	if (d.typ > DT_DUNK && d.typ < DT_NDTYP) {
		int32_t sexy = __pos_mod(d_unix, 86400);

		/* temporarily go daisy */
		res.d.typ = DT_DAISY;
		res.d.daisy = d_unix / 86400 + DAISY_UNIX_BASE;
		res.d = dt_dconv(d.d.typ, res.d);

		/* set the other flags too */
		res.sandwich = d.sandwich;
		res.dur = 0;
		res.neg = 0;

		/* convert the time part back */
		res.t.hms.s = sexy % 60;
		res.t.hms.m = (sexy % 3600) / 60;
		res.t.hms.h = sexy / 3600;
		res.t.hms.ns = d.t.hms.ns;

		res.t.typ = DT_HMS;
		res.t.dur = 0;
		res.t.neg = 0;
		res.t.carry = 0;

	} else if (d.typ == DT_SEXY) {
		res.typ = DT_SEXY;
		res.sandwich = 0;
		res.dur = 0;
		res.neg = 0;
		res.sxepoch = d_unix;

	} else {
		res = dt_dt_initialiser();
	}
	return res;
}

/**
 * Return a dt object from a UTC'd DT that uses ZONE. */
DEFUN struct dt_dt_s
dtz_enrichz(struct dt_dt_s d, zif_t zone)
{
	struct dt_dt_s res;
	dt_ssexy_t d_unix;

	if (dt_sandwich_only_d_p(d) || dt_sandwich_only_t_p(d)) {
		return d;
	}

	/* convert date/time part to unix stamp */
	d_unix = ____to_unix_epoch(d);
	d_unix = zif_local_time(zone, d_unix);

	/* convert the date part back */
	if (d.typ > DT_DUNK && d.typ < DT_NDTYP) {
		int32_t sexy = __pos_mod(d_unix, 86400);

		res.d.typ = DT_DAISY;
		res.d.daisy = d_unix / 86400 + DAISY_UNIX_BASE;
		res.d = dt_dconv(d.d.typ, res.d);

		/* set the other flags too */
		res.sandwich = d.sandwich;
		res.dur = 0;
		res.neg = 0;

		/* convert the time part back */
		res.t.hms.s = sexy % 60;
		res.t.hms.m = (sexy % 3600) / 60;
		res.t.hms.h = sexy / 3600;
		res.t.hms.ns = d.t.hms.ns;

		res.t.typ = DT_HMS;
		res.t.dur = 0;
		res.t.neg = 0;
		res.t.carry = 0;

	} else if (d.typ == DT_SEXY) {
		res.typ = DT_SEXY;
		res.sandwich = 0;
		res.dur = 0;
		res.neg = 0;
		res.sxepoch = d_unix;

	} else {
		res = dt_dt_initialiser();
	}
	return res;
}

#endif	/* INCLUDED_dt_core_tz_glue_c_ */
/* dt-core-tz-glue.c ends here */
