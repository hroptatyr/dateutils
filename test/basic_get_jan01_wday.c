#include "dt-core.h"

static unsigned int
super(unsigned int super)
{
	for (int y = 1917; y < 2199; y++) {
		dt_dow_t w;
		w = __get_jan01_wday(y);
		super += y * w;
	}
	return super;
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
