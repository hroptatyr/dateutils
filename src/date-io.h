/*** date-io.h -- helper for formats, parsing, printing, escaping, etc. */
#if !defined INCLUDED_date_io_h_
#define INCLUDED_date_io_h_

#include <stdlib.h>
#include <stdio.h>
#if defined __GLIBC__
/* for *_unlocked protos */
# include <stdio_ext.h>
#endif	/* __GLIBC__ */
#include "date-core.h"
#include "strops.h"

#if !defined LIKELY
# define LIKELY(_x)	__builtin_expect(!!(_x), 1)
#endif
#if !defined UNLIKELY
# define UNLIKELY(_x)	__builtin_expect(!!(_x), 0)
#endif
#if !defined UNUSED
# define UNUSED(x)	__attribute__((unused)) x##_unused
#endif
#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif	/* __INTEL_COMPILER */

static bool
dt_io_today_p(const char *str)
{
	if (str == NULL) {
		return true;
	}
	switch ((char)(*str | 0x20)) {
	default:
		return false;
	case 'n':
		return strcasecmp(str, "now") == 0;
	case 't':
		return strcasecmp(str, "today") == 0;
	}
}

static struct dt_d_s
dt_io_strpd_ep(const char *str, const char *const *fmt, size_t nfmt, char **ep)
{
	struct dt_d_s res = {DT_UNK};

	/* init */
	if (ep) {
		*ep = NULL;
	}
	/* basic sanity check */
	if (UNLIKELY(dt_io_today_p(str))) {
		res = dt_date(DT_YMD);
	} else if (nfmt == 0) {
		res = dt_strpd(str, NULL, ep);
	} else {
		for (size_t i = 0; i < nfmt; i++) {
			if ((res = dt_strpd(str, fmt[i], ep)).typ > DT_UNK) {
				break;
			}
		}
	}
	return res;
}

static struct dt_d_s __attribute__((unused))
dt_io_strpd(const char *input, char *const *fmt, size_t nfmt)
{
	return dt_io_strpd_ep(input, fmt, nfmt, NULL);
}

static struct dt_d_s  __attribute__((unused))
dt_io_find_strpd(
	const char *str, char *const *fmt, size_t nfmt,
	const char *needle, size_t needlen, char **sp, char **ep)
{
	const char *__sp = str;
	struct dt_d_s d = {DT_UNK};

	if ((d = dt_io_strpd_ep(__sp, fmt, nfmt, ep)).typ == DT_UNK) {
		while ((__sp = strstr(__sp, needle)) &&
		       (d = dt_io_strpd_ep(
				__sp += needlen, fmt, nfmt, ep)).typ == DT_UNK);
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
	int8_t off_min;
	int8_t off_max;
	const char *fmt;
};
/* combos */
#define GRPATM_TINY_A_SPEC	(GRPATM_T_FLAG | GRPATM_A_SPEC)
#define GRPATM_TINY_B_SPEC	(GRPATM_T_FLAG | GRPATM_B_SPEC)

/* atoms are maps needle-character -> payload */
struct grep_atom_s {
	char needle;
	struct grpatm_payload_s pl;
};

struct grep_atom_soa_s {
	size_t natoms;
	char *needle;
	struct grpatm_payload_s *flesh;
};

static struct grep_atom_soa_s
make_grep_atom_soa(grep_atom_t atoms, size_t natoms)
{
	struct grep_atom_soa_s res = {.natoms = 0};

	res.needle = (char*)atoms;
	res.flesh = (void*)(res.needle + natoms);
	return res;
}

#define GRPATM_NEEDLELESS_MODE_CHAR	(1)

static struct grep_atom_s
calc_grep_atom(const char *fmt)
{
	struct grep_atom_s res = {0};
	int8_t andl_idx = 0;
	int8_t bndl_idx = 0;
	const char *fp = fmt;

	/* init */
	if (fmt == NULL) {
	std:
		/* standard format, %Y-%m-%d */
		res.needle = '-';
		res.pl.off_min = -4;
		res.pl.off_max = -4;
		goto out;
	}
	/* rest here ... */
	while (*fp) {
		const char *fp_sav = fp;
		struct dt_spec_s spec = __tok_spec(fp_sav, (char**)&fp);

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
		default:
			break;
		}
		if (spec.ord) {
			/* account for the extra 2 letters, but they're
			 * optional now, so don't fiddle with the max bit */
			res.pl.off_min -= 2;
			res.pl.flags |= GRPATM_ORDINALS;
		}
		if (spec.bizda) {
			/* account for the extra suffix character, it's
			 * optional again */
			res.pl.off_min -= 1;
			res.pl.flags |= GRPATM_SUFFIX;
		}
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
		case DT_SPFL_N_STD:
			goto std;
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
		case DT_SPFL_N_CNT_WEEK:
		case DT_SPFL_N_CNT_MON:
		case DT_SPFL_N_QTR:
		case DT_SPFL_N_MDAY:
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
		case DT_SPFL_N_CNT_YEAR:
			res.pl.off_min += -3;
			res.pl.off_max += -1;
			res.pl.flags |= GRPATM_DIGITS;
			break;
		case DT_SPFL_S_QTR:
			res.needle = 'Q';
			goto out;
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

static struct grep_atom_soa_s __attribute__((unused))
build_needle(grep_atom_t atoms, size_t natoms, char *const *fmt, size_t nfmt)
{
	struct grep_atom_soa_s res = make_grep_atom_soa(atoms, natoms);
	struct grep_atom_s a;

	if (nfmt == 0) {

		if ((a = calc_grep_atom(NULL)).needle) {
			size_t idx = res.natoms++;
			res.needle[idx] = a.needle;
			res.flesh[idx] = a.pl;
		}
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

static struct dt_d_s  __attribute__((unused))
dt_io_find_strpd2(
	const char *str,
	const struct grep_atom_soa_s *needles,
	char **sp, char **ep)
{
	struct dt_d_s d = {DT_UNK};
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
			const char *q;

			if (UNLIKELY((p + f.off_min) <= str)) {
				q = str;
			} else {
				q = p + f.off_min;
			}
			for (const char *r = p + f.off_max; *q && q <= r; q++) {
				if ((d = dt_strpd(q, fmt, ep)).typ) {
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
				    (d = dt_strpd(p, fmt, ep)).typ) {
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
				    (d = dt_strpd(p, fmt, ep)).typ) {
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
				if ((d = dt_strpd(p + j, fmt, ep)).typ) {
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
	return d;
}

/* formatter */
static inline size_t
dt_io_strfd_autonl(
	char *restrict buf, size_t bsz, const char *fmt, struct dt_d_s that)
{
	size_t res = dt_strfd(buf, bsz, fmt, that);

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
dt_io_write(struct dt_d_s d, const char *fmt)
{
	static char buf[64];
	size_t n;

	n = dt_io_strfd_autonl(buf, sizeof(buf), fmt, d);
	__io_write(buf, n, stdout);
	return (n > 0) - 1;
}

static int __attribute__((unused))
dt_io_write_sed(
	struct dt_d_s d, const char *fmt,
	const char *line, size_t llen, const char *sp, const char *ep)
{
	static char buf[64];
	size_t n;

	n = dt_strfd(buf, sizeof(buf), fmt, d);
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
static void __attribute__((unused))
dt_io_warn_strpd(const char *inp)
{
	fprintf(stderr, "\
cannot make sense of `%s' using the given input formats\n", inp);
	return;
}


/* duration parser */
/* we parse durations ourselves so we can cope with the
 * non-commutativity of duration addition:
 * 2000-03-30 +1m -> 2000-04-30 +1d -> 2000-05-01
 * 2000-03-30 +1d -> 2000-03-31 +1m -> 2000-04-30 */
struct __strpdur_st_s {
	int sign;
	const char *istr;
	const char *cont;
	struct dt_dur_s curr;
	size_t ndurs;
	struct dt_dur_s *durs;
};

static inline int
__strpdur_more_p(struct __strpdur_st_s *st)
{
	return st->cont != NULL;
}

static inline void
__strpdur_free(struct __strpdur_st_s *st)
{
	if (st->durs) {
		free(st->durs);
	}
	return;
}

static int __attribute__((unused))
dt_io_strpdur(struct __strpdur_st_s *st, const char *str)
{
/* at the moment we allow only one format */
	const char *sp = NULL;
	const char *ep = NULL;
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
	if ((st->curr = dt_strpdur(sp, (char**)&ep)).typ > DT_DUR_UNK) {
		if (st->durs == NULL) {
			st->durs = calloc(16, sizeof(*st->durs));
		} else if ((st->ndurs % 16) == 0) {
			st->durs = realloc(
				st->durs,
				(16 + st->ndurs) * sizeof(*st->durs));
			memset(st->durs + st->ndurs, 0, 16 * sizeof(*st->durs));
		}
		if ((st->sign == 1 && dt_dur_neg_p(st->curr)) ||
		    (st->sign == -1 && !dt_dur_neg_p(st->curr))) {
			st->durs[st->ndurs++] = dt_neg_dur(st->curr);
		} else {
			st->durs[st->ndurs++] = st->curr;
		}
	} else {
		res = -1;
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
