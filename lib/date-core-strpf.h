/*** date-core-strpf.h -- parser and formatter funs for date-core
 *
 * Copyright (C) 2011-2016 Sebastian Freundt
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

struct strpd_s {
	signed int y;
	signed int m;
	union {
		signed int d;
		signed int sd;
	};
	signed int c;
	signed int w;
	/* general flags */
	union {
		unsigned int u;
		struct {
			unsigned int ab:1;
			unsigned int bizda:1;
			unsigned int d_dcnt_p:1;
			unsigned int c_wcnt_p:1;
			unsigned int wk_cnt:2;/*%C,%W,%U,%V*/
			unsigned int real_y_in_q:1;
		};
	} flags;
	signed int b;
	signed int q;
};

struct strpdi_s {
	signed int y;
	signed int m;
	signed int d;
	signed int w;
	signed int b;
};


/* helpers */
static inline __attribute__((pure, const)) struct strpd_s
strpd_initialiser(void)
{
#if defined HAVE_SLOPPY_STRUCTS_INIT
	static const struct strpd_s res = {};
#else
	static const struct strpd_s res;
#endif	/* HAVE_SLOPPY_STRUCTS_INIT */
	return res;
}

static inline __attribute__((pure, const)) struct strpdi_s
strpdi_initialiser(void)
{
#if defined HAVE_SLOPPY_STRUCTS_INIT
	static const struct strpdi_s res = {};
#else
	static const struct strpdi_s res;
#endif	/* HAVE_SLOPPY_STRUCTS_INIT */
	return res;
}

/* textual representations of parts of the date */
/**
 * Long weekday names, english only.
 * Monday, Tuesday, ... */
extern const char **dut_long_wday;
extern const ssize_t dut_nlong_wday;

/**
 * Abbrev'd weekday names, english only.
 * Mon, Tue, ... */
extern const char **dut_abbr_wday;
extern const ssize_t dut_nabbr_wday;

/**
 * Even-more-abbrev'd weekday names, english only.
 * M, T, W, ... */
extern const char *dut_abab_wday;
extern const ssize_t dut_nabab_wday;

/**
 * Long month names, english only.
 * January, February, ... */
extern const char **dut_long_mon;
extern const ssize_t dut_nlong_mon;

/**
 * Abbrev'd month names, english only.
 * Jan, Feb, ... */
extern const char **dut_abbr_mon;
extern const ssize_t dut_nabbr_mon;

/**
 * Even-more-abbrev'd month names.
 * F, G, H, ... */
extern const char *dut_abab_mon;
extern const ssize_t dut_nabab_mon;

#if defined INCLUDED_date_core_h_
/**
 * Populate TGT with duration information from DUR. */
extern void __fill_strpdi(struct strpdi_s *tgt, struct dt_d_s dur);

/**
 * Parse STR with the standard parser, put the end of the parsed string in EP.*/
extern struct dt_d_s __strpd_std(const char *str, char **ep);

/**
 * Given a strpd object D, try to construct a dt_d object.
 * Defined in date-core.c */
extern struct dt_d_s __guess_dtyp(struct strpd_s d);
#endif	/* INCLUDED_date_core_h_ */

/* self-explanatory funs, innit? */
extern int
__strpd_card(struct strpd_s *d, const char *sp, struct dt_spec_s s, char **ep);

extern int
__strpd_rom(struct strpd_s *d, const char *sp, struct dt_spec_s s, char **ep);

extern size_t
__strfd_card(
	char *buf, size_t bsz, struct dt_spec_s s,
	struct strpd_s *d, struct dt_d_s that);

extern size_t
__strfd_rom(
	char *buf, size_t bsz, struct dt_spec_s s,
	struct strpd_s *d, struct dt_d_s that);

extern size_t
__strfd_dur(
	char *buf, size_t bsz, struct dt_spec_s s,
	struct strpd_s *d, struct dt_ddur_s that);

/* specific formatters and parsers */
extern void __prep_strfd_ywd(struct strpd_s *tgt, dt_ywd_t d);
extern void __prep_strfd_daisy(struct strpd_s *tgt, dt_daisy_t d);
extern void
__prep_strfd_bizda(struct strpd_s *tgt, dt_bizda_t d, dt_bizda_param_t bp);

#if defined __cplusplus
}
#endif	/* __cplusplus */

#endif	/* INCLUDED_date_core_strpf_h_ */
