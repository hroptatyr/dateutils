#include "dt-core.h"

int
main(void)
{
	struct dt_dt_s dt;

	dt.dur = 1;
	if (dt.d.dur != 1) {
		return 1;
	}

	dt.dur = 0;
	if (dt.d.dur != 0) {
		return 1;
	}
	return 0;
}

/* struct-3.c ends here */
