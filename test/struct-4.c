#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include "dt-core.h"

int
main(void)
{
	struct dt_dt_s dt;

	dt.neg = 1;
	if (dt.d.neg != 1) {
		return 1;
	}

	dt.neg = 0;
	if (dt.d.neg != 0) {
		return 1;
	}
	return 0;
}

/* struct-4.c ends here */
