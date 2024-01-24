/*** alist.h -- bog standard associative lists
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
#if !defined INCLUDED_alist_h_
#define INCLUDED_alist_h_

typedef struct alist_s *alist_t;
typedef struct acons_s acons_t;

struct alist_s {
	char *data;
	size_t allz;

	/* private slots */
	size_t dend;
	const void *iter;
};

struct acons_s {
	const char *key;
	void *val;
};


extern void free_alist(alist_t);

/**
 * Return value associated with KEY in alist, or NULL if none found. */
extern void *alist_assoc(alist_t, const char *key);

/**
 * Put VAL as value associated with KEY in alist, but don't do a check
 * for duplicates first, i.e. it is assumed that KEY is not present. */
extern void alist_put(alist_t, const char *key, void *val);

/**
 * Set value associated with KEY in alist to VAL. */
extern void alist_set(alist_t, const char *key, void *val);

/**
 * Return next alist cons cell. */
extern acons_t alist_next(alist_t);

#endif	/* INCLUDED_alist_h_ */
