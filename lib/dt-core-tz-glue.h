/*** dt-core-tz-glue.h -- glue between tzraw and dt-core
 *
 * Copyright (C) 2012-2024 Sebastian Freundt
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
#if !defined INCLUDED_dt_core_tz_glue_h_
#define INCLUDED_dt_core_tz_glue_h_

/* include the guys we need gluing, innit */
#include "dt-core.h"
#include "tzraw.h"

#if defined __cplusplus
extern "C" {
#endif	/* __cplusplus */


/* decls */
/**
 * Return a dt object that forgot about DT's zone and uses ZONE instead.
 * In other words: convert from locally represented DT to UTC. */
extern struct dt_dt_s dtz_forgetz(struct dt_dt_s dt, zif_t zone);

/**
 * Return a dt object from a UTC'd DT that uses ZONE.
 * In other words: convert from UTC represented DT to local ZONE time. */
extern struct dt_dt_s dtz_enrichz(struct dt_dt_s dt, zif_t zone);

#if defined __cplusplus
}
#endif	/* __cplusplus */

#endif	/* INCLUDED_dt_core_tz_glue_h_ */
