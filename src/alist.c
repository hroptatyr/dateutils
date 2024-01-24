/*** alist.c -- bog standard associative lists
 *
 * Copyright (C) 2010-2024 Sebastian Freundt
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
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "alist.h"
#include "nifty.h"

/* our alist is a simple flat char array with pointers behind every key
 * aligned to void* boundaries. */

static void**
__assoc(alist_t al, const char *key)
{
	for (const char *ap = al->data, *const ep = ap + al->dend; ap < ep;) {
		const char *kp;
		void **res = (void**)al->data;

		/* unrolled strcmp */
		for (kp = key; *ap && *ap == *kp; ap++, kp++);

		/* fast forward to void** boundary */
		res += (ap - al->data) / sizeof(res) + 1U;

		if (*ap == *kp) {
			/* match! invariant: ap == kp == '\0' */
			return res;
		} else if (*ap) {
			/* fast forward in void** increments */
			for (; ((const char*)res)[-1]; res++);
		}
		/* skip over ptr, start again */
		ap = (const char*)++res;
	}
	return NULL;
}

static bool
__fitsp(alist_t al, size_t keylen)
{
	return al->dend + keylen + 2 * sizeof(void**) < al->allz;
}

static int
__chk_resz(alist_t al, size_t keylen)
{
	if (UNLIKELY(!__fitsp(al, keylen))) {
		size_t ol = al->allz;
		void *tmp;

		al->allz = (al->allz * 2U) ?: 64U;
		if (UNLIKELY((tmp = realloc(al->data, al->allz)) == NULL)) {
			free_alist(al);
			return -1;
		}
		al->data = tmp;
		memset(al->data + ol, 0, al->allz - ol);
	}
	return 0;
}


/* public api */
void
free_alist(alist_t al)
{
	if (LIKELY(al->data != NULL)) {
		free(al->data);
		memset(al, 0, sizeof(*al));
	}
	return;
}

void*
alist_assoc(alist_t al, const char *key)
{
	void **res;

	if (UNLIKELY(al->data == NULL)) {
		goto nada;
	} else if ((res = __assoc(al, key)) == NULL) {
		goto nada;
	}
	return *res;
nada:
	return NULL;
}

void
alist_put(alist_t al, const char *key, void *val)
{
	size_t klen = strlen(key);

	__chk_resz(al, klen);
	memcpy(al->data + al->dend, key, klen);
	/* round up to void** boundary */
	with (const void **data = (const void**)al->data) {
		data += (al->dend + klen) / sizeof(data) + 1U;
		*data++ = val;
		al->dend = (const char*)data - al->data;
	}
	return;
}

void
alist_set(alist_t al, const char *key, void *val)
{
	void **ass;

	if ((ass = __assoc(al, key)) != NULL) {
		*ass = val;
		return;
	}
	alist_put(al, key, val);
	return;
}

acons_t
alist_next(alist_t al)
{
	acons_t res;

	if (UNLIKELY((const char*)al->iter >= al->data + al->dend)) {
		al->iter = NULL;
		res = (acons_t){NULL, NULL};
	} else {
		const char *p = al->iter ?: al->data;
		size_t klen = strlen(p);

		with (void *const *d = (const void*)p) {
			d += klen / sizeof(d) + 1U;
			res.key = p;
			res.val = *d++;
			al->iter = d;
		}
	}
	return res;
}

/* alist.c ends here */
