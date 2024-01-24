/*** tzmap.h -- zonename maps
 *
 * Copyright (C) 2014-2024 Sebastian Freundt
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
#if !defined INCLUDED_tzmap_h_
#define INCLUDED_tzmap_h_

#include <stdint.h>

/*
** Each file begins with. . .
*/
#define	TZM_MAGIC	"TZm1"

typedef struct tzmap_s *tzmap_t;

typedef uint32_t znoff_t;

#define NUL_ZNOFF	((uint32_t)-1)

/** disk representation of tzm files */
struct tzmap_s {
	/* magic cookie, should be TZM_MAGIC */
	const char magic[4U];
	/* offset of mapped names, relative to data */
	znoff_t off;
	/* just to round to 16 bytes boundary */
	znoff_t flags[2U];
	/* \nul term'd list of zonenames followed by mapped names */
	const char data[];
};


/* public API */
extern tzmap_t tzm_open(const char *file);

extern void tzm_close(tzmap_t);

extern const char *tzm_find(tzmap_t m, const char *mname);

#endif	/* INCLUDED_tzmap_h_ */
