/*** boobs.h -- byte-order oberations
 *
 * Copyright (C) 2012 Sebastian Freundt
 * Copyright (C) 2012 Ruediger Meier
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of uterus, dateutils and atem.
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

#if !defined INCLUDED_boobs_h_
#define INCLUDED_boobs_h_

#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
/* *bsd except for openbsd */
#if defined HAVE_SYS_ENDIAN_H
# include <sys/endian.h>
#elif defined HAVE_MACHINE_ENDIAN_H
# include <machine/endian.h>
#elif defined HAVE_ENDIAN_H
# include <endian.h>
#elif defined HAVE_BYTEORDER_H
# include <byteorder.h>
#endif	/* SYS/ENDIAN_H || MACHINE/ENDIAN_H || ENDIAN_H || BYTEORDER_H */

/* check for byteswap to do the swapping ourselves if need be */
#if defined HAVE_BYTESWAP_H
# include <byteswap.h>
#endif	/* BYTESWAP_H */

/* start off with opposite-endianness converters */
#if defined htooe16
/* yay, nothing to do really */
#elif defined __GNUC__ && __GNUC__ == 4 && __GNUC_MINOR__ >= 7
# define htooe16(x)	__builtin_bswap16(x)
#elif defined __bswap_16
# define htooe16(x)	__bswap_16(x)
#elif defined __swap16
# define htooe16(x)	__swap16(x)
#elif defined WORDS_BIGENDIAN && defined le16toh
# define htooe16(x)	le16toh(x)
#elif !defined WORDS_BIGENDIAN && defined be16toh
# define htooe16(x)	be16toh(x)
#else
# warning htooe16() will not convert anything
# define htooe16(x)	(x)
#endif	/* htooe16 */

#if !defined be16toh
# if defined betoh16
#  define be16toh	betoh16
# elif defined WORDS_BIGENDIAN
#  define be16toh(x)	(x)
# else	/* means we need swapping */
#  define be16toh(x)	htooe16(x)
# endif	 /* betoh16 */
#endif	/* !be16toh */

#if !defined le16toh
# if defined letoh16
#  define le16toh	letoh16
# elif defined WORDS_BIGENDIAN
#  define le16toh(x)	htooe16(x)
# else	/* no swapping needed */
#  define le16toh(x)	(x)
# endif	 /* letoh16 */
#endif	/* !le16toh */

#if !defined htobe16
# if defined WORDS_BIGENDIAN
#  define htobe16(x)	(x)
# else	/* need swabbing */
#  define htobe16(x)	htooe16(x)
# endif
#endif	/* !htobe16 */

#if !defined htole16
# if defined WORDS_BIGENDIAN
#  define htole16(x)	htooe16(x)
# else	/* no byte swapping needed */
#  define htole16(x)	(x)
# endif
#endif	/* !htole16 */


/* just to abstract over pure swapping */
#if defined htooe32
/* yay, nothing to do really */
#elif defined __GNUC__ && __GNUC__ == 4 && __GNUC_MINOR__ >= 7
# define htooe32(x)	__builtin_bswap32(x)
#elif defined __bswap_32
# define htooe32(x)	__bswap_32(x)
#elif defined __swap32
# define htooe32(x)	__swap32(x)
#elif defined WORDS_BIGENDIAN && defined le32toh
# define htooe32(x)	le32toh(x)
#elif !defined WORDS_BIGENDIAN && defined be32toh
# define htooe32(x)	be32toh(x)
#else
# warning htooe32() will not convert anything
# define htooe32(x)	(x)
#endif

/* and even now we may be out of luck */
#if !defined be32toh
# if defined betoh32
#  define be32toh	betoh32
# elif defined WORDS_BIGENDIAN
#  define be32toh(x)	(x)
# else	/* need some swaps */
#  define be32toh(x)	htooe32(x)
# endif
#endif	/* !be32toh */

#if !defined le32toh
# if defined letoh32
#  define le32toh	letoh32
# elif defined WORDS_BIGENDIAN
#  define le32toh(x)	htooe32(x)
# else	/* no byte swapping here */
#  define le32toh(x)	(x)
# endif	 /* letoh32 */
#endif	/* !le32toh */

#if !defined htobe32
# if defined WORDS_BIGENDIAN
#  define htobe32(x)	(x)
# else	/* yep, swap me about */
#  define htobe32(x)	htooe32(x)
# endif
#endif	/* !be32toh */

#if !defined htole32
# if defined WORDS_BIGENDIAN
#  define htole32(x)	htooe32(x)
# else	/* nothing to swap */
#  define htole32(x)	(x)
# endif
#endif	/* !htole32 */


#if defined htooe64
/* yay, nothing to do really */
#elif defined __GNUC__ && __GNUC__ == 4 && __GNUC_MINOR__ >= 7
# define htooe64(x)	__builtin_bswap64(x)
#elif defined __bswap_64
# define htooe64(x)	__bswap_64(x)
#elif defined __swap64
# define htooe64(x)	__swap64(x)
#elif defined WORDS_BIGENDIAN && defined le64toh
# define htooe64(x)	le64toh(x)
#elif !defined WORDS_BIGENDIAN && defined be64toh
# define htooe64(x)	be64toh(x)
#else
# warning htooe64() will not convert anything
# define htooe64(x)	(x)
#endif

#if !defined be64toh
# if defined betoh64
#  define be64toh	betoh64
# elif defined WORDS_BIGENDIAN
#  define be64toh(x)	(x)
# else	/* swapping */
#  define be64toh(x)	htooe64(x)
# endif
#endif	/* !be64toh */

#if !defined le64toh
# if defined letoh64
#  define le64toh	letoh64
# elif defined WORDS_BIGENDIAN
#  define le64toh(x)	htooe64(x)
# else	/* nothing to swap */
#  define le64toh(x)	(x)
# endif
#endif	/* !le64toh */

#if !defined htobe64
# if defined WORDS_BIGENDIAN
#  define htobe64(x)	(x)
# else
#  define htobe64(x)	htooe64(x)
# endif
#endif	/* !htobe64 */

#if !defined htole64
# if defined WORDS_BIGENDIAN
#  define htole64(x)	htooe64(x)
# else	/* no need swapping */
#  define htole64(x)	(x)
# endif
#endif	/* !htole64 */

/* we could technically include byteswap.h and to the swap ourselves
 * in the missing cases.  Instead we'll just leave it as is and wait
 * for bug reports. */

#endif	/* INCLUDED_boobs_h_ */
