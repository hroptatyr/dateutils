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

/**
 * Return a dt object that forgot about DT's zone and uses ZONE instead. */
DEFUN struct dt_dt_s
dtz_forgetz(struct dt_dt_s dt, zif_t zone)
{
	dt_daisy_t d = dt_conv_to_daisy(dt.d);
	struct dt_dt_s res = dt_dt_initialiser();
	uint32_t d_unix = (d - DAISY_UNIX_BASE) * 86400 +
		(dt.t.hms.h * 60 + dt.t.hms.m) * 60 + dt.t.hms.s;
	uint32_t d_utc = zif_utc_time(zone, d_unix);

	return dt;
}

#endif	/* INCLUDED_dt_core_tz_glue_c_ */
/* dt-core-tz-glue.c ends here */
