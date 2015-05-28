#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include "dt-core.h"

int
main(void)
{
	struct dt_dt_s dt;

	dt.xxx = 1;
	if (dt.d.xxx != 1) {
		return 1;
	}

	dt.xxx = 0;
	if (dt.d.xxx != 0) {
		return 1;
	}
	return 0;
}

/* struct-3.c ends here */
