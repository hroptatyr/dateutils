/*** prchunk.c -- guessing line oriented data formats
 *
 * Copyright (C) 2010-2022 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of uterus.
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
#define PRCHUNK_C
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#if defined MAP_ANON_NEEDS_DARWIN_SOURCE
# define _DARWIN_C_SOURCE
#endif	/* MAP_ANON_NEEDS_DARWIN_SOURCE */
#if defined MAP_ANON_NEEDS_ALL_SOURCE
# define _ALL_SOURCE
#endif	/* MAP_ANON_NEEDS_ALL_SOURCE */
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>

#include "nifty.h"
#include "prchunk.h"

#define MAX_NLINES	(16384)
#define MAX_LLEN	(1024)

#if !defined MAP_ANONYMOUS && defined MAP_ANON
# define MAP_ANONYMOUS	(MAP_ANON)
#elif !defined MAP_ANON
# define MAP_ANON	(0x1000U)
#endif	/* MAP_ANON->MAP_ANONYMOUS */

#if defined __INTEL_COMPILER
# pragma warning(disable: 981)
#endif	/* __INTEL_COMPILER */

typedef uint32_t off32_t;
typedef uint16_t off16_t;

struct prch_ctx_s {
	/* file descriptor */
	int fd;
	/* buffer */
	char *buf;
	/* number of lines in the buffer */
	uint32_t tot_lno;
	/* number of columns per line */
	uint32_t tot_cno;
	/* number of bytes in the buffer */
	size_t bno;
	/* last known offset */
	size_t off;
	/* offsets */
	off32_t loff[MAX_NLINES];
	off32_t cur_lno;
	/* delimiter offsets */
	off16_t *soff;
};


static inline void
set_loff(prch_ctx_t ctx, uint32_t lno, off32_t off)
{
	ctx->loff[lno] = off;
	ctx->loff[lno] <<= 1;
	return;
}

static inline off32_t
get_loff(prch_ctx_t ctx, uint32_t lno)
{
	off32_t res = ctx->loff[lno];
	return res >> 1;
}

/* return 0 if not \r terminated, 1 otherwise */
static inline int
lftermdp(prch_ctx_t ctx, uint32_t lno)
{
	return ctx->loff[lno] & 1;
}

static inline void
set_lftermd(prch_ctx_t ctx, uint32_t lno)
{
	ctx->loff[lno] |= 1;
	return;
}

static inline size_t
get_llen(prch_ctx_t ctx, uint32_t lno)
{
	if (UNLIKELY(lno == 0)) {
		return get_loff(ctx, 0) - lftermdp(ctx, 0);
	}
	return get_loff(ctx, lno) -
		lftermdp(ctx, lno) -
		get_loff(ctx, lno - 1) - 1;
}


/* internal operations */
FDEFU int
prchunk_fill(prch_ctx_t ctx)
{
/* this is a coroutine consisting of a line counter yielding the number of
 * lines read so far and a reader yielding a buffer fill and the number of
 * bytes read */
#define CHUNK_SIZE	(4096)
#define YIELD(x)	goto yield##x
	char *off = ctx->buf + 0;
	char *bno = ctx->buf + ctx->bno;
	ssize_t nrd;

	/* initial work, reset the line counters et al */
	ctx->tot_lno = 0;
	/* we just memcpy() the left over stuff to the front and restart
	 * from there, someone left us a note in __ctx with the left
	 * over offset */
	/* normally we'd use memmove() but we know there's little chance
	 * for overlapping regions */
	if (UNLIKELY(ctx->bno == 0)) {
		/* do nothing */
		;
	} else if (LIKELY(ctx->bno > ctx->off)) {
		size_t rsz = ctx->bno - ctx->off;
		/* move the top RSZ bytes to the beginning */
		memcpy(ctx->buf, ctx->buf + ctx->off, rsz);
		ctx->bno = rsz;
		bno = ctx->buf + rsz;
	} else if (UNLIKELY(ctx->bno == ctx->off)) {
		/* what are the odds? just reset the counters */
		ctx->bno = 0;
		bno = ctx->buf;
	} else {
		/* the user didn't see the end of the file */
		return -1;
	}

yield1:
	/* read CHUNK_SIZE bytes */
	bno += (nrd = read(ctx->fd, bno, CHUNK_SIZE));
	/* if we came from yield2 then off == __ctx->bno, and if we
	 * read 0 or less bytes then off >= __ctx->bno + nrd, so we
	 * can simply use that compact expression if the buffer has no
	 * more input.
	 * On the contrary if we came from the outside, i.e. fill_buffer()
	 * has been called, then off would be 0 and __ctx->bno would be
	 * the buffer filled so far, if no more bytes could be read then
	 * we'd proceed processing them (off < __ctx->bno + nrd */
	if (UNLIKELY(!nrd && off < bno && ctx->cur_lno <= ctx->tot_lno)) {
		/* last line then, unyielded :| */
		set_loff(ctx, ctx->tot_lno, bno - ctx->buf);
		off = bno;
		/* count it as line and check if we need more */
		if (++ctx->tot_lno >= MAX_NLINES) {
			YIELD(3);
		}
		YIELD(4);
	} else if (UNLIKELY(nrd <= 0 && off == ctx->buf)) {
		/* special case, we worked our arses off and nothing's
		 * in the pipe line so just fuck off here */
		if (!ctx->bno) {
			return -1;
		}
		/* go to drain mode */
		YIELD(2);
	} else if (LIKELY(off < bno || off == ctx->buf)) {
		YIELD(2);
	}
	/* proceed to exit */
	YIELD(3);
yield2:
	while (off < bno) {
		size_t rsz = bno - off;
		char *p = memchr(off, '\n', rsz);
		if (UNLIKELY(p == NULL)) {
			if (LIKELY(nrd > 0)) {
				break;
			}
			/* not concluded with \n, let's hope we're in drain mode */
			return -1;
		}
		/* massage our status structures */
		set_loff(ctx, ctx->tot_lno, p - ctx->buf);
		if (UNLIKELY(p[-1] == '\r')) {
			/* oh god, when is this nightmare gonna end */
			p[-1] = '\0';
			set_lftermd(ctx, ctx->tot_lno);
		}
		*p = '\0';
		off = ++p;
		/* count it as line and check if we need more */
		if (++ctx->tot_lno >= MAX_NLINES) {
			YIELD(3);
		}
	}
	YIELD(1);
yield3:
	/* need clean up, something like unread(),
	 * in particular leave a note in __ctx with the left over offset */
	ctx->cur_lno = 0;
yield4:
	ctx->off = off - ctx->buf;
	ctx->bno = bno - ctx->buf;
#undef YIELD
#undef CHUNK_SIZE
	return 0;
}


/* public operations */
FDEFU prch_ctx_t
init_prchunk(int fd)
{
#define MAP_MEM		(MAP_ANON | MAP_PRIVATE)
#define PROT_MEM	(PROT_READ | PROT_WRITE)
#define MAP_LEN		(MAX_NLINES * MAX_LLEN)
	static struct prch_ctx_s __ctx;

	__ctx.buf = mmap(NULL, MAP_LEN, PROT_MEM, MAP_MEM, -1, 0);
	if (__ctx.buf == MAP_FAILED) {
		return NULL;
	}

	/* bit of space for the rechunker */
	__ctx.soff = mmap(NULL, MAP_LEN, PROT_MEM, MAP_MEM, -1, 0);
	if (__ctx.soff == MAP_FAILED) {
		return NULL;
	}

	if ((__ctx.fd = fd) > STDIN_FILENO) {
#if defined POSIX_FADV_SEQUENTIAL
		/* give advice about our read pattern */
		int rc = posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);

		if (UNLIKELY(rc < 0)) {
			munmap(__ctx.soff, MAP_LEN);
			return NULL;
		}
#endif	/* POSIX_FADV_SEQUENTIAL */
	}
	return &__ctx;
}

FDEFU void
free_prchunk(prch_ctx_t ctx)
{
	if (LIKELY(ctx->buf != NULL)) {
		munmap(ctx->buf, MAP_LEN);
		ctx->buf = NULL;
	}
	return;
}


/* accessors/iterators/et al. */
FDEFU size_t
prchunk_get_nlines(prch_ctx_t ctx)
{
	return ctx->tot_lno;
}

FDEFU size_t
prchunk_getlineno(prch_ctx_t ctx, char **p, int lno)
{
	if (UNLIKELY(lno <= 0)) {
		*p = ctx->buf;
		return get_llen(ctx, 0);
	} else if (UNLIKELY((size_t)lno >= prchunk_get_nlines(ctx))) {
		*p = NULL;
		return 0;
	}
	/* likely case last, what bollocks */
	*p = ctx->buf + get_loff(ctx, lno - 1) + 1;
	return get_llen(ctx, lno);
}

FDEFU size_t
prchunk_getline(prch_ctx_t ctx, char **p)
{
	return prchunk_getlineno(ctx, p, ctx->cur_lno++);
}

FDEFU void
prchunk_reset(prch_ctx_t ctx)
{
	ctx->cur_lno = 0;
	return;
}

FDEFU int
prchunk_haslinep(prch_ctx_t ctx)
{
/* the second condition is to allow unterminated last lines */
	return ctx->cur_lno < ctx->tot_lno || ctx->cur_lno == 0U;
}


static inline void
set_ncols(prch_ctx_t ctx, size_t ncols)
{
	ctx->tot_cno = ncols;
	return;
}

FDEFU size_t
prchunk_get_ncols(prch_ctx_t ctx)
{
	return ctx->tot_cno;
}

static inline void
set_col_off(prch_ctx_t ctx, size_t lno, size_t cno, size_t off)
{
	ctx->soff[lno * prchunk_get_ncols(ctx) + cno] = (off16_t)off;
	return;
}

static inline off16_t
get_col_off(prch_ctx_t ctx, size_t lno, size_t cno)
{
	return ctx->soff[lno * prchunk_get_ncols(ctx) + cno];
}

/* rechunker, chop the lines into smaller bits
 * Strategy is to go over all lines in the current chunk and
 * memchr() for the delimiter DELIM.
 * Store the offsets into __ctx->soff and bugger off leaving a \0
 * where the delimiter was. */
FDEFU void
prchunk_rechunk(prch_ctx_t ctx, char dlm, int ncols)
{
/* very naive implementation, we prefer prchunk_rechunk_by_dstfld()
 * where a distance histogram demarks possible places */
	size_t lno = 0;
	size_t cno = 0;
	char *line;
	char *off;
	char *p;
	char *bno = ctx->buf + ctx->off;
	size_t rsz;

	set_ncols(ctx, ncols);
	off = line = ctx->buf;
	rsz = bno - off;
	while ((p = memchr(off, dlm, rsz)) != NULL) {
		size_t co;
		size_t llen = get_llen(ctx, lno);
		while ((co = p - line) > llen) {
			/* last column offset equals the length of the line */
			set_col_off(ctx, lno, cno, llen);
			/* get the new line */
			line = ctx->buf + get_loff(ctx, lno++) + 1;
			cno = 0;
		}
		/* store the offset of the column within the line */
		set_col_off(ctx, lno, cno++, co);
		/* prepare the counters for the next round */
		*p = '\0';
		off = ++p;
		rsz = bno - off;
	}
	/* last column offset equals the length of the line */
	rsz = get_llen(ctx, lno);
	set_col_off(ctx, lno, cno, rsz);
	return;
}

FDEFU size_t
prchunk_getcolno(prch_ctx_t ctx, char **p, int lno, int cno)
{
	size_t co1, co2;

	if (UNLIKELY(cno < 0 || (size_t)cno >= prchunk_get_ncols(ctx))) {
		*p = NULL;
		return 0;
	}
	(void)prchunk_getlineno(ctx, p, lno);
	if (UNLIKELY(cno == 0)) {
		return get_col_off(ctx, lno, 0);
	}
	/* likely case last */
	co1 = get_col_off(ctx, lno, cno);
	co2 = get_col_off(ctx, lno, cno - 1);
	*p += co2 + 1;
	return co1 - co2 - 1;
}


#if defined STANDALONE
#include <stdio.h>

int
main(int argc, char *argv[])
{
	int fd;
	prch_ctx_t ctx;

	if (argc <= 1) {
		fd = STDIN_FILENO;
	} else if ((fd = open(argv[1], O_RDONLY)) < 0) {
		return 1;
	}
	/* get our prchunk up n running */
	if ((ctx = init_prchunk(fd)) == NULL) {
		error(errno, "Error: ctx NULL");
		return 1;
	}
	/* fill the buffer */
	while (!(prchunk_fill(ctx) < 0)) {
		char *l[1];
		size_t llen;
		int i = 0;

		while ((llen = prchunk_getline(ctx, l))) {
			fprintf(stderr, "%d (%zu) %s\n", i++, llen, l[0]);
		}
	}
	/* and out */
	free_prchunk(ctx);
	close(fd);
	return 0;
}
#endif	/* STANDALONE */

/* prchunk.c ends here */
