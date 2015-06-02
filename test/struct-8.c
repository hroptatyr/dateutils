#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdio.h>
#include "date-core.h"

int
main(void)
{
	int res = 0;

#define CHECK_SIZE(x, y)						\
	if (sizeof(x) != sizeof(y)) {					\
		fprintf(stderr, "sizeof(" #x ") -> %zu\n", sizeof(x));	\
		res = 1;						\
	}

	CHECK_SIZE(struct dt_d_s, uint64_t);
	CHECK_SIZE(dt_ymd_t, uint32_t);
	CHECK_SIZE(dt_ymcw_t, uint32_t);
	CHECK_SIZE(dt_bizda_t, uint32_t);
	CHECK_SIZE(dt_daisy_t, uint32_t);
	return res;
}

/* struct-8.c ends here */
