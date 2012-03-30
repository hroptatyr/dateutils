#include "dt-core.h"

int
main(void)
{
	int res = 0;

	if (DT_PACK != DT_NDTYP) {
		fprintf(stderr, "DT_PACK %hu != DT_NDTYP %hu\n",
			DT_PACK, DT_NDTYP);
		res = 1;
	}
	if (DT_SEXY != DT_NDTYP + 1) {
		fprintf(stderr, "DT_SEXY %hu != DT_NTYP + 1 %hu\n",
			DT_SEXY, DT_NDTYP + 1);
		res = 1;
	}
	return res;
}

/* enum-1.c ends here */
