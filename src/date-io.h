/*** date-io.h -- helper for formats, parsing, printing, escaping, etc. */
#if !defined INCLUDED_date_io_h_
#define INCLUDED_date_io_h_

#include "date-core.h"

static struct dt_d_s __attribute__((unused))
dt_io_strpd(const char *input, const char *const *fmt, size_t nfmt)
{
	struct dt_d_s res = {DT_UNK};

	/* basic sanity check */
	if (input == NULL || strcmp(input, "now") == 0) {
		return dt_date(DT_YMD);
	} else if (nfmt == 0) {
		res = dt_strpd(input, NULL);
	} else {
		for (size_t i = 0; i < nfmt; i++) {
			if ((res = dt_strpd(input, fmt[i])).typ && res.u ) {
				break;
			}
		}
	}
	if (res.u == 0) {
		res.typ = DT_UNK;
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

#endif	/* INCLUDED_date_io_h_ */
