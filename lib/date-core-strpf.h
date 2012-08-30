/*** date-core-strpf.h -- parser and formatter funs for date-core
 *
 * Copyright (C) 2011-2012 Sebastian Freundt
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

#if !defined INCLUDED_date_core_strpf_h_
#define INCLUDED_date_core_strpf_h_

#if defined __cplusplus
extern "C" {
#endif	/* __cplusplus */

#if !defined DECLF
# define DECLF	static __attribute__((unused))
# define DEFUN	static
# define INCLUDE_DATE_CORE_STRPF_IMPL
#elif !defined DEFUN
# define DEFUN
#endif	/* !DECLF */
#if !defined restrict
# define restrict	__restrict
#endif	/* !restrict */
#if !defined DECLV
# define DECLV		DECLF
#endif	/* !DECLV */
#if !defined DEFVAR
# define DEFVAR		DEFUN
#endif	/* !DEFVAR */

struct strpd_s {
	unsigned int y;
	unsigned int m;
	union {
		unsigned int d;
		signed int sd;
	};
	unsigned int c;
	unsigned int w;
	/* general flags */
	struct {
		unsigned int ab:1;
		unsigned int bizda:1;
		unsigned int d_dcnt_p:1;
		unsigned int wk_cnt:2;/*%C,%W,%U,%V*/
	} flags;
	unsigned int b;
	unsigned int q;
};

struct strpdi_s {
	signed int m;
	signed int d;
	signed int w;
	signed int b;
};


/* helpers */
static inline __attribute__((pure, const)) struct strpd_s
strpd_initialiser(void)
{
	struct strpd_s res = {0};
	return res;
}

static inline __attribute__((pure, const)) struct strpdi_s
strpdi_initialiser(void)
{
	struct strpdi_s res = {0};
	return res;
}

/* textual representations of parts of the date */
/**
 * Long weekday names, english only.
 * Monday, Tuesday, ... */
DECLV const char **dut_long_wday;
DECLV const size_t dut_nlong_wday;

/**
 * Abbrev'd weekday names, english only.
 * Mon, Tue, ... */
DECLV const char **dut_abbr_wday;
DECLV const size_t dut_nabbr_wday;

/**
 * Even-more-abbrev'd weekday names, english only.
 * M, T, W, ... */
DECLV const char *dut_abab_wday;
DECLV const size_t dut_nabab_wday;

/**
 * Long month names, english only.
 * January, February, ... */
DECLV const char **dut_long_mon;
DECLV const size_t dut_nlong_mon;

/**
 * Abbrev'd month names, english only.
 * Jan, Feb, ... */
DECLV const char **dut_abbr_mon;
DECLV const size_t dut_nabbr_mon;

/**
 * Even-more-abbrev'd month names.
 * F, G, H, ... */
DECLV const char *dut_abab_mon;
DECLV const size_t dut_nabab_mon;

#if defined INCLUDED_date_core_h_
/**
 * Populate TGT with duration information from DUR. */
DECLF inline void __fill_strpdi(struct strpdi_s *tgt, struct dt_d_s dur);

/**
 * Parse STR with the standard parser, put the end of the parsed string in EP.*/
DECLF struct dt_d_s __strpd_std(const char *str, char **ep);

/**
 * Given a strpd object D, try to construct a dt_d object.
 * Defined in date-core.c */
DECLF struct dt_d_s __guess_dtyp(struct strpd_s d);
#endif	/* INCLUDED_date_core_h_ */

/* self-explanatory funs, innit? */
DECLF int
__strpd_card(struct strpd_s *d, const char *sp, struct dt_spec_s s, char **ep);

DECLF int
__strpd_rom(struct strpd_s *d, const char *sp, struct dt_spec_s s, char **ep);

DECLF size_t
__strfd_card(
	char *buf, size_t bsz, struct dt_spec_s s,
	struct strpd_s *d, struct dt_d_s that);

DECLF size_t
__strfd_rom(
	char *buf, size_t bsz, struct dt_spec_s s,
	struct strpd_s *d, struct dt_d_s that);

DECLF size_t
__strfd_dur(
	char *buf, size_t bsz, struct dt_spec_s s,
	struct strpd_s *d, struct dt_d_s that);


#if defined INCLUDE_DATE_CORE_STRPF_IMPL
# include "date-core-strpf.c"
#endif	/* INCLUDE_DATE_CORE_STRPF_IMPL */

#if defined __cplusplus
}
#endif	/* __cplusplus */

#endif	/* INCLUDED_date_core_strpf_h_ */
