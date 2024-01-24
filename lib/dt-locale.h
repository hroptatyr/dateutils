/*** locale.h -- locale light
 *
 * Copyright (C) 2015-2024 Sebastian Freundt
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
#if !defined INCLUDED_dt_locale_h_
#define INCLUDED_dt_locale_h_

/* textual representations of parts of the date */
/**
 * Long weekday names, english only.
 * Monday, Tuesday, ...
 * The duf_* version is for the formatters. */
extern const char **dut_long_wday;
extern const char **duf_long_wday;
extern const ssize_t dut_nlong_wday;
extern struct strprng_s dut_rlong_wday;

/**
 * Abbrev'd weekday names, english only.
 * Mon, Tue, ...
 * The duf_* version is for the formatters */
extern const char **dut_abbr_wday;
extern const char **duf_abbr_wday;
extern const ssize_t dut_nabbr_wday;
extern struct strprng_s dut_rabbr_wday;

/**
 * Even-more-abbrev'd weekday names, english only.
 * M, T, W, ...
 * There is no distinction between input and output formats. */
extern const char *dut_abab_wday;
extern const ssize_t dut_nabab_wday;

/**
 * Long month names, english only.
 * January, February, ...
 * The duf_* version is for the formatters. */
extern const char **dut_long_mon;
extern const char **duf_long_mon;
extern const ssize_t dut_nlong_mon;
extern struct strprng_s dut_rlong_mon;

/**
 * Abbrev'd month names, english only.
 * Jan, Feb, ...
 * The duf_* version is for the formatters. */
extern const char **dut_abbr_mon;
extern const char **duf_abbr_mon;
extern const ssize_t dut_nabbr_mon;
extern struct strprng_s dut_rabbr_mon;

/**
 * Even-more-abbrev'd month names.
 * F, G, H, ...
 * There is no distinction between input and output formats. */
extern const char *dut_abab_mon;
extern const ssize_t dut_nabab_mon;


/* public API */
/**
 * Set input locale (only LC_TIME values) to LOCALE.
 * Just as stupid as setlocale(3). */
extern int setilocale(const char *locale);

/**
 * Set formatting locale (only LC_TIME values) to LOCALE.
 * Just as stupid as setlocale(3). */
extern int setflocale(const char *locale);

#endif	/* INCLUDED_dt_locale_h_ */
