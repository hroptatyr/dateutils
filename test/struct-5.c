#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include "dt-core.h"

int
main(void)
{
	struct dt_dt_s dt;

	dt.neg = 0;
	dt.xxx = 0;
	dt.typ = DT_SEXY;
	if (dt.d.typ != (dt_dtyp_t)DT_SEXY || dt.d.neg || dt.d.xxx) {
		return 1;
	}

	dt_make_sandwich(&dt, DT_DUNK, DT_TUNK);
	if (!dt.sandwich || dt.d.neg || dt.d.xxx) {
		return 1;
	}
	return 0;
}

/* struct-5.c ends here */
