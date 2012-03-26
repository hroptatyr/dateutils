#include "dt-core.h"

int
main(void)
{
	struct dt_dt_s dt;

	dt.typ = (dt_dttyp_t)DT_UNK;
	if (dt.d.typ != DT_UNK) {
		return 1;
	}
	return 0;
}

/* struct-1.c ends here */
