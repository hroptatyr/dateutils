/*** date-io.h -- helper for formats, parsing, printing, escaping, etc. */
#if !defined INCLUDED_time_io_h_
#define INCLUDED_time_io_h_

#include <stdlib.h>
#include <stdio.h>
#include <stdio_ext.h>
#include "time-core.h"

#if !defined LIKELY
# define LIKELY(_x)	__builtin_expect(!!(_x), 1)
#endif
#if !defined UNLIKELY
# define UNLIKELY(_x)	__builtin_expect(!!(_x), 0)
#endif
#if !defined UNUSED
# define UNUSED(_x)	__attribute__((unused)) _x
#endif	/* !UNUSED */

static struct dt_t_s
dt_time(void)
{
/* get the current time, will wander into time-core.c */
	struct dt_t_s res;

	res.hms.h = 12;
	res.hms.m = 34;
	res.hms.s = 56;
	res.hms.ns = 0;
	return res;
}

static struct dt_t_s
dt_io_strpt_ep(const char *str, char *const *fmt, size_t nfmt, char **ep)
{
	struct dt_t_s res = {.s = -1};

	/* init */
	if (ep) {
		*ep = NULL;
	}
	/* basic sanity check */
	if (str == NULL || strcmp(str, "now") == 0) {
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
	struct dt_t_s t = {.s = -1};

	while ((__sp = strstr(__sp, needle)) &&
	       (t = dt_io_strpt_ep(
			__sp += needlen, fmt, nfmt, ep)).s < 0);
	*sp = (char*)__sp;
	return t;
}

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


static int __attribute__((unused))
dt_io_write(struct dt_t_s t, const char *fmt)
{
	static char buf[64];
	size_t n;

	n = dt_io_strft_autonl(buf, sizeof(buf), fmt, t);
	fwrite_unlocked(buf, sizeof(*buf), n, stdout);
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
		fwrite_unlocked(line, sizeof(char), sp - line, stdout);
	}
	fwrite_unlocked(buf, sizeof(*buf), n, stdout);
	if (ep) {
		fwrite_unlocked(ep, sizeof(char), line + llen - ep, stdout);
	}
	return (n > 0 || sp < ep) - 1;
}


/* error messages, warnings, etc. */
static void
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
	struct dt_t_s res = {.s = 0};
	const char *sp = str;
	int tmp;

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

#endif	/* INCLUDED_date_io_h_ */
