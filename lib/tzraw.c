/*** tzraw.c -- reader for olson database zoneinfo files
 *
 * Copyright (C) 2009 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of uterus and dateutils.
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
/* implementation part of tzraw.h */
#if !defined INCLUDED_tzraw_c_
#define INCLUDED_tzraw_c_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>

/* me own header, innit */
#include "tzraw.h"

#if !defined LIKELY
# define LIKELY(_x)	__builtin_expect((_x), 1)
#endif	/* !LIKELY */
#if !defined UNLIKELY
# define UNLIKELY(_x)	__builtin_expect((_x), 0)
#endif	/* !UNLIKELY */

static int
__open_zif(const char *file)
{
	static const char tzdir[] = TZDIR;

	if (file == NULL || file[0] == '\0') {
		return -1;
	}

	if (file[0] != '/') {
		/* not an absolute file name */
		size_t len = strlen(file) + 1;
		size_t tzd_len = sizeof(tzdir) - 1;
		char *new, *tmp;

		new = alloca(tzd_len + 1 + len);
		tmp = mempcpy(new, tzdir, tzd_len);
		*tmp++ = '/';
		memcpy(tmp, file, len);
		file = new;
	}
	return open(file, O_RDONLY, 0644);
}

static zif_t
__read_zif(int fd)
{
	zif_t res;
	struct stat st;
	void *map;

	if (fstat(fd, &st) < 0) {
		return NULL;
	}
	res = malloc(sizeof(*res));
	res->mpsz = st.st_size;
	res->fd = fd;
	map = mmap(NULL, res->mpsz, PROT_READ, MAP_SHARED, fd, 0);
	if (map != MAP_FAILED) {
		res->hdr = map;
	}
	return res;
}

static void
__pars_zif(zif_t z)
{
	z->trs = (int32_t*)((char*)z->hdr + sizeof(*z->hdr));
	z->tys = (uint8_t*)(z->trs + zif_ntrans(z));
	z->tda = (ztrdtl_t)(z->tys + zif_ntrans(z));
	z->zn = (char*)(z->tda + zif_ntypes(z));
	return;
}

DEFUN inline zif_t
zif_read(const char *file)
{
	int fd;
	zif_t res = NULL;

	if ((fd = __open_zif(file)) > STDIN_FILENO &&
	    (res = __read_zif(fd)) != NULL) {
		__pars_zif(res);
	}
	return res;
}

DEFUN void
zif_close(zif_t z)
{
	if (UNLIKELY(z == NULL)) {
		/* nothing to do */
		return;
	}
	if (z->hdr != MAP_FAILED && (z + 1) != (void*)z->hdr) {
		munmap((void*)z->hdr, z->mpsz);
	}
	if (z->fd > STDIN_FILENO) {
		close(z->fd);
	}
	/* check if z is in mmap()'d space */
	if ((z + 1) != (void*)z->hdr) {
		/* nope? it's safe to free it then */
		free(z);
	} else {
		munmap(z, z->mpsz);
	}
	return;
}

DEFUN inline zif_t
zif_inst(zif_t z)
{
#define PROT_MEMMAP	PROT_READ | PROT_WRITE
#define MAP_MEMMAP	MAP_PRIVATE | MAP_ANONYMOUS
	void *map;
	size_t sz;
	zif_t res = NULL;

	if (UNLIKELY(z == NULL)) {
		/* no need to bother */
		return res;
	}
	sz = z->mpsz + sizeof(*z);

	map = mmap(NULL, sz, PROT_MEMMAP, MAP_MEMMAP, 0, 0);
	if (map != MAP_FAILED) {
		/* we mmap'ped ourselves a slightly larger struct
		 * res + 1 points to the header*/
		memcpy((zif_t)(res = map) + 1, z->hdr, z->mpsz);
		/* fill in the rest */
		res->mpsz = sz;
		res->hdr = (void*)(res + 1);
		__pars_zif(res);
		/* make sure we denote that this isnt connected to a file */
		res->fd = -1;
	}
	return res;
}

DEFUN zif_t
zif_read_inst(const char *name)
{
	zif_t tmpz = zif_read(name);
	zif_t res = zif_inst(tmpz);
	zif_close(tmpz);
	return res;
}


static inline int
__find_trno(zif_t z, int32_t t, int this, int min, int max)
{
	do {
		int32_t tl, tu;

		if (UNLIKELY(max == 0)) {
			/* special case */
			return 0;
		}

		tl = zif_trans(z, this);
		tu = zif_trans(z, this + 1);

		if (t >= tl && t < tu) {
			/* found him */
			return this;
		} else if (max - 1 <= min) {
			/* nearly found him */
			return this + 1;
		} else if (t >= tu) {
			min = this + 1;
			this = (this + max) / 2;
		} else if (t < tl) {
			max = this - 1;
			this = (this + min) / 2;
		}
	} while (true);
	/* not reached */
}

DEFUN inline int
zif_find_trans(zif_t z, int32_t t)
{
/* find the last transition before time, time is expected to be UTC */
	int max = zif_ntrans(z);
	int min = 0;
	int this = max / 2;

	return __find_trno(z, t, this, min, max);
}

static inline struct zrng_s
__find_zrng(zif_t z, int32_t t, int this, int min, int max)
{
	struct zrng_s res;
	size_t ntr = zif_ntrans(z);
	int trno = __find_trno(z, t, this, min, max);

	res.trno = (uint8_t)trno;
	res.prev = zif_trans(z, trno);
	/* special case for GMT+/- zones */
	res.next = trno + 1 < ntr ? zif_trans(z, trno + 1) : INT_MAX;
	res.offs = zif_troffs(z, trno);
	return res;
}

DEFUN inline struct zrng_s
zif_find_zrng(zif_t z, int32_t t)
{
/* find the last transition before time, time is expected to be UTC */
	int max = zif_ntrans(z);
	int min = 0;
	int this = max / 2;

	return __find_zrng(z, t, this, min, max);
}

static inline int32_t
__offs(zif_t z, int32_t t)
{
/* return the offset of T in Z and cache the result. */
	if (LIKELY(t >= z->cache.prev && t < z->cache.next)) {
		/* use the cached offset */
		;
	} else if (t >= z->cache.next) {
		int this = z->cache.trno + 1;
		int min = this;
		int max = zif_ntrans(z);
		z->cache = __find_zrng(z, t, this, min, max);
	} else if (t < z->cache.prev) {
		int max = z->cache.trno;
		int this = max - 1;
		int min = 0;
		z->cache = __find_zrng(z, t, this, min, max);
	}
	return z->cache.offs;
}

DEFUN int32_t
zif_utc_time(zif_t z, int32_t t)
{
/* here's the setup, given t in local time, we denote the corresponding
 * UTC time by t' = t - x' where x' is the true offset
 * however, since we do not know the offset in advance, we have to solve
 * for an estimate of the offset x:
 * t - x + x' = t, or equivalently t - x = t' or as a root finding problem
 * x' - x = 0.
 * To make this iterative we just solve:
 * x_{i+1} - x_i = 0, where x_{i+1} = o(t - x_i) and o maps a given
 * time stamp to an offset. */
/* make me use the cache please! */
	/* let's go */
	int32_t xi = 0, xj;

	/* jump off the cliff if Z is nought */
	if (UNLIKELY(z == NULL)) {
		return t;
	}

	while ((xj = __offs(z, t - xi)) != xi) {
		xi = xj;
	}
	return t - xj;
}

/* convert utc to local */
DEFUN int32_t
zif_local_time(zif_t z, int32_t t)
{
	/* jump off the cliff if Z is nought */
	if (UNLIKELY(z == NULL)) {
		return t;
	}
	return t + __offs(z, t);
}

#endif	/* INCLUDED_tzraw_c_ */
/* tzraw.c ends here */
