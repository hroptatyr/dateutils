#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include "dt-core.h"

int
main(void)
{
	struct dt_dt_s dt;

	dt.typ = (dt_dttyp_t)DT_UNK;
	if (dt.d.typ != DT_DUNK) {
		return 1;
	}
	return 0;
}

/* struct-1.c ends here */
