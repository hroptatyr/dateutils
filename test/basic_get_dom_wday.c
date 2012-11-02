#include "dt-core.h"

static unsigned int
super(void)
{
	unsigned int res = 0;

	for (int y = 1917; y < 2199; y++) {
		for (int m = 1; m <= 12; m ++) {
			for (int d = 1; d <= 28; d++) {
				dt_dow_t w = __get_dom_wday(y, m, d);
				res += y * m * w + d;
			}
		}
	}
	return res;
}

int
main(void)
{
	unsigned int supersum = 0;

	for (size_t i = 0; i < 512; i++) {
		supersum += super();
	}
	printf("super %u\n", supersum);
	if (supersum != 1486417920U) {
		return 1;
	}
	return 0;
}

/* basic_get_wday.c ends here */
