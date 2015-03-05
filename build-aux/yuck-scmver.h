/*** yuck-version.h -- snarf versions off project cwds
 *
 * Copyright (C) 2013-2015 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of yuck.
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
#if !defined INCLUDED_yuck_version_h_
#define INCLUDED_yuck_version_h_

#include "stdlib.h"

typedef const struct yuck_version_s *yuck_version_t;

typedef enum {
	YUCK_SCM_ERROR = -1,
	YUCK_SCM_TARBALL,
	YUCK_SCM_GIT,
	YUCK_SCM_BZR,
	YUCK_SCM_HG,
} yuck_scm_t;

struct yuck_version_s {
	yuck_scm_t scm;
	unsigned int dirty:1U;
	char vtag[16U];
	unsigned int dist;
	/* up to 28bits of revision id (hash for git),
	 * the lower 4bits denote the length */
	unsigned int rvsn;
};

extern const char *const yscm_strs[];


/* public api */
/**
 * Determine SCM version of file(s) in PATH. */
extern int yuck_version(struct yuck_version_s *restrict v, const char *path);

/**
 * Read a reference file FN and return scm version information. */
extern int yuck_version_read(struct yuck_version_s *restrict, const char *fn);

/**
 * Write scm version information in V to reference file FN. */
extern int yuck_version_write(const char *fn, const struct yuck_version_s *v);

/**
 * Write scm version into buffer. */
extern ssize_t yuck_version_write_fd(int fd, const struct yuck_version_s *v);

/**
 * Compare two version objects, return <0 if V1 < V2, >0 if V1 > V2 and
 * 0 if V1 and V2 are considered equal. */
extern int yuck_version_cmp(yuck_version_t v1, yuck_version_t v2);

#endif	/* INCLUDED_yuck_version_h_ */
