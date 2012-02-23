/*** token.h -- tokeniser specs and stuff
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
#if !defined INCLUDED_token_h_
#define INCLUDED_token_h_

#if defined __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* spec tokeniser, spec flags plus modifiers and stuff */
typedef enum {
	DT_SPFL_UNK,

	/* date specs */
	DT_SPFL_N_MDAY,
	DT_SPFL_N_MON,
	DT_SPFL_N_YEAR,
	/* %F, but generally stands for calendar's standard format */
	DT_SPFL_N_DSTD,
	/* for 4-level calendars this counts the property within the week */
	DT_SPFL_N_CNT_WEEK,
	/* count of property within the month, %d could be mapped here */
	DT_SPFL_N_CNT_MON,
	/* count of property within the year */
	DT_SPFL_N_CNT_YEAR,
	DT_SPFL_N_QTR,
	DT_SPFL_N_LAST = DT_SPFL_N_QTR,

	DT_SPFL_S_WDAY,
	DT_SPFL_S_MON,
	DT_SPFL_S_QTR,
	DT_SPFL_S_DLAST = DT_SPFL_S_QTR,

	/* time specs */
	DT_SPFL_N_SEC,
	DT_SPFL_N_MIN,
	DT_SPFL_N_HOUR,
	/* %T, but generally stands for calendar's standard format */
	DT_SPFL_N_TSTD,
	/* for 4-level calendars this counts the property within the week */
	DT_SPFL_N_NANO,
	DT_SPFL_N_TLAST = DT_SPFL_N_NANO,

	DT_SPFL_S_AMPM,
	DT_SPFL_S_TLAST = DT_SPFL_S_AMPM,

	DT_SPFL_LIT_PERCENT,
	DT_SPFL_LIT_TAB,
	DT_SPFL_LIT_NL,
} dt_spfl_t;

#if defined __cplusplus
}
#endif	/* __cplusplus */

#endif	/* INCLUDED_token_h_ */
