#include "dt-core.h"

int
main(void)
{
	struct dt_dt_s dt;

	dt.d.typ = DT_YMD;
	if (dt.typ != DT_YMD) {
		return 1;
	}
	return 0;
}

/* struct-2.c ends here */
