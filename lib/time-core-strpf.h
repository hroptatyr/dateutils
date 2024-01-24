/*** time-core-strpf.h -- parser and formatter funs for time-core
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

#if !defined INCLUDED_time_core_strpf_h_
#define INCLUDED_time_core_strpf_h_

#if defined __cplusplus
extern "C" {
#endif	/* __cplusplus */

struct strpt_s {
	signed int h;
	signed int m;
	signed int s;
	signed int ns;
	union {
		unsigned int u;
		struct {
			/* whether am/pm indicator was given */
			unsigned int am_pm_bit:1;
			/* 0 for am, 1 for pm */
			unsigned int pm_p:1;

			unsigned int h_set:1;
			unsigned int m_set:1;
			unsigned int s_set:1;
			unsigned int ns_set:1;
		};
	} flags;
};

extern struct dt_t_s __guess_ttyp(struct strpt_s t);


/* self-explanatory funs */
extern int
__strpt_card(struct strpt_s *d, const char *str, struct dt_spec_s s, char **ep);

extern size_t
__strft_card(
	char *buf, size_t bsz, struct dt_spec_s s,
	struct strpt_s *d, struct dt_t_s that);

#if defined __cplusplus
}
#endif	/* __cplusplus */

#endif	/* INCLUDED_time_core_strpf_h_ */
