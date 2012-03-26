#include "dt-core.h"

int
main(void)
{
	int res = 0;

	if (DT_PACK != DT_NTYP) {
		fprintf(stderr, "DT_PACK %hu != DT_NTYP %hu\n",
			DT_PACK, DT_NTYP);
		res = 1;
	}
	if (DT_SEXY != DT_NTYP + 1) {
		fprintf(stderr, "DT_SEXY %hu != DT_NTYP + 1 %hu\n",
			DT_SEXY, DT_NTYP + 1);
		res = 1;
	}
	if (DT_SANDWICH != 16) {
		fprintf(stderr, "DT_SANDWICH %hu not a 2-power\n",
			DT_SANDWICH);
		res = 1;
	}
	return res;
}

/* enum-1.c ends here */
