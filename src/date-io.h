/*** date-io.h -- helper for formats, parsing, printing, escaping, etc. */
#if !defined INCLUDED_date_io_h_
#define INCLUDED_date_io_h_

#include "date-core.h"

#if !defined LIKELY
# define LIKELY(_x)	__builtin_expect(!!(_x), 1)
#endif
#if !defined UNLIKELY
# define UNLIKELY(_x)	__builtin_expect(!!(_x), 0)
#endif

static struct dt_d_s
dt_io_strpd_ep(const char *str, char *const *fmt, size_t nfmt, char **ep)
{
	struct dt_d_s res = {DT_UNK};

	/* init */
	if (ep) {
		*ep = NULL;
	}
	/* basic sanity check */
	if (str == NULL || strcmp(str, "now") == 0) {
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

#endif	/* INCLUDED_date_io_h_ */
