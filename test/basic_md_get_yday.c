#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include "strops.h"
#include "strops.c"
#include "token.h"
#include "token.c"
#include "date-core.h"
#include "date-core.c"
#include "dt-locale.h"
#include "dt-locale.c"

static unsigned int
super(unsigned int res)
{
	for (int y = 1917; y < 2299; y++) {
		for (int m = 1; m <= 12; m++) {
			for (int d = 0; d < 32; d++) {
				unsigned int yd;
				yd = __md_get_yday(y, m, d);
				res += y * m * yd + d;
			}
		}
	}
	return res;
}

#if 0
static unsigned int
hyper(unsigned int hyper)
{
	dt_ymd_t x;
	for (int y = 1917; y < 4096; y++) {
		for (int m = 1; m <= 12; m++) {
			for (int d = 0; d < 32; d++) {
				unsigned int yd;
				x.y = y;
				x.m = m;
				x.d = d;
				yd = __ymd_get_yday(x);
				hyper += yd;
			}
		}
	}
	return hyper;
}
#endif

int
main(void)
{
	unsigned int supersum = 0;

	for (size_t i = 0; i < 4096; i++) {
		supersum += super(supersum);
	}
	printf("super %u\n", supersum);
	if (supersum != 2780223808U) {
		return 1;
	}
	return 0;
}

/* basic_md_get_yday.c ends here */
