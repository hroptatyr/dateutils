#include "dt-core.h"

int
main(void)
{
	struct dt_dt_s dt;

	dt.neg = 0;
	dt.dur = 0;
	dt.typ = DT_SEXY;
	if (dt.d.typ != DT_SEXY || dt.d.neg || dt.d.dur) {
		return 1;
	}

	dt.typ = DT_SANDWICH_DT(DT_UNK);
	dt.sandwich = 1;
	if (dt.d.neg || dt.d.dur) {
		return 1;
	}
	return 0;
}

/* struct-5.c ends here */
