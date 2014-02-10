/*** dt-io.h -- helper for formats, parsing, printing, escaping, etc. */
#if !defined INCLUDED_dt_io_h_
#define INCLUDED_dt_io_h_

#include <stdlib.h>
#include <stdio.h>
#if defined __GLIBC__
/* for *_unlocked protos */
# include <stdio_ext.h>
#endif	/* __GLIBC__ */
/* for strstr() */
#include <string.h>
/* for strcasecmp() */
#include <strings.h>
#include "dt-core.h"
#include "dt-io-zone.h"
#include "nifty.h"

typedef enum {
	STRPDT_UNK,
	STRPDT_DATE,
	STRPDT_TIME,
	STRPDT_NOW,
	STRPDT_YDAY,
	STRPDT_TOMO,
} dt_strpdt_special_t;

/* needles for the grep mode */
typedef struct grep_atom_s *grep_atom_t;
typedef const struct gep_atom_s *const_grep_atom_t;

struct grpatm_payload_s {
	uint8_t flags;
#define GRPATM_DIGITS	(1)
#define GRPATM_ORDINALS	(2)
#define GRPATM_SUFFIX	(4)
#define GRPATM_A_SPEC	(8)
#define GRPATM_B_SPEC	(16)
#define GRPATM_O_SPEC	(32)
#define GRPATM_T_FLAG	(64)
#define GRPATM_P_SPEC	(128)
	int8_t off_min;
	int8_t off_max;
	const char *fmt;
};
/* combos */
#define GRPATM_TINY_A_SPEC	(GRPATM_T_FLAG | GRPATM_A_SPEC)
#define GRPATM_TINY_B_SPEC	(GRPATM_T_FLAG | GRPATM_B_SPEC)

/* atoms are maps needle-character -> payload */
struct grep_atom_s {
	/* 4 bytes it should be */
	char needle;
	struct grpatm_payload_s pl;
};

struct grep_atom_soa_s {
	size_t natoms;
	char *needle;
	struct grpatm_payload_s *flesh;
};

/* duration parser */
/* we parse durations ourselves so we can cope with the
 * non-commutativity of duration addition:
 * 2000-03-30 +1m -> 2000-04-30 +1d -> 2000-05-01
 * 2000-03-30 +1d -> 2000-03-31 +1m -> 2000-04-30 */
struct __strpdtdur_st_s {
	int sign;
	const char *istr;
	const char *cont;
	size_t ndurs;
	struct dt_dt_s *durs;
};


/* public API */
extern dt_strpdt_special_t dt_io_strpdt_special(const char *str);
extern struct dt_dt_s
dt_io_strpdt_ep(
	const char *str,
	const char *const *fmt, size_t nfmt, char **ep,
	zif_t zone);

extern struct dt_dt_s
dt_io_find_strpdt(
	const char *str, char *const *fmt, size_t nfmt,
	const char *needle, size_t needlen, char **sp, char **ep,
	zif_t zone);

extern struct dt_dt_s
dt_io_find_strpdt2(
	const char *str,
	const struct grep_atom_soa_s *needles,
	char **sp, char **ep,
	zif_t zone);

extern int
dt_io_write(struct dt_dt_s d, const char *fmt, zif_t zone, int apnd_ch);

/* grep atoms */
extern struct grep_atom_s calc_grep_atom(const char *fmt);

extern struct grep_atom_soa_s
build_needle(grep_atom_t atoms, size_t natoms, char *const *fmt, size_t nfmt);

extern void dt_io_unescape(char *s);

/* error messages, warnings, etc. */
extern __attribute__((format(printf, 2, 3))) void
error(int eno, const char *fmt, ...);

/* duration parser */
extern int __add_dur(struct __strpdtdur_st_s *st, struct dt_dt_s dur);
extern int dt_io_strpdtdur(struct __strpdtdur_st_s *st, const char *str);

/* zone handling, tzmaps et al. */
extern zif_t dt_io_zone(const char *spec);


/* inlines */
static inline struct dt_dt_s
dt_io_strpdt(const char *input, char *const *fmt, size_t nfmt, zif_t zone)
{
	return dt_io_strpdt_ep(input, (const char*const*)fmt, nfmt, NULL, zone);
}

/* grep atoms */
static inline __attribute__((pure, const)) struct grep_atom_s
__grep_atom_initialiser(void)
{
#if defined HAVE_SLOPPY_STRUCTS_INIT
	static const struct grep_atom_s res = {0};
#else
	static const struct grep_atom_s res;
#endif	/* HAVE_SLOPPY_STRUCTS_INIT */
	return res;
}

static inline __attribute__((pure, const)) struct grep_atom_soa_s
__grep_atom_soa_initialiser(void)
{
#if defined HAVE_SLOPPY_STRUCTS_INIT
	static const struct grep_atom_soa_s res = {0};
#else
	static const struct grep_atom_soa_s res;
#endif	/* HAVE_SLOPPY_STRUCTS_INIT */
	return res;
}

static inline struct grep_atom_soa_s
make_grep_atom_soa(grep_atom_t atoms, size_t natoms)
{
	struct grep_atom_soa_s res = __grep_atom_soa_initialiser();

	res.needle = (char*)atoms;
	res.flesh = (void*)(res.needle + natoms);
	return res;
}

#define GRPATM_NEEDLELESS_MODE_CHAR	(1)

/* formatter */
static inline size_t
dt_io_strfdt(
	char *restrict buf, size_t bsz,
	const char *fmt, struct dt_dt_s that, int apnd_ch)
{
	size_t res = dt_strfdt(buf, bsz, fmt, that);

	if (LIKELY(res > 0) && apnd_ch && buf[res - 1] != apnd_ch) {
		/* auto-newline */
		buf[res++] = (char)apnd_ch;
	}
	return res;
}


static __attribute__((unused)) size_t
__io_write(const char *line, size_t llen, FILE *where)
{
#if defined __GLIBC__
	return fwrite_unlocked(line, sizeof(*line), llen, where);
#else  /* !__GLIBC__ */
	return fwrite(line, sizeof(*line), llen, where);
#endif	/* __GLIBC__ */
}

static __attribute__((unused)) int
__io_putc(int c, FILE *where)
{
#if defined __GLIBC__
	return fputc_unlocked(c, where);
#else  /* !__GLIBC__ */
	return fputc(c, where);
#endif	/* __GLIBC__ */
}

static inline __attribute__((unused)) void
#if defined __GLIBC__
__io_setlocking_bycaller(FILE *fp)
{
	__fsetlocking(fp, FSETLOCKING_BYCALLER);
	return;
}
#else  /* !__GLIBC__ */
__io_setlocking_bycaller(__attribute__((unused)) FILE *fp)
{
	return;
}
#endif	/* __GLIBC__ */

static inline __attribute__((unused)) int
__io_eof_p(FILE *fp)
{
#if defined __GLIBC__
	return feof_unlocked(fp);
#else  /* !__GLIBC__ */
	return feof(fp);
#endif	/* __GLIBC__ */
}

static inline void
dt_io_warn_strpdt(const char *inp)
{
	error(0, "\
cannot make sense of `%s' using the given input formats", inp);
	return;
}


/* duration parser */
static inline __attribute__((pure, const)) struct __strpdtdur_st_s
__strpdtdur_st_initialiser(void)
{
#if defined HAVE_SLOPPY_STRUCTS_INIT
	static const struct __strpdtdur_st_s res = {};
#else
	static const struct __strpdtdur_st_s res;
#endif	/* HAVE_SLOPPY_STRUCTS_INIT */
	return res;
}

static inline int
__strpdtdur_more_p(struct __strpdtdur_st_s *st)
{
	return st->cont != NULL;
}

static inline void
__strpdtdur_free(struct __strpdtdur_st_s *st)
{
	if (st->durs != NULL) {
		free(st->durs);
	}
	return;
}

#endif	/* INCLUDED_dt_io_h_ */
