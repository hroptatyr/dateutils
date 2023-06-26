#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
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
#include <stdarg.h>
#include <errno.h>
#include "dt-core.h"
#include "dt-core-tz-glue.h"
#include "date-core-private.h"
#include "date-core-strpf.h"
#include "dt-locale.h"
#include "tzraw.h"
#include "tzmap.h"
#include "strops.h"
#include "token.h"
#include "nifty.h"
#include "dt-io.h"
#include "alist.h"

#if defined __INTEL_COMPILER
/* we MUST return a char* */
# pragma warning (disable:2203)
#elif defined __GNUC__
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif	/* __INTEL_COMPILER */


/* our own perror() implementation */
void
__attribute__((format(printf, 1, 2)))
error(const char *fmt, ...)
{
	va_list vap;
	va_start(vap, fmt);
	fputs(prog, stderr);
	fputs(": ", stderr);
	vfprintf(stderr, fmt, vap);
	va_end(vap);
	fputc('\n', stderr);
	return;
}

void
__attribute__((format(printf, 1, 2)))
serror(const char *fmt, ...)
{
	va_list vap;
	va_start(vap, fmt);
	fputs(prog, stderr);
	fputs(": ", stderr);
	vfprintf(stderr, fmt, vap);
	va_end(vap);
	if (errno) {
		fputc(':', stderr);
		fputc(' ', stderr);
		fputs(strerror(errno), stderr);
	}
	fputc('\n', stderr);
	return;
}


#include "strpdt-special.c"

/* coverity[-tainted_data_sink: arg-0] */
dt_strpdt_special_t
dt_io_strpdt_special(const char *str)
{
	const struct dt_strpdt_special_s *res;
	size_t len = strlen(str) & 0xfU;

	for (size_t i = 0U; i < len; i++) {
		if (UNLIKELY((signed char)str[i] < ' ')) {
			return STRPDT_UNK;
		}
	}
	if (UNLIKELY((res = __strpdt_special(str, len)) != NULL)) {
		return res->e;
	}
	return STRPDT_UNK;
}

struct dt_dt_s
dt_io_strpdt(
	const char *str,
	char *const *fmt, size_t nfmt,
	zif_t zone)
{
	struct dt_dt_s res = {DT_UNK};
	dt_strpdt_special_t now;

	/* basic sanity checks, catch phrases first */
	now = dt_io_strpdt_special(str);

	if (now > STRPDT_UNK) {
		res = dt_datetime((dt_dttyp_t)DT_YMD);
		/* rinse according to flags */
		switch (now) {
			static signed int add[] = {
				[STRPDT_YDAY] = -1,
				[STRPDT_TOMO] = 1,
			};
		case STRPDT_TOMO:
		case STRPDT_YDAY:
		case STRPDT_DATE:
			res.t = (struct dt_t_s){DT_TUNK};
			dt_make_d_only(&res, res.d.typ);
			if (LIKELY(now == STRPDT_DATE)) {
				break;
			}
			res.d = dt_dadd(res.d, dt_make_ddur(DT_DURD, add[now]));
			break;
		case STRPDT_TIME:
			res.d = (struct dt_d_s){DT_DUNK};
			dt_make_t_only(&res, res.t.typ);
			break;
		case STRPDT_NOW:
		case STRPDT_UNK:
		default:
			break;
		}
		return res;
	} else if (nfmt == 0) {
		res = dt_strpdt(str, NULL, NULL);
	} else {
		for (size_t i = 0; i < nfmt; i++) {
			if (!dt_unk_p(res = dt_strpdt(str, fmt[i], NULL))) {
				break;
			}
		}
	}
	return dtz_forgetz(res, zone);
}

struct dt_dt_s
dt_io_strpdt_ep(
	const char *str,
	char *const *fmt, size_t nfmt, char **ep,
	zif_t zone)
{
	struct dt_dt_s res = {DT_UNK};

	if (nfmt == 0) {
		res = dt_strpdt(str, NULL, ep);
	} else {
		for (size_t i = 0; i < nfmt; i++) {
			if (!dt_unk_p(res = dt_strpdt(str, fmt[i], ep))) {
				break;
			}
		}
	}
	return dtz_forgetz(res, zone);
}

struct dt_dt_s
dt_io_find_strpdt(
	const char *str, char *const *fmt, size_t nfmt,
	const char *needle, size_t needlen, char **sp, char **ep,
	zif_t zone)
{
	const char *__sp = str;
	struct dt_dt_s d;

	d = dt_io_strpdt_ep(__sp, fmt, nfmt, ep, zone);
	if (dt_unk_p(d)) {
		while ((__sp = strstr(__sp, needle)) &&
		       (d = dt_io_strpdt_ep(
				__sp += needlen, fmt, nfmt, ep, zone),
			dt_unk_p(d)));
	}
	*sp = (char*)__sp;
	return d;
}

struct dt_dt_s
dt_io_find_strpdt2(
	const char *str, size_t len,
	const struct grep_atom_soa_s *needles,
	char **sp, char **ep,
	zif_t zone)
{
	struct dt_dt_s d = {DT_UNK};
	const char *needle = needles->needle;
	const char *p = str;
	const char *const zp = str + len;

	for (; (p = xmempbrk(p, zp - p, needle)) < zp && *p; p++) {
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

			for (; q < zp && q <= r; q++) {
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
		case GRPATM_TA_SPEC:
			ndl = ta_needle;
			break;
		case GRPATM_TB_SPEC:
			ndl = tb_needle;
			break;
		case GRPATM_O_SPEC:
			ndl = o_needle;
			break;

		case GRPATM_DIGITS:
			/* yay, look for all digits */
			for (p = str; p < zp && !(*p >= '0' && *p <= '9'); p++);
			for (const char *q = p;
			     q < zp && *q >= '0' && *q <= '9'; q++) {
				if ((--f.off_min <= 0) &&
				    !dt_unk_p(d = dt_strpdt(p, fmt, ep))) {
					goto found;
				}
			}
			continue;

		case GRPATM_DIGITS | GRPATM_ORDINALS:
			/* yay, look for all digits and ordinals */
			for (p = str; p < zp && !(*p >= '0' && *p <= '9'); p++);
			for (const char *q = p; q < zp; q++) {
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
		for (p = str; (p = xmempbrk(p, zp - p, ndl)) < zp && *p; p++) {
			if (p + f.off_min < str || p + f.off_max > zp) {
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
	return dtz_forgetz(d, zone);
}

int
dt_io_write(struct dt_dt_s d, const char *fmt, zif_t zone, int apnd_ch)
{
	static char buf[256];
	size_t n;

	if (zone != NULL) {
		d = dtz_enrichz(d, zone);
	} else {
		/* zone == NULL is UTC, kill zdiff */
		d.zdiff = 0U;
		d.neg = 0U;
	}
	n = dt_io_strfdt(buf, sizeof(buf), fmt, d, apnd_ch);
	__io_write(buf, n, stdout);
	return (n > 0) - 1;
}


/* needles for the grep mode */
struct grep_atom_s
calc_grep_atom(const char *fmt)
{
	struct grep_atom_s res = {0};
	int8_t andl_idx = 0;
	int8_t bndl_idx = 0;
	int8_t pndl_idx = 0;

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
	} else {
		/* try and transform shortcuts */
		switch (__trans_dfmt(&fmt)) {
		default:
			break;
		case DT_LDN:
		case DT_JDN:
		case DT_MDN:
			/* there's no format specifiers for lilian/julian */
			res.pl.off_min += -10;
			res.pl.off_max += -1;
			res.pl.flags |= GRPATM_DIGITS;
			goto post_snarf;
		}
	}

	/* rest here ... */
	for (const char *fp = fmt; *fp;) {
		const char *fp_sav = fp;
		struct dt_spec_s spec = __tok_spec(fp_sav, &fp);

		/* pre checks */
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
			switch (spec.abbr) {
			case DT_SPMOD_LONG:
				res.pl.off_min += -4;
				res.pl.off_max += -4;
				break;
			case DT_SPMOD_NORM:
				res.pl.off_min += -2;
				res.pl.off_max += -2;
				break;
			case DT_SPMOD_ABBR:
				res.pl.off_min += -1;
				res.pl.off_max += -1;
				break;
			case DT_SPMOD_ILL:
			default:
				/* should be impossible */
				break;
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
			if (res.pl.off_min == res.pl.off_max) {
				andl_idx = res.pl.off_min;
			}
			switch (spec.abbr) {
			case DT_SPMOD_NORM:
				res.pl.off_min -= dut_rabbr_wday.max;
				res.pl.off_max -= dut_rabbr_wday.min;
				res.pl.flags |= GRPATM_A_SPEC;
				break;
			case DT_SPMOD_ABBR:
				res.pl.off_min -= 1;
				res.pl.off_max -= 1;
				res.pl.flags |= GRPATM_TA_SPEC;
				break;
			case DT_SPMOD_LONG:
				res.pl.off_min -= dut_rlong_wday.max;
				res.pl.off_max -= dut_rlong_wday.min;
				res.pl.flags |= GRPATM_A_SPEC;
				break;
			default:
				break;
			}
			break;
		case DT_SPFL_S_MON:
			if (res.pl.off_min == res.pl.off_max) {
				bndl_idx = res.pl.off_min;
			}
			switch (spec.abbr) {
			case DT_SPMOD_NORM:
				res.pl.off_min -= dut_rabbr_mon.max;
				res.pl.off_max -= dut_rabbr_mon.min;
				res.pl.flags |= GRPATM_B_SPEC;
				break;
			case DT_SPMOD_ABBR:
				res.pl.off_min -= 1;
				res.pl.off_max -= 1;
				res.pl.flags |= GRPATM_TB_SPEC;
				break;
			case DT_SPMOD_LONG:
				res.pl.off_min -= dut_rlong_mon.max;
				res.pl.off_max -= dut_rlong_mon.min;
				res.pl.flags |= GRPATM_B_SPEC;
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
			res.pl.flags |= GRPATM_P_SPEC;
			if (res.pl.off_min == res.pl.off_max) {
				pndl_idx = res.pl.off_min;
			}
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

post_snarf:
	if (res.needle == 0 && (res.pl.off_min || res.pl.off_max)) {
		if ((res.pl.flags & ~(GRPATM_DIGITS | GRPATM_ORDINALS)) == 0) {
			/* ah, only digits, that's good */
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
		} else if (res.pl.flags & GRPATM_TA_SPEC) {
			/* very short but better than going for digits aye? */
			res.needle = GRPATM_NEEDLELESS_MODE_CHAR;
			res.pl.off_min = res.pl.off_max = andl_idx;
			res.pl.flags = GRPATM_TA_SPEC;
			goto out;
		} else if (res.pl.flags & GRPATM_TB_SPEC) {
			res.needle = GRPATM_NEEDLELESS_MODE_CHAR;
			res.pl.off_min = res.pl.off_max = bndl_idx;
			res.pl.flags = GRPATM_TB_SPEC;
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

struct grep_atom_soa_s
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

void
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


/* duration parser */
/* we parse durations ourselves so we can cope with the
 * non-commutativity of duration addition:
 * 2000-03-30 +1m -> 2000-04-30 +1d -> 2000-05-01
 * 2000-03-30 +1d -> 2000-03-31 +1m -> 2000-04-30 */
int
__add_dur(struct __strpdtdur_st_s *st, struct dt_dtdur_s dur)
{
	if (dt_durunk_p(dur)) {
		return -1;
	}
	if (st->durs == NULL) {
		st->durs = calloc(16, sizeof(*st->durs));
	} else if ((st->ndurs % 16) == 0) {
		void *tmp;
		tmp = realloc(st->durs, (16 + st->ndurs) * sizeof(*st->durs));
		if (UNLIKELY(tmp == NULL)) {
			return -1;
		}
		/* otherwise proceed as usual */
		st->durs = tmp;
		memset(st->durs + st->ndurs, 0, 16 * sizeof(*st->durs));
	}
	st->durs[st->ndurs++] = dur;
	return 0;
}

int
dt_io_strpdtdur(struct __strpdtdur_st_s *st, const char *str)
{
/* at the moment we allow only one format */
	const char *sp = NULL;
	const char *ep = NULL;
	int res = 0;

	/* check if we should continue */
	if (st->cont != NULL) {
		str = st->istr = st->cont;
	} else if (LIKELY((st->istr = str) != NULL)) {
		;
	} else {
		res = -1;
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
		case '/':
			st->flags = 1U;
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
		struct dt_dtdur_s d = dt_strpdtdur(sp, (char**)&ep);
		if ((st->sign == 1 && dt_dtdur_neg_p(d)) ||
		    (st->sign == -1 && !dt_dtdur_neg_p(d))) {
			d = dt_neg_dtdur(d);
		}
		d.cocl = (bool)(st->flags & 0x1U);
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

/* dt-io.c ends here */
