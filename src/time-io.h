/*** date-io.h -- helper for formats, parsing, printing, escaping, etc. */
#if !defined INCLUDED_time_io_h_
#define INCLUDED_time_io_h_

#include <stdlib.h>
#include <stdio.h>
#if defined __GLIBC__
# include <stdio_ext.h>
#endif	/* __GLIBC__ */
#include "time-core.h"
#include "strops.h"
#include "token.h"

#if !defined LIKELY
# define LIKELY(_x)	__builtin_expect(!!(_x), 1)
#endif
#if !defined UNLIKELY
# define UNLIKELY(_x)	__builtin_expect(!!(_x), 0)
#endif
#if !defined UNUSED
# define UNUSED(_x)	__attribute__((unused)) _x
#endif	/* !UNUSED */
#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

static bool
dt_io_now_p(const char *str)
{
	if (str == NULL) {
		return true;
	}
	switch ((char)(*str | 0x20)) {
	default:
		return false;
	case 'n':
		return strcasecmp(str, "now") == 0;
	}
}

static struct dt_t_s
dt_io_strpt_ep(const char *str, char *const *fmt, size_t nfmt, char **ep)
{
#if defined __C1X
	struct dt_t_s res = {.s = -1};
#else
	struct dt_t_s res;
#endif

#if !defined __C1X
	res.s = -1;
#endif

	/* init */
	if (ep) {
		*ep = NULL;
	}
	/* basic sanity check */
	if (UNLIKELY(dt_io_now_p(str))) {
		res = dt_time();
	} else if (nfmt == 0) {
		res = dt_strpt(str, NULL, ep);
	} else {
		for (size_t i = 0; i < nfmt; i++) {
			if ((res = dt_strpt(str, fmt[i], ep)).s >= 0) {
				break;
			}
		}
	}
	return res;
}

static struct dt_t_s __attribute__((unused))
dt_io_strpt(const char *input, char *const *fmt, size_t nfmt)
{
	return dt_io_strpt_ep(input, fmt, nfmt, NULL);
}

static struct dt_t_s  __attribute__((unused))
dt_io_find_strpt(
	const char *str, char *const *fmt, size_t nfmt,
	const char *needle, size_t needlen, char **sp, char **ep)
{
	const char *__sp = str;
#if defined __C1X
	struct dt_t_s t = {.s = -1};
#else
	struct dt_t_s t;
#endif

#if !defined __C1X
	t.s = -1;
#endif
	while ((__sp = strstr(__sp, needle)) &&
	       (t = dt_io_strpt_ep(
			__sp += needlen, fmt, nfmt, ep)).s < 0);
	*sp = (char*)__sp;
	return t;
}


/* needles for the grep mode */
typedef struct tgrep_atom_s *tgrep_atom_t;
typedef const struct tgrep_atom_s *const_tgrep_atom_t;

struct tgrpatm_payload_s {
	uint8_t flags;
#define TGRPATM_DIGITS	(1)
#define TGRPATM_P_SPEC	(2)
	int8_t off_min;
	int8_t off_max;
	const char *fmt;
};

struct tgrep_atom_s {
	/* 4 bytes */
	char needle;
	struct tgrpatm_payload_s pl;
};

struct tgrep_atom_soa_s {
	size_t natoms;
	char *needle;
	struct tgrpatm_payload_s *flesh;
};

static struct tgrep_atom_soa_s
make_tgrep_atom_soa(tgrep_atom_t atoms, size_t natoms)
{
	struct tgrep_atom_soa_s res = {.natoms = 0};

	res.needle = (char*)atoms;
	res.flesh = (void*)(res.needle + natoms);
	return res;
}

#define TGRPATM_NEEDLELESS_MODE_CHAR	(1)

static struct tgrep_atom_s
calc_tgrep_atom(const char *fmt)
{
	struct tgrep_atom_s res = {0};
	int8_t pndl_idx = 0;
	const char *fp = fmt;

	/* init */
	if (fmt == NULL) {
	std:
		/* standard format, %H:%M:%S */
		res.needle = ':';
		res.pl.off_min = -2;
		res.pl.off_max = -1;
		goto out;
	}
	/* rest here ... */
	while (*fp) {
		const char *fp_sav = fp;
		struct dt_spec_s spec = __tok_spec(fp_sav, (char**)&fp);

		/* pre checks */
		switch (spec.spfl) {
		case DT_SPFL_S_AMPM:
			res.pl.flags |= TGRPATM_P_SPEC;
			if (res.pl.off_min == res.pl.off_max) {
				pndl_idx = res.pl.off_min;
			}
			break;
		default:
			break;
		}
		/* real checks now */
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
		case DT_SPFL_N_TSTD:
			goto std;

		case DT_SPFL_N_HOUR:
		case DT_SPFL_N_MIN:
		case DT_SPFL_N_SEC:
			res.pl.off_min += -2;
			res.pl.off_max += -1;
			res.pl.flags |= TGRPATM_DIGITS;
			break;
		case DT_SPFL_S_AMPM:
			res.pl.off_min += -2;
			res.pl.off_max += -2;
			break;
		case DT_SPFL_N_NANO:
			res.pl.off_min += -9;
			res.pl.off_min += -1;
			res.pl.flags |= TGRPATM_DIGITS;
			break;
		}
	}
	if (res.needle == 0 && (res.pl.off_min || res.pl.off_max)) {
		if (res.pl.flags == TGRPATM_DIGITS) {
			/* purely digits is the least of our worries */
			int8_t tmp = (int8_t)-res.pl.off_min;

			/* swap-invert min and max */
			res.pl.off_min = (int8_t)-res.pl.off_max;
			res.pl.off_max = tmp;
			/* use a needle that is unlikely to occur and
			 * that will bubble to the front of the needlestack */
			res.needle = TGRPATM_NEEDLELESS_MODE_CHAR;
			goto out;
		} else if (res.pl.flags & TGRPATM_P_SPEC) {
			res.needle = TGRPATM_NEEDLELESS_MODE_CHAR;
			res.pl.off_min = res.pl.off_max = pndl_idx;
			res.pl.flags = TGRPATM_P_SPEC;
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

static __attribute__((unused)) struct tgrep_atom_soa_s
build_tneedle(tgrep_atom_t atoms, size_t natoms, char *const *fmt, size_t nfmt)
{
	struct tgrep_atom_soa_s res = make_tgrep_atom_soa(atoms, natoms);
	struct tgrep_atom_s a;;

	if (nfmt == 0) {
		if ((a = calc_tgrep_atom(NULL)).needle) {
			size_t idx = res.natoms++;
			res.needle[idx] = a.needle;
			res.flesh[idx] = a.pl;
		}
		goto out;
	}
	/* otherwise collect needles from all formats */
	for (size_t i = 0; i < nfmt && res.natoms < natoms; i++) {
		if ((a = calc_tgrep_atom(fmt[i])).needle) {
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
			res.needle[idx] = a.needle;
			res.flesh[idx] = a.pl;
		}
	}
out:
	/* terminate needle with \0 */
	res.needle[res.natoms] = '\0';
	return res;
}

static  __attribute__((unused)) struct dt_t_s
dt_io_find_strpt2(
	const char *str,
	const struct tgrep_atom_soa_s *needles,
	char **sp, char **ep)
{
#if defined __C1X
	struct dt_t_s t = {.s = -1};
#else
	struct dt_t_s t;
#endif
	const char *needle = needles->needle;
	const char *p;

#if !defined __C1X
	t.s = -1;
#endif
	for (p = str; *(p = xstrpbrk(p, needle)); p++) {
		/* find the offset */
		const struct tgrpatm_payload_s *fp;
		const char *np;

		/* find the index of *p in needle */
		for (np = needle, fp = needles->flesh; *np < *p; np++, fp++);

		/* nc points to the first occurrence of *p in needle,
		 * f is the associated grpatm payload */
		while (*np++ == *p) {
			const struct tgrpatm_payload_s f = *fp++;
			const char *fmt = f.fmt;

			/* lest we access stuff outside the boundaries */
			if (p + f.off_min < str /*|| p + f.off_max > ?*/) {
				continue;
			}
			/* check p + min_off .. p + max_off for times */
			for (int8_t i = f.off_min; i <= f.off_max; i++) {
				if ((t = dt_strpt(p + i, fmt, ep)).s >= 0) {
					p += i;
					goto found;
				}
			}
		}
	}
	/* otherwise check character classes */
	for (size_t i = 0; needle[i] == TGRPATM_NEEDLELESS_MODE_CHAR; i++) {
		struct tgrpatm_payload_s f = needles->flesh[i];
		const char *fmt = f.fmt;
		const char *ndl;

		/* look out for char classes*/
		switch (f.flags) {
			/* this isn't the bestest of approaches as it involves
			 * details about the contents behind the specifiers */
			static const char p_needle[] = "APap";
		case TGRPATM_P_SPEC:
			ndl = p_needle;
			break;

		case TGRPATM_DIGITS:
			/* yay, look for all digits */
			for (p = str; *p && !(*p >= '0' && *p <= '9'); p++);
			for (const char *q = p;
			     *q && *q >= '0' && *q <= '9'; q++) {
				if ((--f.off_min <= 0) &&
				    (t = dt_strpt(p, fmt, ep)).s >= 0) {
					goto found;
				}
			}
		default:
			continue;
		}
		/* not reached unless ndl is set */
		for (p = str; *(p = xstrpbrk(p, ndl)); p++) {
			if (p + f.off_min < str /*|| p + f.off_max > ?*/) {
				continue;
			}
			for (int8_t j = f.off_min; j <= f.off_max; j++) {
				if ((t = dt_strpt(p + j, fmt, ep)).s >=0) {
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
	return t;
}


/* formatter */
static inline size_t
dt_io_strft_autonl(
	char *restrict buf, size_t bsz, const char *fmt, struct dt_t_s that)
{
	size_t res = dt_strft(buf, bsz, fmt, that);

	if (res > 0 && buf[res - 1] != '\n') {
		/* auto-newline */
		buf[res++] = '\n';
	}
	return res;
}

static void __attribute__((unused))
dt_io_unescape(char *s)
{
	static const char esc_map[] = "\a\bcd\e\fghijklm\nopq\rs\tu\v";
	char *p, *q;

	if (UNLIKELY(s == NULL)) {
		return;
	} else if ((p = q = strchr(s, '\\'))) {
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

static void __attribute__((unused))
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

static int __attribute__((unused))
dt_io_write(struct dt_t_s t, const char *fmt)
{
	static char buf[64];
	size_t n;

	n = dt_io_strft_autonl(buf, sizeof(buf), fmt, t);
	__io_write(buf, n, stdout);
	return (n > 0) - 1;
}

static int __attribute__((unused))
dt_io_write_sed(
	struct dt_t_s t, const char *fmt,
	const char *line, size_t llen, const char *sp, const char *ep)
{
	static char buf[64];
	size_t n;

	n = dt_strft(buf, sizeof(buf), fmt, t);
	if (sp) {
		__io_write(line, sp - line, stdout);
	}
	__io_write(buf, n, stdout);
	if (ep) {
		__io_write(ep, line + llen - ep, stdout);
	}
	return (n > 0 || sp < ep) - 1;
}


/* error messages, warnings, etc. */
static __attribute__((unused)) void
dt_io_warn_strpt(const char *inp)
{
	fprintf(stderr, "\
cannot make sense of `%s' using the given input formats\n", inp);
	return;
}


/* duration parser */
struct __strptdur_st_s {
	int sign;
	const char *istr;
	const char *cont;
	struct dt_t_s curr;
};

static inline int
__strptdur_more_p(struct __strptdur_st_s *st)
{
	return st->cont != NULL;
}

static inline void
__strptdur_free(struct __strptdur_st_s *UNUSED(st))
{
	return;
}

DEFUN struct dt_t_s
dt_strptdur(const char *str, char **ep)
{
/* at the moment we allow only one format */
#if defined __C1X
	struct dt_t_s res = {.s = 0};
#else
	struct dt_t_s res;
#endif
	const char *sp = str;
	int tmp;


#if !defined __C1X
	res.s = 0;
#endif
	if (str == NULL) {
		goto out;
	}
	/* read just one component */
	tmp = strtol(sp, (char**)&sp, 10);
	switch (*sp++) {
	case '\0':
		/* must have been seconds then */
		sp--;
		break;
	case 's':
	case 'S':
		break;
	case 'm':
	case 'M':
		tmp *= SECS_PER_MIN;
		break;
	case 'h':
	case 'H':
		tmp *= SECS_PER_HOUR;
		break;
	default:
		sp = str;
		goto out;
	}
	/* assess */
	res.sdur += tmp;
out:
	if (ep) {
		*ep = (char*)sp;
	}
	return res;
}

static int __attribute__((unused))
dt_io_strptdur(struct __strptdur_st_s *st, const char *str)
{
/* at the moment we allow only one format */
	const char *sp = NULL;
	const char *ep = NULL;
	struct dt_t_s curr;
	int res = 0;

	/* check if we should continue */
	if (st->cont) {
		str = st->istr = st->cont;
	} else if ((st->istr = str)) {
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
	curr = dt_strptdur(sp, (char**)&ep);
	if (st->sign == 1) {
		st->curr.sdur += curr.sdur;
	} else if (st->sign == -1) {
		st->curr.sdur -= curr.sdur;
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

#endif	/* INCLUDED_date_io_h_ */
