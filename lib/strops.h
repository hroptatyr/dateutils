/*** strops.h -- useful string operations
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
#if !defined INCLUDED_strops_h_
#define INCLUDED_strops_h_

#include <stdint.h>
#include <stddef.h>

/* stolen from Klaus Klein/David Laight's strptime() */
/**
 * Convert STR to i32 and point to the end of the string in EP. */
extern __attribute__((nonnull(1, 2))) int32_t
strtoi_lim(const char *str, const char **ep, int32_t llim, int32_t ulim);

/**
 * Convert possibly padded STR to i32 and point to the end in EP. */
extern __attribute__((nonnull(1, 2))) int32_t
padstrtoi_lim(const char *str, const char **ep, int32_t llim, int32_t ulim);

/**
 * Convert STR to i32 and point to the end of the string in EP.
 * Return INT32_MIN on error. */
extern __attribute__((nonnull(1, 2))) int32_t
strtoi32(const char *str, const char **ep);

/**
 * Convert STR to i64 and point to the end of the string in EP.
 * Return INT64_MIN on error. */
extern __attribute__((nonnull(1, 2))) int64_t
strtoi64(const char *str, const char **ep);

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
 * If S could be found in the array, point to the end of the string S in EP.
 * The 0-th index will not be checked. */
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

/**
 * Like strpbrk() but consider a string of length LEN. */
extern char*
xmempbrk(const char *src, size_t len, const char *set);


static inline char
ui2c(uint32_t x, char pad)
{
	return (char)((pad > 0 || x > 0) << 5U | ((x > 0) << 4U) | (x ^ pad));
}

static inline size_t
ui99topstr(char *restrict b, size_t z, uint32_t d, size_t width, char pad)
{
/* specifically for numbers 00-99, signature like ui32topstr() */
	if (z) {
		uint32_t d10 = d / 10U, drem = d % 10U;
		size_t i;

		i = 0U;
		b[i] = ui2c(d10, pad);
		i += (d10 > 0U || width > 1U && pad) && z > 1U;
		b[i++] = ui2c(drem, '0');
		return i;
	}
	return 0U;
}

static inline size_t
ui999topstr(char *restrict b, size_t z, uint32_t d, size_t width, char pad)
{
/* specifically for numbers 000-999, signature like ui32topstr() */
	if (z) {
		uint32_t d100 = d / 100U, drem = d % 100U;
		size_t i;

		i = 0U;
		b[i] = ui2c(d100, pad);
		i += (d100 > 0U || width > 2U && pad) && z > 2U;
		d100 = drem / 10U, drem = drem % 10U;
		b[i] = ui2c(d100, pad);
		i += (d100 > 0U || width > 1U && pad) && z > 1U;
		b[i++] = ui2c(drem, '0');
		return i;
	}
	return 0U;
}

static inline size_t
ui9999topstr(char *restrict b, size_t z, uint32_t d, size_t width, char pad)
{
/* specifically for numbers 0000-9999, signature like ui32topstr() */
	if (z) {
		uint32_t d1000 = d / 1000U, drem = d % 1000U;
		size_t i;

		i = 0U;
		b[i] = ui2c(d1000, pad);
		i += (d1000 > 0U || width > 3U && pad) && z > 3U;
		d1000 = drem / 100U, drem = drem % 100U;
		b[i] = ui2c(d1000, pad);
		i += (d1000 > 0U || width > 2U && pad) && z > 2U;
		d1000 = drem / 10U, drem = drem % 10U;
		b[i] = ui2c(d1000, pad);
		i += (d1000 > 0U || width > 1U && pad) && z > 1U;
		b[i++] = ui2c(drem, '0');
		return i;
	}
	return 0U;
}

static inline size_t
ui999999999tostr(char *restrict b, size_t z, uint32_t d)
{
/* specifically for nanoseconds, this one fills the buffer from
 * most significant to least significant */
	if (z) {
		uint32_t dx = d / 100000000U, dr = d % 100000000U;
		size_t i = 0U;

		b[i++] = ui2c(dx, '0');
		if (!--z) {
			return i;
		}
		dx = dr / 10000000U, dr = dr % 10000000U;
		b[i++] = ui2c(dx, '0');
		if (!--z) {
			return i;
		}
		dx = dr / 1000000U, dr = dr % 1000000U;
		b[i++] = ui2c(dx, '0');
		if (!--z) {
			return i;
		}
		dx = dr / 100000U, dr = dr % 100000U;
		b[i++] = ui2c(dx, '0');
		if (!--z) {
			return i;
		}
		dx = dr / 10000U, dr = dr % 10000U;
		b[i++] = ui2c(dx, '0');
		if (!--z) {
			return i;
		}
		dx = dr / 1000U, dr = dr % 1000U;
		b[i++] = ui2c(dx, '0');
		if (!--z) {
			return i;
		}
		dx = dr / 100U, dr = dr % 100U;
		b[i++] = ui2c(dx, '0');
		if (!--z) {
			return i;
		}
		dx = dr / 10U, dr = dr % 10U;
		b[i++] = ui2c(dx, '0');
		if (!--z) {
			return i;
		}
		b[i++] = ui2c(dr, '0');
		return i;
	}
	return 0U;
}

#endif	/* INCLUDED_strops_h_ */
