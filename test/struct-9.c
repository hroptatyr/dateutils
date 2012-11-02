#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdio.h>
#include "time-core.h"

int
main(void)
{
	int res = 0;

#define CHECK_SIZE(x, y)						\
	if (sizeof(x) != sizeof(y)) {					\
		fprintf(stderr, "sizeof(" #x ") -> %zu\n", sizeof(x));	\
		res = 1;						\
	}

	CHECK_SIZE(struct dt_t_s, uint64_t);
	CHECK_SIZE(dt_hms_t, struct {
		uint64_t foo:56;
	} __attribute__((packed)));
	return res;
}

/* struct-9.c ends here */
