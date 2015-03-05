#include <stdio.h>
#include "strops.h"
#include "strops.c"
#include "nifty.h"

static const char *tst1[] = {"84", "52", "01", "99", "102", "120", "001", "4"};
static const char *tst0[] = {"8.4", "5a", "a1", "9.", "1+2", "", "\t", "#4"};

int
main(void)
{
	int32_t s = 0;

	for (size_t i = 0; i < 100000000U; i++) {
		const char *UNUSED(x);
		s += strtoi_lim(tst1[0U], &x, 0, 90);
	}
	printf("%d\n", s);
	return 0;
}

