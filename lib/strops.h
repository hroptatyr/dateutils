/*** strops.h -- useful string operations
 *
 * Copyright (C) 2011-2014 Sebastian Freundt
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
#if !defined INCLUDED_strops_h_
#define INCLUDED_strops_h_

#include <stdint.h>
#include <stddef.h>

#if defined __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* stolen from Klaus Klein/David Laight's strptime() */
/**
 * Convert STR to i32 and point to the end of the string in EP. */
extern int32_t
strtoi_lim(const char *str, const char **ep, int32_t llim, int32_t ulim);

/**
 * Convert STR to i32 and point to the end of the string in EP. */
extern int32_t
strtoi(const char *str, const char **ep);

/**
 * Convert D (padded with at most PAD zeroes) into BUF and return the size. */
extern size_t
ui32tostr(char *restrict buf, size_t bsz, uint32_t d, int pad);

/**
 * Convert D (padded with at most WIDTH PAD chars) into B, return the size. */
extern size_t
ui32topstr(char *restrict b, size_t z, uint32_t d, int width, char pad);

/**
 * Convert roman numeral (string) to i32 and point to its end in EP. */
extern int32_t
romstrtoi_lim(const char *str, const char **ep, int32_t llim, int32_t ulim);

/**
 * Convert D to a roman numeral string and put it into BUF, return the size. */
extern size_t
ui32tostrrom(char *restrict buf, size_t bsz, uint32_t d);

/**
 * Find and skip ordinal suffixes in SUF, point to the end of the suffix. */
extern int __ordinalp(const char *num, size_t off_suf, char **ep);

/**
 * Append ordinal suffix to the most recently printed number in BUF,
 * eating away a leading 0. */
extern size_t __ordtostr(char *buf, size_t bsz);

/**
 * Take a string S, (case-insensitively) compare it to an array of strings ARR
 * of size NARR and return its index if found or -1 if not.
 * If S could be found in the array, point to the end of the string S in EP. */
extern int32_t
strtoarri(const char *s, const char **ep, const char *const *arr, size_t narr);

/**
 * Take a string array ARR (of size NARR) and an index I into the array, print
 * the string ARR[I] into BUF and return the number of bytes copied. */
extern size_t
arritostr(
	char *restrict buf, size_t bsz, size_t i,
	const char *const *arr, size_t narr);

/**
 * Faster strspn(). */
extern size_t
xstrspn(const char *src, const char *set);

/**
 * Faster strcspn(). */
extern size_t
xstrcspn(const char *src, const char *set);

/**
 * Faster strpbrk(). */
extern char*
xstrpbrk(const char *src, const char *set);

/**
 * Like xstrpbrk() but also return the offset to the character in set
 * that caused the match. */
extern char*
xstrpbrkp(const char *src, const char *set, size_t *set_offs);


static inline char*
__c2p(const char *p)
{
	union {
		char *p;
		const char *c;
	} res = {.c = p};
	return res.p;
}

#if defined __cplusplus
}
#endif	/* __cplusplus */

#endif	/* INCLUDED_strops_h_ */
