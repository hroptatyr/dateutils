/*** date-io.h -- helper for formats, parsing, printing, escaping, etc. */
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
#include "dt-core-tz-glue.h"
#include "strops.h"
#include "token.h"
#include "nifty.h"

#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

typedef enum {
	STRPDT_UNK,
	STRPDT_DATE,
	STRPDT_TIME,
	STRPDT_NOW,
	STRPDT_YDAY,
	STRPDT_TOMO,
} dt_strpdt_special_t;

#include "strpdt-special.c"

static dt_strpdt_special_t
dt_io_strpdt_special(const char *str)
{
	size_t len = strlen(str);
	const struct dt_strpdt_special_s *res;

	if (UNLIKELY((res = __strpdt_special(str, len)) != NULL)) {
		return res->e;
	}
	return STRPDT_UNK;
}

static struct dt_dt_s
dt_io_strpdt_ep(
	const char *str,
	const char *const *fmt, size_t nfmt, char **ep,
	zif_t zone)
{
	struct dt_dt_s res = dt_dt_initialiser();
	dt_strpdt_special_t now;

	/* init */
	if (ep != NULL) {
		*ep = NULL;
	}
	/* basic sanity checks, catch phrases first */
	now = dt_io_strpdt_special(str);

	if (now > STRPDT_UNK) {
		res = dt_datetime((dt_dttyp_t)DT_YMD);
		/* rinse according to flags */
		switch (now) {
			signed int add;
		case STRPDT_TOMO:
			add = 1;
			goto date;
		case STRPDT_YDAY:
			add = -1;
			goto date;
		case STRPDT_DATE:
			add = 0;
		date:
			res.t = dt_t_initialiser();
			dt_make_d_only(&res, res.d.typ);
			if (add) {
				res.d = dt_dadd(res.d, dt_make_daisydur(add));
			}
			break;
		case STRPDT_TIME:
			res.d = dt_d_initialiser();
			dt_make_t_only(&res, res.t.typ);
			break;
		case STRPDT_NOW:
		case STRPDT_UNK:
		default:
			break;
		}
		return res;
	} else if (nfmt == 0) {
		res = dt_strpdt(str, NULL, ep);
	} else {
		for (size_t i = 0; i < nfmt; i++) {
			if (!dt_unk_p(res = dt_strpdt(str, fmt[i], ep))) {
				break;
			}
		}
	}
	if (LIKELY(!dt_unk_p(res)) && zone != NULL) {
		return dtz_forgetz(res, zone);
	}
	return res;
}

static __attribute__((unused)) struct dt_dt_s
dt_io_strpdt(const char *input, char *const *fmt, size_t nfmt, zif_t zone)
{
	return dt_io_strpdt_ep(input, (const char*const*)fmt, nfmt, NULL, zone);
}

static __attribute__((unused)) struct dt_dt_s
dt_io_find_strpdt(
	const char *str, char *const *fmt, size_t nfmt,
	const char *needle, size_t needlen, char **sp, char **ep,
	zif_t zone)
{
	const char *__sp = str;
	struct dt_dt_s d = dt_dt_initialiser();
	const char *const *cfmt = (const char*const*)fmt;

	d = dt_io_strpdt_ep(__sp, cfmt, nfmt, ep, zone);
	if (dt_unk_p(d)) {
		while ((__sp = strstr(__sp, needle)) &&
		       (d = dt_io_strpdt_ep(
				__sp += needlen, cfmt, nfmt, ep, zone),
			dt_unk_p(d)));
	}
	*sp = (char*)__sp;
	return d;
}


/* needles for the grep mode */
typedef struct grep_atom_s *grep_atom_t;
typedef const struct grep_atom_s *const_grep_atom_t;

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

static struct grep_atom_soa_s
make_grep_atom_soa(grep_atom_t atoms, size_t natoms)
{
	struct grep_atom_soa_s res = __grep_atom_soa_initialiser();

	res.needle = (char*)atoms;
	res.flesh = (void*)(res.needle + natoms);
	return res;
}

#define GRPATM_NEEDLELESS_MODE_CHAR	(1)

static struct grep_atom_s
calc_grep_atom(const char *fmt)
{
	struct grep_atom_s res = __grep_atom_initialiser();
	int8_t andl_idx = 0;
	int8_t bndl_idx = 0;
	int8_t pndl_idx = 0;
	const char *fp = fmt;

	/* init */
	if (fmt == NULL) {
	dstd:
		/* standard format, %Y-%m-%d */
		res.needle = '-';
		res.pl.off_min = -4;
		res.pl.off_max = -4;
		goto out;
	tstd:
		/* standard format, %H:%M:%S */
		res.needle = ':';
		res.pl.off_min = -2;
		res.pl.off_max = -1;
		goto out;
	}
	/* rest here ... */
	while (*fp) {
		const char *fp_sav = fp;
		struct dt_spec_s spec = __tok_spec(fp_sav, &fp);

		/* pre checks */
		switch (spec.spfl) {
		case DT_SPFL_S_WDAY:
			res.pl.flags |= GRPATM_A_SPEC;
			if (res.pl.off_min == res.pl.off_max) {
				andl_idx = res.pl.off_min;
			}
			break;
		case DT_SPFL_S_MON:
			res.pl.flags |= GRPATM_B_SPEC;
			if (res.pl.off_min == res.pl.off_max) {
				bndl_idx = res.pl.off_min;
			}
			break;
		case DT_SPFL_S_AMPM:
			res.pl.flags |= GRPATM_P_SPEC;
			if (res.pl.off_min == res.pl.off_max) {
				pndl_idx = res.pl.off_min;
			}
			break;
		default:
			break;
		}
		if (spec.ord) {
			/* account for the extra 2 letters, but they're
			 * optional now, so don't fiddle with the max bit */
			res.pl.off_min -= 2;
			res.pl.flags |= GRPATM_ORDINALS;
		}
#if 0
		if (spec.bizda) {
			/* account for the extra suffix character, it's
			 * optional again */
			res.pl.off_min -= 1;
			res.pl.flags |= GRPATM_SUFFIX;
		}
#endif
		switch (spec.spfl) {
		case DT_SPFL_UNK:
			/* found a non-spec character that can be
			 * used as needle, we should check for the
			 * character's suitability though, a space is not
			 * the best needle to find in a haystack of
			 * english text, in fact it's more like a haystack
			 * itself */
			res.needle = *fp_sav;
			goto out;
		case DT_SPFL_LIT_PERCENT:
			/* very good needle character methinks */
			res.needle = '%';
			goto out;
		case DT_SPFL_LIT_NL:
			/* quite good needle characters */
			res.needle = '\n';
			goto out;
		case DT_SPFL_LIT_TAB:
			res.needle = '\t';
			goto out;
		case DT_SPFL_N_DSTD:
			goto dstd;
		case DT_SPFL_N_TSTD:
			goto tstd;
		case DT_SPFL_N_YEAR:
			if (spec.abbr != DT_SPMOD_ABBR) {
				res.pl.off_min += -4;
				res.pl.off_max += -4;
			} else {
				res.pl.off_min += -2;
				res.pl.off_max += -2;
			}
			res.pl.flags |= GRPATM_DIGITS;
			break;
		case DT_SPFL_N_MON:
		case DT_SPFL_N_DCNT_WEEK:
		case DT_SPFL_N_DCNT_MON:
		case DT_SPFL_N_WCNT_MON:
		case DT_SPFL_N_WCNT_YEAR:
		case DT_SPFL_N_QTR:
		case DT_SPFL_N_HOUR:
		case DT_SPFL_N_MIN:
		case DT_SPFL_N_SEC:
			res.pl.off_min += -2;
			res.pl.off_max += -1;
			res.pl.flags |= GRPATM_DIGITS;
			break;
		case DT_SPFL_S_WDAY:
			if (spec.abbr == DT_SPMOD_LONG) {
				/* Wednesday */
				res.pl.off_min += -9;
				/* Friday */
				res.pl.off_max += -6;
				break;
			}
		case DT_SPFL_S_MON:
			if (spec.abbr == DT_SPMOD_LONG) {
				/* September */
				res.pl.off_min += -9;
				/* May */
				res.pl.off_max += -3;
				break;
			}
			switch (spec.abbr) {
			case DT_SPMOD_NORM:
				res.pl.off_min += -3;
				res.pl.off_max += -3;
				break;
			case DT_SPMOD_ABBR:
				res.pl.off_min += -1;
				res.pl.off_max += -1;
				res.pl.flags |= GRPATM_T_FLAG;
				break;
			default:
				break;
			}
			break;
		case DT_SPFL_N_DCNT_YEAR:
			res.pl.off_min += -3;
			res.pl.off_max += -1;
			res.pl.flags |= GRPATM_DIGITS;
			break;
		case DT_SPFL_S_QTR:
			res.needle = 'Q';
			goto out;
		case DT_SPFL_S_AMPM:
			res.pl.off_min += -2;
			res.pl.off_max += -2;
			break;

		case DT_SPFL_N_EPOCH:
			res.pl.off_min += -10;
			res.pl.off_max += -1;
			res.pl.flags |= GRPATM_DIGITS;
			break;
		default:
			break;
		}
	}
	if (res.needle == 0 && (res.pl.off_min || res.pl.off_max)) {
		if ((res.pl.flags & ~(GRPATM_DIGITS | GRPATM_ORDINALS)) == 0) {
			/* ah, only digits, thats good */
			int8_t tmp = (int8_t)-res.pl.off_min;

			/* swap-invert min and max */
			res.pl.off_min = (int8_t)-res.pl.off_max;
			res.pl.off_max = tmp;
			/* use a needle that is unlikely to occur and
			 * that will bubble to the front of the needlestack */
			res.needle = GRPATM_NEEDLELESS_MODE_CHAR;
			goto out;
		} else if (res.pl.flags & GRPATM_A_SPEC) {
			res.needle = GRPATM_NEEDLELESS_MODE_CHAR;
			res.pl.off_min = res.pl.off_max = andl_idx;
			res.pl.flags = GRPATM_A_SPEC;
			goto out;
		} else if (res.pl.flags & GRPATM_B_SPEC) {
			res.needle = GRPATM_NEEDLELESS_MODE_CHAR;
			res.pl.off_min = res.pl.off_max = bndl_idx;
			res.pl.flags = GRPATM_B_SPEC;
			goto out;
		} else if (res.pl.flags & GRPATM_O_SPEC) {
			res.needle = GRPATM_NEEDLELESS_MODE_CHAR;
		} else if (res.pl.flags & GRPATM_P_SPEC) {
			res.needle = GRPATM_NEEDLELESS_MODE_CHAR;
			res.pl.off_min = res.pl.off_max = pndl_idx;
			res.pl.flags = GRPATM_P_SPEC;
			goto out;
		}
	}
	/* naught flags mean the usual needle char search */
	res.pl.flags = 0;
out:
	/* finally assign the format */
	if (res.needle || res.pl.flags) {
		res.pl.fmt = fmt;
	}
	return res;
}

static __attribute__((unused)) struct grep_atom_soa_s
build_needle(grep_atom_t atoms, size_t natoms, char *const *fmt, size_t nfmt)
{
	struct grep_atom_soa_s res = make_grep_atom_soa(atoms, natoms);
	struct grep_atom_s a;

	if (nfmt == 0) {
		/* inject the standard needles for %F and %T */
		size_t idx;

		/* standard format %F */
		idx = res.natoms++;
		res.needle[idx] = '-';
		res.flesh[idx].off_min = -4;
		res.flesh[idx].off_max = -4;
		res.flesh[idx].fmt = NULL;

		/* standard format, %T */
		idx = res.natoms++;
		res.needle[idx] = ':';
		res.flesh[idx].off_min = -2;
		res.flesh[idx].off_max = -1;
		res.flesh[idx].fmt = NULL;
		goto out;
	}
	/* otherwise collect needles from all formats */
	for (size_t i = 0; i < nfmt && res.natoms < natoms; i++) {
		if ((a = calc_grep_atom(fmt[i])).needle) {
			const char *ndl = res.needle;
			size_t idx = res.natoms++;
			size_t j;

			/* stable insertion sort, find the slot first ... */
			for (j = 0; j < idx && ndl[j] <= a.needle; j++);
			/* ... j now points to where we insert, so move
			 * everything behind j */
			if (j < idx) {
				memmove(res.needle + j + 1, ndl + j, idx - j);
				memmove(
					res.flesh + j + 1,
					res.flesh + j,
					(idx - j) * sizeof(*res.flesh));
			}
			res.needle[j] = a.needle;
			res.flesh[j] = a.pl;
		}
	}
out:
	/* terminate needle with \0 */
	res.needle[res.natoms] = '\0';
	return res;
}

static __attribute__((unused)) struct dt_dt_s
dt_io_find_strpdt2(
	const char *str,
	const struct grep_atom_soa_s *needles,
	char **sp, char **ep,
	zif_t zone)
{
	struct dt_dt_s d = dt_dt_initialiser();
	const char *needle = needles->needle;
	const char *p = str;

	for (; *(p = xstrpbrk(p, needle)); p++) {
		/* find the offset */
		const struct grpatm_payload_s *fp;
		const char *np;

		for (np = needle, fp = needles->flesh; *np < *p; np++, fp++);

		/* nc points to the first occurrence of *p in needle,
		 * f is the associated grpatm payload */
		while (*np++ == *p) {
			const struct grpatm_payload_s f = *fp++;
			const char *fmt = f.fmt;
			const char *q = p + f.off_min;
			const char *r = p + f.off_max;

			if (UNLIKELY(q < str)) {
				q = str;
			}

			for (; *q && q <= r; q++) {
				if (!dt_unk_p(d = dt_strpdt(q, fmt, ep))) {
					p = q;
					goto found;
				}
			}
		}
	}
	/* otherwise check character classes */
	for (size_t i = 0; needle[i] == GRPATM_NEEDLELESS_MODE_CHAR; i++) {
		struct grpatm_payload_s f = needles->flesh[i];
		const char *fmt = f.fmt;
		const char *ndl;

		/* look out for char classes*/
		switch (f.flags) {
			/* this isn't the bestest of approaches as it involves
			 * details about the contents behind the specifiers */
			static const char a_needle[] = "FMSTWfmstw";
			static const char ta_needle[] = "MTWRFAS";
			static const char b_needle[] = "ADFJMNOSadfjmnos";
			static const char tb_needle[] = "FGHJKMNQUVXZ";
			static const char o_needle[] = "CDILMVXcdilmvx";
		case GRPATM_A_SPEC:
			ndl = a_needle;
			break;
		case GRPATM_B_SPEC:
			ndl = b_needle;
			break;
		case GRPATM_TINY_A_SPEC:
			ndl = ta_needle;
			break;
		case GRPATM_TINY_B_SPEC:
			ndl = tb_needle;
			break;
		case GRPATM_O_SPEC:
			ndl = o_needle;
			break;

		case GRPATM_DIGITS:
			/* yay, look for all digits */
			for (p = str; *p && !(*p >= '0' && *p <= '9'); p++);
			for (const char *q = p;
			     *q && *q >= '0' && *q <= '9'; q++) {
				if ((--f.off_min <= 0) &&
				    !dt_unk_p(d = dt_strpdt(p, fmt, ep))) {
					goto found;
				}
			}
			continue;

		case GRPATM_DIGITS | GRPATM_ORDINALS:
			/* yay, look for all digits and ordinals */
			for (p = str; *p && !(*p >= '0' && *p <= '9'); p++);
			for (const char *q = p; *q; q++) {
				switch (*q) {
				case '0' ... '9':
					break;
				case 't':
					if (*++q == 'h') {
						break;
					}
					goto bugger;
				case 's':
					if (*++q == 't') {
						break;
					}
					goto bugger;
				case 'n':
				case 'r':
					if (*++q == 'd') {
						break;
					}
					goto bugger;
				default:
					goto bugger;
				}
				if ((--f.off_min <= 0) &&
				    !dt_unk_p(d = dt_strpdt(p, fmt, ep))) {
					goto found;
				}
			}
		bugger:
		default:
			continue;
		}
		/* not reached unless ndl is set */
		for (p = str; *(p = xstrpbrk(p, ndl)); p++) {
			if (p + f.off_min < str /*|| p + f.off_max > ?*/) {
				continue;
			}
			for (int8_t j = f.off_min; j <= f.off_max; j++) {
				if (!dt_unk_p(d = dt_strpdt(p + j, fmt, ep))) {
					p += j;
					goto found;
				}
			}
		}
	}
	/* reset to some sane defaults */
	*ep = (char*)(p = str);
found:
	*sp = (char*)p;
	if (LIKELY(!dt_unk_p(d)) && zone != NULL) {
		return dtz_forgetz(d, zone);
	}
	return d;
}

/* formatter */
static inline size_t
dt_io_strfdt(
	char *restrict buf, size_t bsz, const char *fmt, struct dt_dt_s that)
{
	return dt_strfdt(buf, bsz, fmt, that);
}

static inline size_t
dt_io_strfdt_autonl(
	char *restrict buf, size_t bsz, const char *fmt, struct dt_dt_s that)
{
	size_t res = dt_io_strfdt(buf, bsz, fmt, that);

	if (res > 0 && buf[res - 1] != '\n') {
		/* auto-newline */
		buf[res++] = '\n';
	}
	return res;
}

static __attribute__((unused)) void
dt_io_unescape(char *s)
{
	static const char esc_map[] = "\a\bcd\e\fghijklm\nopq\rs\tu\v";
	char *p, *q;

	if (UNLIKELY(s == NULL)) {
		return;
	} else if ((p = q = strchr(s, '\\')) != NULL) {
		do {
			if (*p != '\\' || !*++p) {
				*q++ = *p++;
			} else if (*p < 'a' || *p > 'v') {
				*q++ = *p++;
			} else {
				*q++ = esc_map[*p++ - 'a'];
			}
		} while (*p);
		*q = '\0';
	}
	return;
}


#define MAGIC_CHAR	'~'

static __attribute__((unused)) void
fixup_argv(int argc, char *argv[], const char *additional)
{
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-' &&
		    ((argv[i][1] >= '1' && argv[i][1] <= '9') ||
		     (additional && strchr(additional, argv[i][1])))) {
			/* assume this is meant to be an integer
			 * as opposed to an option that begins with a digit */
			argv[i][0] = MAGIC_CHAR;
		}
	}
	return;
}

static inline char*
unfixup_arg(char *arg)
{
	if (UNLIKELY(arg[0] == MAGIC_CHAR)) {
		arg[0] = '-';
	}
	return arg;
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
__io_setlocking_bycaller(FILE *__attribute__((unused)) fp)
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

static __attribute__((unused)) int
dt_io_write(struct dt_dt_s d, const char *fmt, zif_t zone)
{
	static char buf[64];
	size_t n;

	if (LIKELY(!dt_unk_p(d)) && zone != NULL) {
		d = dtz_enrichz(d, zone);
	}
	n = dt_io_strfdt_autonl(buf, sizeof(buf), fmt, d);
	__io_write(buf, n, stdout);
	return (n > 0) - 1;
}

static __attribute__((unused)) int
dt_io_write_sed(
	struct dt_dt_s d, const char *fmt,
	const char *line, size_t llen, const char *sp, const char *ep,
	zif_t zone)
{
	static char buf[64];
	size_t n;

	if (LIKELY(!dt_unk_p(d)) && zone != NULL) {
		d = dtz_enrichz(d, zone);
	}
	n = dt_io_strfdt(buf, sizeof(buf), fmt, d);
	if (sp != NULL) {
		__io_write(line, sp - line, stdout);
	}
	__io_write(buf, n, stdout);
	if (ep != NULL) {
		size_t eolen = line + llen - ep;
		if (LIKELY(eolen > 0)) {
			__io_write(ep, line + llen - ep, stdout);
		} else {
			__io_putc('\n', stdout);
		}
	}
	return (n > 0 || sp < ep) - 1;
}


/* error messages, warnings, etc. */
static __attribute__((format(printf, 2, 3))) void
error(int eno, const char *fmt, ...);

static inline void
dt_io_warn_strpdt(const char *inp)
{
	error(0, "\
cannot make sense of `%s' using the given input formats", inp);
	return;
}


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

static int
__add_dur(struct __strpdtdur_st_s *st, struct dt_dt_s dur)
{
	if (dt_unk_p(dur)) {
		return -1;
	}
	if (st->durs == NULL) {
		st->durs = calloc(16, sizeof(*st->durs));
	} else if ((st->ndurs % 16) == 0) {
		st->durs = realloc(
			st->durs,
			(16 + st->ndurs) * sizeof(*st->durs));
		memset(st->durs + st->ndurs, 0, 16 * sizeof(*st->durs));
	}
	st->durs[st->ndurs++] = dur;
	return 0;
}

static __attribute__((unused)) int
dt_io_strpdtdur(struct __strpdtdur_st_s *st, const char *str)
{
/* at the moment we allow only one format */
	const char *sp = NULL;
	const char *ep = NULL;
	int res = 0;

	/* check if we should continue */
	if (st->cont != NULL) {
		str = st->istr = st->cont;
	} else if ((st->istr = str) != NULL) {
		;
	} else {
		goto out;
	}

	/* read over signs and prefixes */
	sp = str;
	while (1) {
		switch (*sp++) {
		case '\0':
			res = -1;
			ep = sp;
			goto out;
		case '+':
			st->sign = 1;
			break;
		case '-':
			st->sign = -1;
			break;
		case '=':
			if (st->sign > 0) {
				st->sign++;
			} else if (st->sign < 0) {
				st->sign--;
			} else {
				st->sign = 0;
			}
			break;
		case '>':
			st->sign = 2;
			break;
		case '<':
			st->sign = -2;
			break;
		case 'p':
		case 'P':
		case ' ':
		case '\t':
		case '\n':
			continue;
		default:
			sp--;
			break;
		}
		break;
	}

	/* try reading the stuff with our strpdur() */
	{
		struct dt_dt_s d = dt_strpdtdur(sp, (char**)&ep);
		if ((st->sign == 1 && dt_dtdur_neg_p(d)) ||
		    (st->sign == -1 && !dt_dtdur_neg_p(d))) {
			d = dt_neg_dtdur(d);
		}
		res = __add_dur(st, d);
	}
out:
	if (((st->cont = ep) && *ep == '\0') || (sp == ep)) {
		st->sign = 0;
		st->cont = NULL;
	}
	return res;
}

#if defined __INTEL_COMPILER
# pragma warning (default:2203)
#elif defined __GNUC__
# pragma GCC diagnostic warning "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

#endif	/* INCLUDED_dt_io_h_ */
