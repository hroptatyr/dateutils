/*** date-core-private.h -- our universe of dates, private bits
 *
 * Copyright (C) 2011-2024 Sebastian Freundt
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
/* private portion of date-core.h */
#if !defined INCLUDED_date_core_private_h_
#define INCLUDED_date_core_private_h_

#include "date-core.h"


/* formatting defaults */
extern const char ymd_dflt[];
extern const char ymcw_dflt[];
extern const char ywd_dflt[];
extern const char yd_dflt[];
extern const char daisy_dflt[];
extern const char bizsi_dflt[];
extern const char bizda_dflt[];

extern const char ymddur_dflt[];
extern const char ymcwdur_dflt[];
extern const char ywddur_dflt[];
extern const char yddur_dflt[];
extern const char daisydur_dflt[];
extern const char bizsidur_dflt[];
extern const char bizdadur_dflt[];

extern dt_dtyp_t __trans_dfmt_special(const char*);
extern dt_dtyp_t __trans_dfmt(const char **fmt);
extern dt_durtyp_t __trans_ddurfmt(const char**fmt);

/**
 * Get the week count of D in the year when weeks start at _1st_wd. */
extern int __yd_get_wcnt(dt_yd_t d, dt_dow_t _1st_wd);

/**
 * Like __yd_get_wcnt() but for ISO week convention. */
extern int __yd_get_wcnt_iso(dt_yd_t d);

/**
 * Like __yd_get_wcnt() but disregard what day the year started with. */
extern int __yd_get_wcnt_abs(dt_yd_t d);

/**
 * Return the N-th W-day in the year of THAT.
 * This is equivalent with 8601's Y-W-D calendar where W is the week
 * of the year and D the day in the week */
extern unsigned int __ymcw_get_yday(dt_ymcw_t that);

/**
 * Get the number of days in month M of year Y. */
extern unsigned int __get_mdays(unsigned int y, unsigned int m);

/**
 * Get the number of business days in month M of year Y. */
extern unsigned int __get_bdays(unsigned int y, unsigned int m);

/**
 * Get the number of ISO weeks in year Y. */
extern unsigned int __get_isowk(unsigned int y);

/**
 * Compare two ymcw objects, return <0, 0, >0 when D1 < D2, D1 == D2, D1 > D2 */
extern int __ymcw_cmp(dt_ymcw_t d1, dt_ymcw_t d2);

/**
 * Get N where N is the N-th occurrence of wday in the month of that year */
extern unsigned int __ymd_get_count(dt_ymd_t that);

/**
 * Get the number of days in month M of year Y in the ummulqura calendar. */
extern unsigned int __get_mdays_hijri(unsigned int y, signed int m);

/**
 * Crop dates with days beyond ultimo. */
extern __attribute__((pure)) dt_ymd_t __ymd_fixup(dt_ymd_t);
extern __attribute__((pure)) dt_ywd_t __ywd_fixup(dt_ywd_t);
extern __attribute__((pure)) dt_yd_t __yd_fixup(dt_yd_t);
extern __attribute__((pure)) dt_ymcw_t __ymcw_fixup(dt_ymcw_t);
extern __attribute__((pure)) dt_bizda_t __bizda_fixup(dt_bizda_t);
extern __attribute__((pure)) dt_ummulqura_t __ummulqura_fixup(dt_ummulqura_t);

#endif	/* INCLUDED_date_core_private_h_ */
