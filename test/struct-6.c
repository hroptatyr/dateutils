#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdio.h>
#include <string.h>
#include "dt-core.h"

int
main(void)
{
	struct dt_dt_s dt;

	memset(&dt, 0, sizeof(dt));
	dt.sexy = 0x1fffffffffffffLL;
	if (dt.typ != DT_UNK || dt.xxx || dt.neg) {
		return 1;
	}
	return 0;
}

/* struct-6.c ends here */
