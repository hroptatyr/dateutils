#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include "dt-core.h"

int
main(void)
{
	int res = 0;

	if ((unsigned int)DT_PACK != (unsigned int)DT_NDTYP) {
		fprintf(stderr, "DT_PACK %u != DT_NDTYP %u\n",
			(unsigned int)DT_PACK, (unsigned int)DT_NDTYP);
		res = 1;
	}
	if ((unsigned int)DT_SEXY != (unsigned int)(DT_NDTYP + 1U)) {
		fprintf(stderr, "DT_SEXY %u != DT_NTYP + 1 %u\n",
			(unsigned int)DT_SEXY, (unsigned int)(DT_NDTYP + 1U));
		res = 1;
	}
	return res;
}

/* enum-1.c ends here */
