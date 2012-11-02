#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include "dt-core.h"

int
main(void)
{
	struct dt_dt_s dt;

	dt.d.typ = DT_YMD;
	if (dt.typ != (dt_dttyp_t)DT_YMD) {
		return 1;
	}
	return 0;
}

/* struct-2.c ends here */
