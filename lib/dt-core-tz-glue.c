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

#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
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


/**
 * Return a dt object that forgot about DT's zone and uses ZONE instead. */
DEFUN struct dt_dt_s
dtz_forgetz(struct dt_dt_s d, zif_t zone)
{
	dt_ssexy_t d_unix;
	dt_ssexy_t d_locl;
	int32_t zdiff;

	if (dt_sandwich_only_d_p(d) || dt_sandwich_only_t_p(d)) {
		return d;
	} else if (d.znfxd) {
		/* already forgotten about */
		return d;
	}

	/* convert date/time part to unix stamp */
	d_locl = ____to_unix_epoch(d);
	d_unix = zif_utc_time(zone, d_locl);
	if (LIKELY((zdiff = d_unix - d_locl))) {
		/* let dt_dtadd() do the magic */
		struct dt_dt_s zd = dt_dt_initialiser();

		dt_make_t_only(&zd, DT_HMS);
		zd.t.dur = 1;
		zd.t.sdur = zdiff;
		d = dt_dtadd(d, zd);
		d.znfxd = 1;
		if (zdiff > 0) {
			d.zdiff = (uint16_t)(zdiff / ZDIFF_RES);
		} else if (zdiff < 0) {
			d.neg = 1;
			d.zdiff = (uint16_t)(-zdiff / ZDIFF_RES);
		}
	}
	return d;
}

/**
 * Return a dt object from a UTC'd DT that uses ZONE. */
DEFUN struct dt_dt_s
dtz_enrichz(struct dt_dt_s d, zif_t zone)
{
	dt_ssexy_t d_unix;
	dt_ssexy_t d_locl;
	int32_t zdiff;

	if (dt_sandwich_only_d_p(d) || dt_sandwich_only_t_p(d)) {
		return d;
	}

	/* convert date/time part to unix stamp */
	d_unix = ____to_unix_epoch(d);
	d_locl = zif_local_time(zone, d_unix);
	if (LIKELY((zdiff = d_locl - d_unix))) {
		/* let dt_dtadd() do the magic */
		struct dt_dt_s zd = dt_dt_initialiser();

		dt_make_t_only(&zd, DT_HMS);
		zd.t.dur = 1;
		zd.t.sdur = zdiff;
		d = dt_dtadd(d, zd);
		if (zdiff > 0) {
			d.zdiff = (uint16_t)(zdiff / ZDIFF_RES);
		} else if (zdiff < 0) {
			d.neg = 1;
			d.zdiff = (uint16_t)(-zdiff / ZDIFF_RES);
		}
	}
	return d;
}

#endif	/* INCLUDED_dt_core_tz_glue_c_ */
/* dt-core-tz-glue.c ends here */
