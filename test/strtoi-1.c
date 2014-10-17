#include <stdio.h>
#include "strops.h"
#include "strops.c"
#include "nifty.h"

static const char *tst1[] = {"84", "52", "01", "99", "102", "120", "001", "4"};
static const char *tst0[] = {"8.4", "5a", "a1", "9.", "1+2", "", "\t", "#4"};

int
main(void)
{
	for (size_t i = 0U; i < countof(tst1); i++) {
		const char *x;
		int32_t r = strtoi_lim(tst1[i], &x, 0, 60);
		printf("%d %td\n", r, x - tst1[i]);
	}
	for (size_t i = 0U; i < countof(tst0); i++) {
		const char *x;
		int32_t r = strtoi_lim(tst0[i], &x, 0, 60);
		printf("%d %td\n", r, x - tst0[i]);
	}
	return 0;
}

