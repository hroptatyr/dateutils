#include <stdio.h>
#include "dt-core.h"

int
main(void)
{
	struct dt_dt_s dt;

	memset(&dt, 0, sizeof(dt));
	dt.sexy = 0x1fffffffffffff;
	if (dt.typ != DT_UNK || dt.dur || dt.neg) {
		return 1;
	}
	return 0;
}

/* struct-6.c ends here */
