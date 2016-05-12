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
	for (int y = 1917; y < 2199; y++) {
		dt_dow_t w;
		w = __get_jan01_wday(y);
		res += y * (w == DT_SUNDAY ? 0 : w);
	}
	return res;
}

int
main(void)
{
	unsigned int supersum = 0;

	for (size_t i = 0; i < 1024 * 1024; i++) {
		supersum += super(supersum);
	}
	printf("super %u\n", supersum);
	if (supersum != 4293232620U) {
		return 1;
	}
	return 0;
}

/* basic_get_jan01_wday.c ends here */
